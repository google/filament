// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_MACROS_DEFER_H_
#define SRC_TINT_UTILS_MACROS_DEFER_H_

#include <utility>

#include "src/tint/utils/macros/concat.h"

namespace tint {

/// Defer executes a function or function like object when it is destructed.
template <typename F>
class Defer {
  public:
    /// Constructor
    /// @param f the function to call when the Defer is destructed
    explicit Defer(F&& f) : f_(std::move(f)) {}

    /// Move constructor
    Defer(Defer&&) = default;

    /// Destructor
    /// Calls the deferred function
    ~Defer() { f_(); }

  private:
    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

    F f_;
};

/// Constructor
/// @param f the function to call when the Defer is destructed
/// @return the defer object
template <typename F>
inline Defer<F> MakeDefer(F&& f) {
    return Defer<F>(std::forward<F>(f));
}

}  // namespace tint

/// TINT_DEFER(S) executes the statement(s) `S` when exiting the current lexical
/// scope.
#define TINT_DEFER(S) auto TINT_CONCAT(tint_defer_, __COUNTER__) = ::tint::MakeDefer([&] { S; })

#endif  // SRC_TINT_UTILS_MACROS_DEFER_H_
