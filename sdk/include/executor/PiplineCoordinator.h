#pragma once
#include "Outcome.h"
#include "TosError.h"
#include "executor/PipelineBase.h"

#include <functional>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>

namespace VolcengineTos {

class IStage;
class PiplineCoordinator;  // 前置声明

// 声明包装函数以避免在不完整类型上访问成员（Clangd 友好）
inline void PipelineCoordinator_onStageResult(PiplineCoordinator* coord, IStage* stage, const AnyOutcome& ao);

// Notifier：用于让用户的异步步骤在完成时通知协调器（通过回调）
class Notifier {
public:
    Notifier() = default;
    explicit Notifier(const std::function<void(AnyOutcome)>& cb) : cb_(cb) {
    }

    void setCallback(const std::function<void(AnyOutcome)>& cb) {
        cb_ = cb;
    }

    template <typename O>
    void notify(Outcome<TosError, O>& outcome) const {
        if (cb_) {
            cb_(AnyOutcome(outcome));
        }
    }

    template <typename O>
    std::function<void(Outcome<TosError, O>&)> asCallback() const {
        const auto cb = cb_;
        return [cb](Outcome<TosError, O>& out) {
            if (cb) {
                cb(AnyOutcome(out));
            }
        };
    }

    std::function<void(AnyOutcome)> cb_;

    // 直接返回类型擦除回调以桥接 ProcessingPipline::onAnyRequestDone
    std::function<void(AnyOutcome)> asAnyCallback() const {
        return cb_;
    }
};

// 阶段接口
class IStage {
public:
    virtual ~IStage() = default;
    virtual void startWith(const AnyOutcome& prev, PiplineCoordinator* coord) = 0;
    virtual void setNext(IStage* /*next*/) {
        // 默认空实现
    }
};

// TypedRunner：用于 mapReduce 的并行 runner，接受 typed 回调并执行
// 约定：void(const std::function<void(Outcome<TosError, O>&)>&)
template <typename O>
using TypedRunner = std::function<void(const std::function<void(Outcome<TosError, O>&)>&)>;

// 可链式阶段：内置 next 指针
class NextableStage : public IStage {
public:
    NextableStage() : next_(nullptr) {
    }
    void setNext(IStage* next) override {
        next_ = next;
    }
    IStage* next() const {
        return next_;
    }

private:
    IStage* next_;
};

// 起始阶段：执行基础异步步骤（无前置输入）
template <typename O0>
class StartStage final : public NextableStage {
public:
    explicit StartStage(const std::function<void(const Notifier&)>& step) : step_(step) {
    }

    void startWith(const AnyOutcome& /*prev*/, PiplineCoordinator* coord) override {
        const Notifier notifier(
                [this, coord](const AnyOutcome& ao) { PipelineCoordinator_onStageResult(coord, this, ao); });
        step_(notifier);
    }

private:
    std::function<void(const Notifier&)> step_;
};

// 执行阶段：从上一个结果 I 生成异步输出 O
template <typename I, typename O>
class ExecStage final : public NextableStage {
public:
    explicit ExecStage(const std::function<void(const I&, const Notifier&)>& step) : step_(step) {
    }

    void startWith(const AnyOutcome& prev, PiplineCoordinator* coord) override {
        const Outcome<TosError, I>* in = prev.tryCast<I>();
        if (in == nullptr) {
            Outcome<TosError, O> bad;
            TosError e;
            e.setIsClientError(true);
            e.setMessage("ExecStage input type mismatch");
            bad.setE(e);
            bad.setSuccess(false);
            PipelineCoordinator_onStageResult(coord, this, AnyOutcome(bad));
            return;
        }
        if (!in->isSuccess()) {
            // 直接转发错误，不做转换（由 Coordinator 在最终回调进行统一处理）
            PipelineCoordinator_onStageResult(coord, this, prev);
            return;
        }
        Notifier notifier([this, coord](const AnyOutcome& ao) { PipelineCoordinator_onStageResult(coord, this, ao); });
        step_(in->result(), notifier);
    }

private:
    std::function<void(const I&, const Notifier&)> step_;
};

// 并行 Map/Reduce 阶段：根据上游输入 IPrev 生成多个 OItem 的 runner，并在全部完成后通过 reduce 汇总为 OReduced
template <typename IPrev, typename OItem, typename OReduced>
class MapReduceStage final : public NextableStage {
public:
    typedef std::function<std::vector<TypedRunner<OItem>>(const IPrev&)> MapFunc;
    typedef std::function<void(const std::vector<Outcome<TosError, OItem>>&, Outcome<TosError, OReduced>&)> ReduceFunc;

    MapReduceStage(const MapFunc& map, const ReduceFunc& reduce) : map_(map), reduce_(reduce) {
    }

