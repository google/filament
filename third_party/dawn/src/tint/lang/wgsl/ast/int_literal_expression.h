// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_INT_LITERAL_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_AST_INT_LITERAL_EXPRESSION_H_

#include "src/tint/lang/wgsl/ast/literal_expression.h"

namespace tint::ast {

/// An integer literal. The literal may have an 'i', 'u' or no suffix.
class IntLiteralExpression final : public Castable<IntLiteralExpression, LiteralExpression> {
  public:
    /// Literal suffix
    enum class Suffix {
        /// No suffix
        kNone,
        /// 'i' suffix (i32)
        kI,
        /// 'u' suffix (u32)
        kU,
    };

    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param val the literal value
    /// @param suf the literal suffix
    IntLiteralExpression(NodeID nid, const Source& src, int64_t val, Suffix suf);

    ~IntLiteralExpression() override;

    /// The literal value
    const int64_t value;

    /// The literal suffix
    const Suffix suffix;
};

/// @param suffix the enum value
/// @returns the string for the given enum value
std::string_view ToString(IntLiteralExpression::Suffix suffix);

/// Writes the integer literal suffix to the stream.
/// @param out the stream to write to
/// @param suffix the suffix to write
/// @returns out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, IntLiteralExpression::Suffix suffix) {
    return out << ToString(suffix);
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_INT_LITERAL_EXPRESSION_H_
