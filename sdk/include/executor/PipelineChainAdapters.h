#pragma once
#include "Outcome.h"
#include "TosError.h"
#include "executor/ProcessingPipline.h"
#include "executor/PiplineCoordinator.h"
#include <functional>
#include <vector>

namespace VolcengineTos {

// 起始步骤适配器：无上游输入，仅根据固定 Input 构建并执行 pipeline
// Getter 调用约定：ProcessingPipline<Input, Output>*(const Input&, const std::function<void(Outcome<TosError, Output>&)>&)
// 兼容成员函数/绑定/闭包等任意可调用对象

template <typename Input, typename Output, typename Getter>
inline std::function<void(const Notifier&)> startStepFromPipelineGetter(const Getter& getter, const Input& input) {
    return [getter, input](const Notifier& notifier) {
        ProcessingPipline<Input, Output>* pipeline = getter(input, notifier.asCallback<Output>());
        if (pipeline) {
            pipeline->asyncExecute();
        }
    };
}

// 普通执行步骤适配器：有上游输出 Prev，经映射得到 Input 后构建并执行 pipeline
// Mapper 调用约定：Input(const Prev&)
// Getter 调用约定：同上

template <typename Prev, typename Input, typename Output, typename Mapper, typename Getter>
inline std::function<void(const Prev&, const Notifier&)> execStepFromPipelineGetter(const Mapper& mapPrevToInput,
                                                                                    const Getter& getter) {
    return [mapPrevToInput, getter](const Prev& prev, const Notifier& notifier) {
        const Input input = mapPrevToInput(prev);
        ProcessingPipline<Input, Output>* pipeline = getter(input, notifier.asCallback<Output>());
        if (pipeline) {
            pipeline->asyncExecute();
        }
    };
}

// 带转换重载：在 getter 产生 OIn 后，通过 convert(in, out) 收敛为 OOut 并通知下游
// Converter 调用约定：void(Outcome<TosError, OIn>& in, Outcome<TosError, OOut>& out)
// 注意：convert 需设置 out.setSuccess(...)，并在成功时填充 out.setR(...)
template <typename Prev, typename Input, typename OIn, typename OOut, typename Mapper, typename Getter, typename Converter>
inline std::function<void(const Prev&, const Notifier&)> execStepFromPipelineGetter(const Mapper& mapPrevToInput,
                                                                                    const Getter& getter,
                                                                                    const Converter& convert) {
    return [mapPrevToInput, getter, convert](const Prev& prev, const Notifier& notifier) {
        const Input input = mapPrevToInput(prev);
        ProcessingPipline<Input, OIn>* pipeline = getter(input, [notifier, convert](Outcome<TosError, OIn>& in) {
            Outcome<TosError, OOut> out;
            convert(in, out);
            notifier.notify<OOut>(out);
        });
        if (pipeline) {
            pipeline->asyncExecute();
        }
    };
}

// 转换版执行步骤适配器：上游输出 Prev，经映射得到 Input，getter 返回 OIn，随后通过 convert(in, out) 转为 OOut 并通知
// Converter 调用约定：void(Outcome<TosError, OIn>& in, Outcome<TosError, OOut>& out)
// 注意：convert 需设置 out.setSuccess(...)，并在成功时填充 out.setR(...)

template <typename Prev, typename Input, typename OIn, typename OOut, typename Mapper, typename Getter, typename Converter>
inline std::function<void(const Prev&, const Notifier&)> execStepFromPipelineGetterConvert(const Mapper& mapPrevToInput,
                                                                                           const Getter& getter,
                                                                                           const Converter& convert) {
    // 兼容旧 API：转发到新的带转换重载
    return execStepFromPipelineGetter<Prev, Input, OIn, OOut, Mapper, Getter, Converter>(mapPrevToInput, getter, convert);
}

// 直接传入已构建的 pipeline 指针：起始步骤适配器
// 调用约定：传入 ProcessingPipline<Input, Output>*，内部绑定 typed onRequestDone 并执行

template <typename Input, typename Output>
inline std::function<void(const Notifier&)> startStepFromPipelinePtr(ProcessingPipline<Input, Output>* pipeline) {
    return [pipeline](const Notifier& notifier) {
        if (pipeline) {
            pipeline->onRequestDone(notifier.asCallback<Output>());
            pipeline->asyncExecute();
        }
    };
}

// 直接传入构建函数：执行步骤适配器（Prev -> Input -> ProcessingPipline<Input,Output>*）
// Builder 调用约定：ProcessingPipline<Input, Output>*(const Prev&)

template <typename Prev, typename Input, typename Output, typename Builder>
inline std::function<void(const Prev&, const Notifier&)> execStepFromPipelinePtr(const Builder& builder) {
    return [builder](const Prev& prev, const Notifier& notifier) {
        ProcessingPipline<Input, Output>* pipeline = builder(prev);
        if (pipeline) {
            pipeline->onRequestDone(notifier.asCallback<Output>());
            pipeline->asyncExecute();
        }
    };
}

// 纯转换执行步骤适配器：不构建/执行 pipeline，仅将上游 Prev 转换为 Output 并通知下游
// Converter 调用约定：void(const Prev&, Outcome<TosError, Output>& out)

template <typename Prev, typename Output, typename Converter>
inline std::function<void(const Prev&, const Notifier&)> execStepFromValue(const Converter& convert) {
    return [convert](const Prev& prev, const Notifier& notifier) {
        Outcome<TosError, Output> out;
        convert(prev, out);
        notifier.notify<Output>(out);
    };
}

}  // namespace VolcengineTos