    void startWith(const AnyOutcome& prev, PiplineCoordinator* coord) override {
        const Outcome<TosError, IPrev>* in = prev.tryCast<IPrev>();
        if (in == nullptr) {
            Outcome<TosError, OReduced> bad;
            TosError e;
            e.setIsClientError(true);
            e.setMessage("MapReduceStage input type mismatch");
            bad.setE(e);
            bad.setSuccess(false);
            PipelineCoordinator_onStageResult(coord, this, AnyOutcome(bad));
            return;
        }
        if (!in->isSuccess()) {
            // 上游失败则直接转发错误（协调器会终止）
            PipelineCoordinator_onStageResult(coord, this, prev);
            return;
        }

        const IPrev& iprev = in->result();
        std::vector<TypedRunner<OItem>> runners = map_(iprev);

        // 若没有 runner，则对空集合执行 reduce
        if (runners.empty()) {
            Outcome<TosError, OReduced> out;
            const std::vector<Outcome<TosError, OItem>> empty;
            reduce_(empty, out);
            PipelineCoordinator_onStageResult(coord, this, AnyOutcome(out));
            return;
        }

        // 并行状态：收集所有 item 结果，待全部完成后执行 reduce
        std::shared_ptr<std::vector<Outcome<TosError, OItem>>> results(new std::vector<Outcome<TosError, OItem>>());
        std::shared_ptr<std::atomic<int>> pending(new std::atomic<int>(static_cast<int>(runners.size())));
        std::shared_ptr<std::mutex> mtx(new std::mutex());
        ReduceFunc reduce = reduce_;

        auto typedCb = [results, pending, mtx, coord, this, reduce](Outcome<TosError, OItem>& item) {
            {
                std::lock_guard<std::mutex> lg(*mtx);
                results->push_back(item);
            }
            const int left = --(*pending);
            if (left == 0) {
                Outcome<TosError, OReduced> reduced;
                reduce(*results, reduced);
                PipelineCoordinator_onStageResult(coord, this, AnyOutcome(reduced));
            }
        };

        // 启动所有 runner
        for (size_t i = 0; i < runners.size(); ++i) {
            try {
                runners[i](typedCb);
            } catch (...) {
                Outcome<TosError, OItem> err;
                TosError e;
                e.setIsClientError(true);
                e.setMessage("MapReduceStage runner threw");
                err.setE(e);
                err.setSuccess(false);
                {
                    std::lock_guard<std::mutex> lg(*mtx);
                    results->push_back(err);
                }
                const int left = --(*pending);
                if (left == 0) {
                    Outcome<TosError, OReduced> reduced;
                    reduce(*results, reduced);
                    PipelineCoordinator_onStageResult(coord, this, AnyOutcome(reduced));
                }
            }
        }
    }

private:
    MapFunc map_;
    ReduceFunc reduce_;
};

// 条件阶段：根据谓词选择 true/false 分支（二者输出类型需一致，后续再汇合）
template <typename I>
class ConditionStage final : public NextableStage {
public:
    explicit ConditionStage(const std::function<bool(const I&)>& pred)
            : pred_(pred), trueNext_(nullptr), falseNext_(nullptr) {
    }

    void setTrueNext(IStage* next) {
        trueNext_ = next;
    }
    void setFalseNext(IStage* next) {
        falseNext_ = next;
    }

    IStage* trueNext() const {
        return trueNext_;
    }
    IStage* falseNext() const {
        return falseNext_;
    }

    void setNext(IStage* next) override {
        NextableStage::setNext(next);
        if (trueNext_) {
            trueNext_->setNext(next);
        }
        if (falseNext_) {
            falseNext_->setNext(next);
        }
    }

    void startWith(const AnyOutcome& prev, PiplineCoordinator* coord) override {
        const Outcome<TosError, I>* in = prev.tryCast<I>();
        if (in == nullptr) {
            Outcome<TosError, I> bad;
            TosError e;
            e.setIsClientError(true);
            e.setMessage("ConditionStage input type mismatch");
            bad.setE(e);
            bad.setSuccess(false);
            PipelineCoordinator_onStageResult(coord, this, AnyOutcome(bad));
            return;
        }
        if (!in->isSuccess()) {
            PipelineCoordinator_onStageResult(coord, this, prev);
            return;
        }
        const bool chooseTrue = pred_(in->result());
        IStage* next = chooseTrue ? trueNext_ : falseNext_;
        if (next) {
            next->startWith(prev, coord);
        } else {
            Outcome<TosError, I> bad;
            TosError e;
            e.setIsClientError(true);
            e.setMessage("ConditionStage branch not set");
            bad.setE(e);
            bad.setSuccess(false);
            PipelineCoordinator_onStageResult(coord, this, AnyOutcome(bad));
        }
    }

private:
    std::function<bool(const I&)> pred_;
    IStage* trueNext_;
    IStage* falseNext_;
};

// Builder：链式构建（then / thenIf ... else_）

template <typename OPrev>
class Chain;

template <typename IPrev, typename ONext>
class ConditionBuilder {
public:
    ConditionBuilder(PiplineCoordinator* coord, ConditionStage<IPrev>* cond, IStage* trueExec = nullptr)
            : coord_(coord), cond_(cond), trueExec_(trueExec), falseExec_(nullptr) {
    }

