#pragma once
#include "Outcome.h"
#include "TosError.h"

#include <typeinfo>

namespace VolcengineTos {

class AnyOutcome {
public:
    AnyOutcome() : impl_(nullptr) {}

    template <typename O>
    explicit AnyOutcome(const Outcome<TosError, O>& out) : impl_(new Holder<O>(out)) {}

    AnyOutcome(const AnyOutcome& other) : impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

    AnyOutcome& operator=(const AnyOutcome& other) {
        if (this != &other) {
            delete impl_;
            impl_ = other.impl_ ? other.impl_->clone() : nullptr;
        }
        return *this;
    }

    ~AnyOutcome() {
        delete impl_;
        impl_ = nullptr;
    }

    bool hasValue() const { return impl_ != nullptr; }
    bool isSuccess() const { return impl_ ? impl_->isSuccess() : false; }

    template <typename O>
    const Outcome<TosError, O>* tryCast() const {
        if (!impl_) {
            return nullptr;
        }
        if (impl_->type() == typeid(O)) {
            return &static_cast<const Holder<O>*>(impl_)->value_;
        }
        return nullptr;
    }

    template <typename O>
    Outcome<TosError, O>* tryCast() {
        if (!impl_) {
            return nullptr;
        }
        if (impl_->type() == typeid(O)) {
            return &static_cast<Holder<O>*>(impl_)->value_;
        }
        return nullptr;
    }

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual Concept* clone() const = 0;
        virtual const std::type_info& type() const = 0;
        virtual bool isSuccess() const = 0;
    };

    template <typename O>
    struct Holder final : Concept {
        Outcome<TosError, O> value_;
        explicit Holder(const Outcome<TosError, O>& v) : value_(v) {}
        Concept* clone() const override { return new Holder<O>(value_); }
        const std::type_info& type() const override { return typeid(O); }
        bool isSuccess() const override { return value_.isSuccess(); }
    };

    Concept* impl_;
};

}  // namespace VolcengineTos
