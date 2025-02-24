// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Finally can be used to execute a lambda or function when the final reference
// to the Finally is dropped.
//
// The purpose of a finally is to perform cleanup or termination logic and is
// especially useful when there are multiple early returns within a function.
//
// A moveable Finally can be constructed with marl::make_finally().
// A sharable Finally can be constructed with marl::make_shared_finally().

#ifndef marl_finally_h
#define marl_finally_h

#include "export.h"

#include <functional>
#include <memory>
#include <utility>

namespace marl {

// Finally is a pure virtual base class, implemented by the templated
// FinallyImpl.
class Finally {
 public:
  virtual ~Finally() = default;
};

// FinallyImpl implements a Finally.
// The template parameter F is the function type to be called when the finally
// is destructed. F must have the signature void().
template <typename F>
class FinallyImpl : public Finally {
 public:
  MARL_NO_EXPORT inline FinallyImpl(const F& func);
  MARL_NO_EXPORT inline FinallyImpl(F&& func);
  MARL_NO_EXPORT inline FinallyImpl(FinallyImpl<F>&& other);
  MARL_NO_EXPORT inline ~FinallyImpl();

 private:
  FinallyImpl(const FinallyImpl<F>& other) = delete;
  FinallyImpl<F>& operator=(const FinallyImpl<F>& other) = delete;
  FinallyImpl<F>& operator=(FinallyImpl<F>&&) = delete;
  F func;
  bool valid = true;
};

template <typename F>
FinallyImpl<F>::FinallyImpl(const F& func_) : func(func_) {}

template <typename F>
FinallyImpl<F>::FinallyImpl(F&& func_) : func(std::move(func_)) {}

template <typename F>
FinallyImpl<F>::FinallyImpl(FinallyImpl<F>&& other)
    : func(std::move(other.func)) {
  other.valid = false;
}

template <typename F>
FinallyImpl<F>::~FinallyImpl() {
  if (valid) {
    func();
  }
}

template <typename F>
inline FinallyImpl<F> make_finally(F&& f) {
  return FinallyImpl<F>(std::forward<F>(f));
}

template <typename F>
inline std::shared_ptr<Finally> make_shared_finally(F&& f) {
  return std::make_shared<FinallyImpl<F>>(std::forward<F>(f));
}

}  // namespace marl

#endif  // marl_finally_h
