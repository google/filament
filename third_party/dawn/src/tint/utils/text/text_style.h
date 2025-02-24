// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this
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
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_TEXT_TEXT_STYLE_H_
#define SRC_TINT_UTILS_TEXT_TEXT_STYLE_H_

#include <cstdint>
#include <tuple>
#include <type_traits>

// Forward declarations
namespace tint {
template <typename... VALUES>
struct ScopedTextStyle;
}

namespace tint {

/// TextStyle is a styling that can be applied to span of a StyledText.
class TextStyle {
  public:
    /// Bits is the integer type used to store the text style.
    using Bits = uint16_t;

    /// Bit patterns

    static constexpr Bits kStyleMask /*          */ = 0b00'00000'0000'00'111;
    static constexpr Bits kStyleUnderlined /*    */ = 0b00'00000'0000'00'001;
    static constexpr Bits kStyleBold /*          */ = 0b00'00000'0000'00'010;
    static constexpr Bits kStyleNoQuote /*       */ = 0b00'00000'0000'00'100;

    static constexpr Bits kCompareMask /*        */ = 0b00'00000'0000'11'000;
    static constexpr Bits kCompareMatch /*       */ = 0b00'00000'0000'01'000;
    static constexpr Bits kCompareMismatch /*    */ = 0b00'00000'0000'10'000;

    static constexpr Bits kSeverityMask /*       */ = 0b00'00000'1111'00'000;
    static constexpr Bits kSeverityDefault /*    */ = 0b00'00000'0000'00'000;
    static constexpr Bits kSeveritySuccess /*    */ = 0b00'00000'0001'00'000;
    static constexpr Bits kSeverityWarning /*    */ = 0b00'00000'0010'00'000;
    static constexpr Bits kSeverityError /*      */ = 0b00'00000'0011'00'000;
    static constexpr Bits kSeverityFatal /*      */ = 0b00'00000'0100'00'000;

    static constexpr Bits kKindMask /*           */ = 0b00'11111'0000'00'000;
    static constexpr Bits kKindCode /*           */ = 0b00'00001'0000'00'000;
    static constexpr Bits kKindKeyword /*        */ = 0b00'00011'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindVariable /*       */ = 0b00'00101'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindType /*           */ = 0b00'00111'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindFunction /*       */ = 0b00'01001'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindEnum /*           */ = 0b00'01011'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindLiteral /*        */ = 0b00'01101'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindAttribute /*      */ = 0b00'01111'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindComment /*        */ = 0b00'10001'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindLabel /*          */ = 0b00'10011'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindInstruction /*    */ = 0b00'10101'0000'00'000;  // includes kKindCode
    static constexpr Bits kKindSquiggle /*       */ = 0b00'00010'0000'00'000;

    /// Getters

    constexpr bool IsBold() const { return (bits & kStyleBold) != 0; }
    constexpr bool IsUnderlined() const { return (bits & kStyleUnderlined) == kStyleUnderlined; }
    constexpr bool IsNoQuote() const { return (bits & kStyleNoQuote) == kStyleNoQuote; }

    constexpr bool HasCompare() const { return (bits & kCompareMask) != 0; }
    constexpr bool IsMatch() const { return (bits & kCompareMask) == kCompareMatch; }
    constexpr bool IsMismatch() const { return (bits & kCompareMask) == kCompareMismatch; }

    constexpr bool HasSeverity() const { return (bits & kSeverityMask) != 0; }
    constexpr bool IsSuccess() const { return (bits & kSeverityMask) == kSeveritySuccess; }
    constexpr bool IsWarning() const { return (bits & kSeverityMask) == kSeverityWarning; }
    constexpr bool IsError() const { return (bits & kSeverityMask) == kSeverityError; }
    constexpr bool IsFatal() const { return (bits & kSeverityMask) == kSeverityFatal; }

    constexpr bool HasKind() const { return (bits & kKindMask) != 0; }
    constexpr bool IsCode() const { return (bits & kKindCode) == kKindCode; }
    constexpr bool IsKeyword() const { return (bits & kKindMask) == kKindKeyword; }
    constexpr bool IsVariable() const { return (bits & kKindMask) == kKindVariable; }
    constexpr bool IsType() const { return (bits & kKindMask) == kKindType; }
    constexpr bool IsFunction() const { return (bits & kKindMask) == kKindFunction; }
    constexpr bool IsEnum() const { return (bits & kKindMask) == kKindEnum; }
    constexpr bool IsLiteral() const { return (bits & kKindMask) == kKindLiteral; }
    constexpr bool IsAttribute() const { return (bits & kKindMask) == kKindAttribute; }
    constexpr bool IsComment() const { return (bits & kKindMask) == kKindComment; }
    constexpr bool IsLabel() const { return (bits & kKindMask) == kKindLabel; }
    constexpr bool IsInstruction() const { return (bits & kKindMask) == kKindInstruction; }
    constexpr bool IsSquiggle() const { return (bits & kKindMask) == kKindSquiggle; }

    /// Equality operator
    constexpr bool operator==(TextStyle other) const { return bits == other.bits; }