    // C++ 关键字 else 无法作为方法名，使用 else_ 作为替代
    Chain<ONext> else_(const std::function<void(const IPrev&, const Notifier&)>& pipelineFalse);

private:
    PiplineCoordinator* coord_;
    ConditionStage<IPrev>* cond_;
    IStage* trueExec_;   // thenIf 中创建的 true 分支首阶段（OPrev -> ITrue）
    IStage* falseExec_;  // else_ 中创建的 false 分支首阶段（OPrev -> ONext）
};

class PiplineCoordinator {
public:
    PiplineCoordinator() : root_(nullptr) {
    }
    ~PiplineCoordinator() = default;

    PiplineCoordinator(PiplineCoordinator&&) = default;
    PiplineCoordinator& operator=(PiplineCoordinator&&) = default;

    PiplineCoordinator(const PiplineCoordinator&) = delete;
    PiplineCoordinator& operator=(const PiplineCoordinator&) = delete;

    // 控制运行结束后的自动释放：默认关闭，仅当通过 newWithStart 创建并设为 true 时启用
    void setAutoRelease(const bool v) {
        auto_release_ = v;
    }

    // 模板构造：在创建协调器时直接传入基础起始步骤（baseStep）
    template <typename O0>
    explicit PiplineCoordinator(const std::function<void(const Notifier&)>& baseStep) {
        StartStage<O0>* start = emplaceStartStage<O0>(baseStep);
        root_ = start;
    }

    // 堆分配工厂：返回指针，并在运行结束后按需自动释放（autoRelease 默认开启）
    // 安全使用说明：仅在不需要外部持有协调器生命期、且链路结束后不再访问协调器时使用
    static PiplineCoordinator* withPiplineRelease(const bool autoRelease = true) {
        auto* c = new PiplineCoordinator();
        c->setAutoRelease(autoRelease);
        return c;
    }

    template <typename O0>
    Chain<O0> create(const std::function<void(const Notifier&)>& baseStep) {
        StartStage<O0>* start = emplaceStartStage<O0>(baseStep);
        root_ = start;
        return Chain<O0>(this, start);
    }

    // 从 root 起始返回链式构建器
    template <typename OPrev>
    Chain<OPrev> begin(const std::function<void(const Notifier&)>& baseStep) {
        this->create<OPrev>(baseStep);
        return Chain<OPrev>(this, root_);
    }

    // 类型安全版 run：最终回调接收 Outcome<TosError, OFinal>&
    template <typename OFinal>
    void run(const std::function<void(Outcome<TosError, OFinal>&)>& finalCallback) {
        final_ = [finalCallback](AnyOutcome ao) {
            const Outcome<TosError, OFinal>* p = ao.tryCast<OFinal>();
            if (p) {
                Outcome<TosError, OFinal> outCopy = *p;
                finalCallback(outCopy);
                return;
            }
            Outcome<TosError, OFinal> res;
            TosError e;
            e.setIsClientError(true);
            if (!ao.hasValue()) {
                e.setMessage("final outcome missing");
            } else if (!ao.isSuccess()) {
                e.setMessage("pipeline stopped on error");
            } else {
                e.setMessage("pipeline final type mismatch");
            }
            res.setE(e);
            res.setSuccess(false);
            finalCallback(res);
        };
        if (root_) {
            root_->startWith(AnyOutcome(), this);
        }
    }

    // 阶段回调：由 Notifier 驱动调用
    void onStageResult(IStage* stage, const AnyOutcome& ao) {
        if (!final_) {
            return;
        }
        if (!ao.isSuccess()) {
            // 默认：出错即停止
            final_(ao);
            if (auto_release_) {
                delete this;
            }
            return;
        }
        const auto* ns = dynamic_cast<NextableStage*>(stage);
        IStage* next = ns ? ns->next() : nullptr;
        if (next) {
            next->startWith(ao, this);
        } else {
            // 无后继：结束
            final_(ao);
            if (auto_release_) {
                delete this;
            }
        }
    }

    // 构建阶段工厂（统一所有权）
    template <typename I, typename O>
    ExecStage<I, O>* emplaceExecStage(const std::function<void(const I&, const Notifier&)>& step) {
        auto* s = new ExecStage<I, O>(step);
        stages_.push_back(std::unique_ptr<NextableStage>(s));
        return s;
    }

