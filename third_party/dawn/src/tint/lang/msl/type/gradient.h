// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_MSL_TYPE_GRADIENT_H_
#define SRC_TINT_LANG_MSL_TYPE_GRADIENT_H_

#include <cstdint>
#include <string>

#include "src/tint/lang/core/type/type.h"

namespace tint::msl::type {

/// Gradient represents the `metal::gradient{2d,3d,cube}` type.
class Gradient final : public Castable<Gradient, core::type::Type> {
  public:
    enum class Dim : uint8_t {
        k2d,
        k3d,
        kCube,
    };

    /// Constructor
    /// @param dim the gradient dimensionality
    explicit Gradient(enum Dim dim);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this type
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Gradient* Clone(core::type::CloneContext& ctx) const override;

    /// @returns the number of dimensions of the gradient
    enum Dim Dim() const { return dim_; }

  private:
    const enum Dim dim_;
};

}  // namespace tint::msl::type

#endif  // SRC_TINT_LANG_MSL_TYPE_GRADIENT_H_