    /// Inequality operator
    constexpr bool operator!=(TextStyle other) const { return bits != other.bits; }

    /// @returns the combination of this TextStyle and @p other.
    /// If both this TextStyle and @p other have a compare style, severity style or kind style, then
    /// the style collision will resolve by using the style of @p other.
    constexpr TextStyle operator+(TextStyle other) const {
        Bits out = other.bits;
        out |= bits & kStyleMask;
        if (HasCompare() && !other.HasCompare()) {
            out |= bits & kCompareMask;
        }
        if (HasSeverity() && !other.HasSeverity()) {
            out |= bits & kSeverityMask;
        }
        if (HasKind() && !other.HasKind()) {
            out |= bits & kKindMask;
        }
        return TextStyle{out};
    }

    /// @returns a new ScopedTextStyle of @p values using with this TextStyle
    template <typename... VALUES>
    inline ScopedTextStyle<VALUES...> operator()(VALUES&&... values) const;

    /// The style bit pattern
    Bits bits = 0;
};

/// ScopedTextStyle is a span of text, styled with a TextStyle
template <typename... VALUES>
struct ScopedTextStyle {
    std::tuple<VALUES...> values;
    TextStyle style;
};

template <typename... VALUES>
ScopedTextStyle<VALUES...> TextStyle::operator()(VALUES&&... values) const {
    return ScopedTextStyle<VALUES...>{std::forward_as_tuple(values...), *this};
}

namespace detail {
template <typename T>
struct IsScopedTextStyle : std::false_type {};
template <typename... VALUES>
struct IsScopedTextStyle<ScopedTextStyle<VALUES...> > : std::true_type {};
}  // namespace detail

/// Resolves to true iff T is a ScopedTextStyle.
template <typename T>
static constexpr bool IsScopedTextStyle = detail::IsScopedTextStyle<T>::value;

}  // namespace tint

namespace tint::style {

/// Plain renders text without any styling
static constexpr TextStyle Plain = TextStyle{};
/// Bold renders text with a heavy weight
static constexpr TextStyle Bold = TextStyle{TextStyle::kStyleBold};
/// Underlined renders text with an underline
static constexpr TextStyle Underlined = TextStyle{TextStyle::kStyleUnderlined};
/// Code renders without a single quote when printed as plain text.
static constexpr TextStyle NoQuote = TextStyle{TextStyle::kStyleNoQuote};
/// Match renders text with the compare-match style
static constexpr TextStyle Match = TextStyle{TextStyle::kCompareMatch};
/// Mismatch renders text with the compare-mismatch style
static constexpr TextStyle Mismatch = TextStyle{TextStyle::kCompareMismatch};
/// Success renders text with the styling for a successful status
static constexpr TextStyle Success = TextStyle{TextStyle::kSeveritySuccess};
/// Warning renders text with the styling for a warning status
static constexpr TextStyle Warning = TextStyle{TextStyle::kSeverityWarning};
/// Error renders text with the styling for a error status
static constexpr TextStyle Error = TextStyle{TextStyle::kSeverityError};
/// Fatal renders text with the styling for a fatal status
static constexpr TextStyle Fatal = TextStyle{TextStyle::kSeverityFatal};
/// Code renders text with a 'code' style
static constexpr TextStyle Code = TextStyle{TextStyle::kKindCode};
/// Keyword renders text with a 'code' style that represents a 'keyword' token
static constexpr TextStyle Keyword = TextStyle{TextStyle::kKindKeyword};
/// Variable renders text with a 'code' style that represents a 'variable' token
static constexpr TextStyle Variable = TextStyle{TextStyle::kKindVariable};
/// Type renders text with a 'code' style that represents a 'type' token
static constexpr TextStyle Type = TextStyle{TextStyle::kKindType};
/// Function renders text with a 'code' style that represents a 'function' token
static constexpr TextStyle Function = TextStyle{TextStyle::kKindFunction};
/// Enum renders text with a 'code' style that represents a 'enum' token
static constexpr TextStyle Enum = TextStyle{TextStyle::kKindEnum};
/// Literal renders text with a 'code' style that represents a 'literal' token
static constexpr TextStyle Literal = TextStyle{TextStyle::kKindLiteral};
/// Attribute renders text with a 'code' style that represents an 'attribute' token
static constexpr TextStyle Attribute = TextStyle{TextStyle::kKindAttribute};
/// Comment renders text with a 'code' style that represents an 'comment' token
static constexpr TextStyle Comment = TextStyle{TextStyle::kKindComment};
/// Label renders text with a 'code' style that represents an 'label' token
static constexpr TextStyle Label = TextStyle{TextStyle::kKindLabel};
/// Instruction renders text with a 'code' style that represents an 'instruction' token
static constexpr TextStyle Instruction = TextStyle{TextStyle::kKindInstruction};
/// Squiggle renders text with a squiggle-highlight style (`^^^^^`)
static constexpr TextStyle Squiggle = TextStyle{TextStyle::kKindSquiggle};

}  // namespace tint::style

#endif  // SRC_TINT_UTILS_TEXT_TEXT_STYLE_H_
