// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/intrinsic/ctor_conv.h"
#include "src/tint/lang/core/intrinsic/table_data.h"

namespace tint::core::intrinsic {

/// Dialect holds the intrinsic table data and types for the core dialect
struct Dialect {
    /// The dialect's intrinsic table data
    static const TableData kData;

    /// The dialect's builtin function enumerator
    using BuiltinFn = core::BuiltinFn;

    /// The dialect's type constructor / convertor enumerator
    using CtorConv = core::intrinsic::CtorConv;

    /// @returns the name of the builtin function @p fn
    /// @param fn the builtin function
    static std::string_view ToString(BuiltinFn fn) { return str(fn); }

    /// @returns the name of the type constructor / convertor @p ty
    /// @param ty the type constructor / convertor
    static std::string_view ToString(CtorConv ty) { return str(ty); }
};

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_
