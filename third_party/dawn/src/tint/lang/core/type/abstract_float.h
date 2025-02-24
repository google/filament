// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_ABSTRACT_FLOAT_H_
#define SRC_TINT_LANG_CORE_TYPE_ABSTRACT_FLOAT_H_

#include <string>

#include "src/tint/lang/core/type/abstract_numeric.h"

namespace tint::core::type {

/// An abstract-float type.
/// @see https://www.w3.org/TR/WGSL/#abstractFloat
class AbstractFloat final : public Castable<AbstractFloat, AbstractNumeric> {
  public:
    /// Constructor
    AbstractFloat();

    /// Destructor
    ~AbstractFloat() override;

    /// @returns the name for this type when printed in diagnostics.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    AbstractFloat* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_ABSTRACT_FLOAT_H_
