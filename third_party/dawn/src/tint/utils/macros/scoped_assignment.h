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

#ifndef SRC_TINT_UTILS_MACROS_SCOPED_ASSIGNMENT_H_
#define SRC_TINT_UTILS_MACROS_SCOPED_ASSIGNMENT_H_

#include <type_traits>

#include "src/tint/utils/macros/concat.h"

namespace tint {

/// Helper class that temporarily assigns a value to a variable for the lifetime
/// of the ScopedAssignment object. Once the ScopedAssignment is destructed, the
/// original value is restored.
template <typename T>
class ScopedAssignment {
  public:
    /// Constructor
    /// @param var the variable to temporarily assign a new value to
    /// @param val the value to assign to `ref` for the lifetime of this
    /// ScopedAssignment.
    ScopedAssignment(T& var, T val) : ref_(var) {
        old_value_ = var;
        var = val;
    }

    /// Destructor
    /// Restores the original value of the variable.
    ~ScopedAssignment() { ref_ = old_value_; }

  private:
    ScopedAssignment(const ScopedAssignment&) = delete;
    ScopedAssignment& operator=(const ScopedAssignment&) = delete;

    T& ref_;
    T old_value_;
};

}  // namespace tint

/// TINT_SCOPED_ASSIGNMENT(var, val) assigns `val` to `var`, and automatically
/// restores the original value of `var` when exiting the current lexical scope.
#define TINT_SCOPED_ASSIGNMENT(var, val)                                          \
    ::tint::ScopedAssignment<std::remove_reference_t<decltype(var)>> TINT_CONCAT( \
        tint_scoped_assignment_, __COUNTER__) {                                   \
        var, val                                                                  \
    }

#endif  // SRC_TINT_UTILS_MACROS_SCOPED_ASSIGNMENT_H_
