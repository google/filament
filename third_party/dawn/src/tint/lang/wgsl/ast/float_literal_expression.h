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

#ifndef SRC_TINT_LANG_WGSL_AST_FLOAT_LITERAL_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_AST_FLOAT_LITERAL_EXPRESSION_H_

#include <string>

#include "src/tint/lang/wgsl/ast/literal_expression.h"

namespace tint::ast {

/// A float literal
class FloatLiteralExpression final : public Castable<FloatLiteralExpression, LiteralExpression> {
  public:
    /// Literal suffix
    enum class Suffix {
        /// No suffix
        kNone,
        /// 'f' suffix (f32)
        kF,
        /// 'h' suffix (f16)
        kH,
    };

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param val the literal value
    /// @param suf the literal suffix
    FloatLiteralExpression(GenerationID pid, NodeID nid, const Source& src, double val, Suffix suf);
    ~FloatLiteralExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const FloatLiteralExpression* Clone(CloneContext& ctx) const override;

    /// The literal value
    const double value;

    /// The literal suffix
    const Suffix suffix;
};

/// @param suffix the enum value
/// @returns the string for the given enum value
std::string_view ToString(FloatLiteralExpression::Suffix suffix);

/// Writes the float literal suffix to the stream.
/// @param out the stream to write to
/// @param suffix the suffix to write
/// @returns out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, FloatLiteralExpression::Suffix suffix) {
    return out << ToString(suffix);
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_FLOAT_LITERAL_EXPRESSION_H_