// MapReduce runner 构造工具
// ------------------------
namespace VolcengineTos {

// 从 getter + 固定输入构造 runner：返回 std::function<void(const std::function<void(Outcome<TosError,OItem>&)>&)>
// 该 runner 在被调用时会构建并执行对应的 pipeline，并把结果通过传入的 typed callback 回传

template <typename Input, typename OItem, typename Getter>
inline std::function<void(const std::function<void(Outcome<TosError, OItem>&)>&)>
makeRunnerFromGetter(const Getter& getter, const Input& input) {
    return [getter, input](const std::function<void(Outcome<TosError, OItem>&)>& cb) {
        ProcessingPipline<Input, OItem>* pipeline = getter(input, cb);
        if (pipeline) {
            pipeline->asyncExecute();
        }
    };
}

// 从已构建的 pipeline 指针构造 runner：返回 std::function<void(const std::function<void(Outcome<TosError,OItem>&)>&)>

template <typename Input, typename OItem>
inline std::function<void(const std::function<void(Outcome<TosError, OItem>&)>&)>
makeRunnerFromPipelinePtr(ProcessingPipline<Input, OItem>* pipeline) {
    return [pipeline](const std::function<void(Outcome<TosError, OItem>&)>& cb) {
        if (pipeline) {
            pipeline->onRequestDone(cb);
            pipeline->asyncExecute();
        }
    };
}

// 将 IPrev -> vector<Input> 的映射与 getter 组合为“map”函数：IPrev -> vector<runner>
// runner 类型为 std::function<void(const std::function<void(Outcome<TosError,OItem>&)>&)>

template <typename IPrev, typename Input, typename OItem, typename MapInputs, typename Getter>
inline std::function<std::vector<std::function<void(const std::function<void(Outcome<TosError, OItem>&)>&)>>(const IPrev&)>
makeRunnersFromInputs(const MapInputs& mapInputs, const Getter& getter) {
    return [mapInputs, getter](const IPrev& prev) -> std::vector<std::function<void(const std::function<void(Outcome<TosError, OItem>&)>&)>> {
        std::vector<std::function<void(const std::function<void(Outcome<TosError, OItem>&)>&)>> runners;
        std::vector<Input> inputs = mapInputs(prev);
        runners.reserve(inputs.size());
        for (size_t i = 0; i < inputs.size(); ++i) {
            runners.push_back(makeRunnerFromGetter<Input, OItem>(getter, inputs[i]));
        }
        return runners;
    };
}

}  // namespace VolcengineTos