    template <typename I>
    ConditionStage<I>* emplaceConditionStage(const std::function<bool(const I&)>& pred) {
        auto* s = new ConditionStage<I>(pred);
        stages_.push_back(std::unique_ptr<NextableStage>(s));
        return s;
    }

    template <typename O0>
    StartStage<O0>* emplaceStartStage(const std::function<void(const Notifier&)>& step) {
        auto* s = new StartStage<O0>(step);
        stages_.push_back(std::unique_ptr<NextableStage>(s));
        return s;
    }

    // 创建并行 MapReduce 阶段
    template <typename IPrev, typename OItem, typename OReduced>
    MapReduceStage<IPrev, OItem, OReduced>* emplaceMapReduceStage(
            const typename MapReduceStage<IPrev, OItem, OReduced>::MapFunc& map,
            const typename MapReduceStage<IPrev, OItem, OReduced>::ReduceFunc& reduce) {
        auto* s = new MapReduceStage<IPrev, OItem, OReduced>(map, reduce);
        stages_.push_back(std::unique_ptr<NextableStage>(s));
        return s;
    }

private:
    std::vector<std::unique_ptr<NextableStage>> stages_;
    NextableStage* root_;
    std::function<void(AnyOutcome)> final_;
    bool auto_release_ = false;
};

inline void PipelineCoordinator_onStageResult(PiplineCoordinator* coord, IStage* stage, const AnyOutcome& ao) {
    if (coord)
        coord->onStageResult(stage, ao);
}

// 链式构建主体
template <typename OPrev>
class Chain {
public:
    Chain(PiplineCoordinator* coord, NextableStage* attach) : coord_(coord), attach_(attach) {
    }

    template <typename ONext>
    Chain<ONext> then(const std::function<void(const OPrev&, const Notifier&)>& step) {
        ExecStage<OPrev, ONext>* exec = coord_->emplaceExecStage<OPrev, ONext>(step);
        // 若 attach_ 是条件阶段：需要将两条分支的 next 都指向新阶段
        if (auto* cond = dynamic_cast<ConditionStage<OPrev>*>(attach_)) {
            if (cond->trueNext())
                cond->trueNext()->setNext(exec);
            if (cond->falseNext())
                cond->falseNext()->setNext(exec);
            cond->setNext(exec);
        } else {
            attach_->setNext(exec);
        }
        return Chain<ONext>(coord_, exec);
    }

    // 并行 map/reduce：根据 OPrev 生成多个 OItem 的 runner，并汇总为 OReduced
    template <typename OItem, typename OReduced>
    Chain<OReduced> mapReduce(const std::function<std::vector<TypedRunner<OItem>>(const OPrev&)>& map,
                              const std::function<void(const std::vector<Outcome<TosError, OItem>>&,
                                                       Outcome<TosError, OReduced>&)>& reducer) {
        MapReduceStage<OPrev, OItem, OReduced>* mr =
                coord_->emplaceMapReduceStage<OPrev, OItem, OReduced>(map, reducer);
        if (auto* cond = dynamic_cast<ConditionStage<OPrev>*>(attach_)) {
            if (cond->trueNext())
                cond->trueNext()->setNext(mr);
            if (cond->falseNext())
                cond->falseNext()->setNext(mr);
            cond->setNext(mr);
        } else {
            attach_->setNext(mr);
        }
        return Chain<OReduced>(coord_, mr);
    }

    template <typename ONext>
    ConditionBuilder<OPrev, ONext> thenIf(const std::function<bool(const OPrev&)>& predicate,
                                          const std::function<void(const OPrev&, const Notifier&)>& pipelineTrue) {
        ConditionStage<OPrev>* cond = coord_->emplaceConditionStage<OPrev>(predicate);
        ExecStage<OPrev, ONext>* trueExec = coord_->emplaceExecStage<OPrev, ONext>(pipelineTrue);
        cond->setTrueNext(trueExec);
        attach_->setNext(cond);
        return ConditionBuilder<OPrev, ONext>(coord_, cond, trueExec);
    }

private:
    PiplineCoordinator* coord_;
    NextableStage* attach_;
};

// ConditionBuilder 的 else_ 实现
template <typename IPrev, typename ONext>
Chain<ONext> ConditionBuilder<IPrev, ONext>::else_(
        const std::function<void(const IPrev&, const Notifier&)>& pipelineFalse) {
    // 创建 false 分支首阶段
    ExecStage<IPrev, ONext>* falseExec = coord_->emplaceExecStage<IPrev, ONext>(pipelineFalse);
    cond_->setFalseNext(falseExec);
    falseExec_ = falseExec;

    return Chain<ONext>(coord_, cond_);
}

}  // namespace VolcengineTos
