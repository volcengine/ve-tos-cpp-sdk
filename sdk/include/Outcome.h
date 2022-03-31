#pragma once

namespace VolcengineTos {
template <typename E, typename R>
class Outcome {
public:
  Outcome() : success_(false), e_(), r_() {}
  explicit Outcome(const E& e) : success_(false), e_(e) {}
  explicit Outcome(const R& r) : success_(true), r_(r) {}
  Outcome(const Outcome& other)
      : success_(other.success_), e_(other.e_), r_(other.r_) {}
  Outcome& operator=(const Outcome& other) {
    if (this != &other) {
      success_ = other.success_;
      e_ = other.e_;
      r_ = other.r_;
    }
    return *this;
  }
  bool isSuccess() const { return success_; }
  void setSuccess(bool success) { success_ = success; }
  const E& error() const { return e_; }
  const R& result() const { return r_; }
  E& error() { return e_; }
  R& result() { return r_; }
  void setE(E e) { e_ = e; }
  void setR(R r) { r_ = r; }

private:
  bool success_;
  E e_;
  R r_;
};
} // namespace VolcengineTos