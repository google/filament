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

#include "src/tint/lang/wgsl/reader/parser/lexer.h"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/strconv/parse_num.h"
#include "src/tint/utils/text/unicode.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint::wgsl::reader {
namespace {

// Unicode parsing code assumes that the size of a single std::string element is
// 1 byte.
static_assert(sizeof(decltype(tint::Source::FileContent::data[0])) == sizeof(uint8_t),
              "tint::wgsl::reader requires the size of a std::string element "
              "to be a single byte");

// A token is ~80bytes. The 4k here comes from looking at the number of tokens in the benchmark
// programs and being a bit bigger then those need (atan2-const-eval is the outlier here).
static constexpr size_t kDefaultListSize = 4092;

bool read_blankspace(std::string_view str,
                     size_t i,
                     bool* is_blankspace,
                     uint32_t* blankspace_size) {
    // See https://www.w3.org/TR/WGSL/#blankspace

    auto* utf8 = reinterpret_cast<const uint8_t*>(&str[i]);
    auto [cp, n] = tint::utf8::Decode(utf8, str.size() - i);

    if (n == 0) {
        return false;
    }

    static const auto kSpace = tint::CodePoint(0x0020);  // space
    static const auto kHTab = tint::CodePoint(0x0009);   // horizontal tab
    static const auto kL2R = tint::CodePoint(0x200E);    // left-to-right mark
    static const auto kR2L = tint::CodePoint(0x200F);    // right-to-left mark

    if (cp == kSpace || cp == kHTab || cp == kL2R || cp == kR2L) {
        *is_blankspace = true;
        *blankspace_size = static_cast<uint32_t>(n);
        return true;
    }

    *is_blankspace = false;
    return true;
}

uint32_t dec_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    return 0;
}

uint32_t hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return 0xA + static_cast<uint32_t>(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 0xA + static_cast<uint32_t>(c - 'A');
    }
    return 0;
}

}  // namespace

Lexer::Lexer(const Source::File* file) : file_(file), location_{1, 1} {}

Lexer::~Lexer() = default;

std::vector<Token> Lexer::Lex() {
    std::vector<Token> tokens;
    tokens.reserve(kDefaultListSize);

    while (true) {
        tokens.emplace_back(next());
        if (tokens.back().IsEof() || tokens.back().IsError()) {
            break;
        }

        // If the token can be split, we insert a placeholder element(s) into the stream to hold the
        // split character.
        size_t num_placeholders = tokens.back().NumPlaceholders();
        for (size_t i = 0; i < num_placeholders; i++) {
            auto src = tokens.back().source();
            src.range.begin.column++;
            tokens.emplace_back(Token::Type::kPlaceholder, src);
        }
    }
    return tokens;
}

std::string_view Lexer::line() const {
    if (file_->content.lines.size() == 0) {
        static const char* empty_string = "";
        return empty_string;
    }
    return file_->content.lines[location_.line - 1];
}

uint32_t Lexer::pos() const {
    return location_.column - 1;
}

uint32_t Lexer::length() const {
    return static_cast<uint32_t>(line().size());
}

const char& Lexer::at(uint32_t pos) const {
    auto l = line();
    // Unlike for std::string, if pos == l.size(), indexing `l[pos]` is UB for
    // std::string_view.
    if (pos >= l.size()) {
        static const char zero = 0;
        return zero;
    }
    return l[pos];
}

std::string_view Lexer::substr(uint32_t offset, uint32_t count) {
    return line().substr(offset, count);
}

void Lexer::advance(uint32_t offset) {
    location_.column += offset;
}

void Lexer::set_pos(uint32_t pos) {
    location_.column = pos + 1;
}

void Lexer::advance_line() {
    location_.line++;
    location_.column = 1;
}

bool Lexer::is_eof() const {
    return location_.line >= file_->content.lines.size() && pos() >= length();
}

bool Lexer::is_eol() const {
    return pos() >= length();
}

Token Lexer::next() {
    if (auto t = skip_blankspace_and_comments(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_hex_float(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_hex_integer(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_float(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_integer(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_ident(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    if (auto t = try_punctuation(); t.has_value() && !t->IsUninitialized()) {
        return std::move(t.value());
    }

    return {Token::Type::kError, begin_source(),
            (is_null() ? "null character found" : "invalid character found")};
}

Source Lexer::begin_source() const {
    Source src{};
    src.file = file_;
    src.range.begin = location_;
    src.range.end = location_;
    return src;
}

void Lexer::end_source(Source& src) const {
    src.range.end = location_;
}

bool Lexer::is_null() const {
    return (pos() < length()) && (at(pos()) == 0);
}

bool Lexer::is_digit(char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch));
}
bool Lexer::is_hex(char ch) const {
    return std::isxdigit(static_cast<unsigned char>(ch));
}

bool Lexer::matches(uint32_t pos, std::string_view sub_string) {
    if (pos >= length()) {
        return false;
    }
    return substr(pos, static_cast<uint32_t>(sub_string.size())) == sub_string;
}

bool Lexer::matches(uint32_t pos, char ch) {
    if (pos >= length()) {
        return false;
    }
    return line()[pos] == ch;
}

std::optional<Token> Lexer::skip_blankspace_and_comments() {
    for (;;) {
        auto loc = location_;
        while (!is_eof()) {
            if (is_eol()) {
                advance_line();
                continue;
            }

            bool is_blankspace;
            uint32_t blankspace_size;
            if (!read_blankspace(line(), pos(), &is_blankspace, &blankspace_size)) {
                return Token{Token::Type::kError, begin_source(), "invalid UTF-8"};
            }
            if (!is_blankspace) {
                break;
            }

            advance(blankspace_size);
        }

        auto t = skip_comment();
        if (t.has_value() && !t->IsUninitialized()) {
            return t;
        }

        // If the cursor didn't advance we didn't remove any blankspace
        // so we're done.
        if (loc == location_) {
            break;
        }
    }
    if (is_eof()) {
        return Token{Token::Type::kEOF, begin_source()};
    }

    return {};
}

std::optional<Token> Lexer::skip_comment() {
    if (matches(pos(), "//")) {
        // Line comment: ignore everything until the end of line.
        while (!is_eol()) {
            if (is_null()) {
                return Token{Token::Type::kError, begin_source(), "null character found"};
            }
            advance();
        }
        return {};
    }

    if (matches(pos(), "/*")) {
        // Block comment: ignore everything until the closing '*/' token.

        // Record source location of the initial '/*'
        auto source = begin_source();
        source.range.end.column += 1;

        advance(2);

        int depth = 1;
        while (!is_eof() && depth > 0) {
            if (matches(pos(), "/*")) {
                // Start of block comment: increase nesting depth.
                advance(2);
                depth++;
            } else if (matches(pos(), "*/")) {
                // End of block comment: decrease nesting depth.
                advance(2);
                depth--;
            } else if (is_eol()) {
                // Newline: skip and update source location.
                advance_line();
            } else if (is_null()) {
                return Token{Token::Type::kError, begin_source(), "null character found"};
            } else {
                // Anything else: skip and update source location.
                advance();
            }
        }
        if (depth > 0) {
            return Token{Token::Type::kError, source, "unterminated block comment"};
        }
    }
    return {};
}

std::optional<Token> Lexer::try_float() {
    auto start = pos();
    auto end = pos();

    auto source = begin_source();
    bool has_mantissa_digits = false;

    std::optional<size_t> first_significant_digit_position;
    while (end < length() && is_digit(at(end))) {
        if (!first_significant_digit_position.has_value() && (at(end) != '0')) {
            first_significant_digit_position = end;
        }

        has_mantissa_digits = true;
        end++;
    }

    std::optional<size_t> dot_position;
    if (end < length() && matches(end, '.')) {
        dot_position = end;
        end++;
    }

    size_t zeros_before_digit = 0;
    while (end < length() && is_digit(at(end))) {
        if (!first_significant_digit_position.has_value()) {
            if (at(end) == '0') {
                zeros_before_digit += 1;
            } else {
                first_significant_digit_position = end;
            }
        }

        has_mantissa_digits = true;
        end++;
    }

    if (!has_mantissa_digits) {
        return {};
    }

    // Parse the exponent if one exists
    std::optional<uint32_t> exponent_value_position;
    bool negative_exponent = false;
    if (end < length() && (matches(end, 'e') || matches(end, 'E'))) {
        end++;
        if (end < length() && (matches(end, '+') || matches(end, '-'))) {
            negative_exponent = matches(end, '-');
            end++;
        }
        exponent_value_position = end;

        bool has_digits = false;
        while (end < length() && isdigit(at(end))) {
            has_digits = true;
            end++;
        }

        // If an 'e' or 'E' was present, then the number part must also be present.
        if (!has_digits) {
            const auto str = std::string{substr(start, end - start)};
            return Token{Token::Type::kError, source,
                         "incomplete exponent for floating point literal: " + str};
        }
    }

    bool has_f_suffix = false;
    bool has_h_suffix = false;
    if (end < length() && matches(end, 'f')) {
        has_f_suffix = true;
    } else if (end < length() && matches(end, 'h')) {
        has_h_suffix = true;
    }

    if (!dot_position.has_value() && !exponent_value_position.has_value() && !has_f_suffix &&
        !has_h_suffix) {
        // If it only has digits then it's an integer.
        return {};
    }

    // Note, the `at` method will return a static `0` if the provided position is >= length. We
    // actually need the end pointer to point to the correct memory location to use `from_chars`.
    // So, handle the case where we point past the length specially.
    auto* end_ptr = &at(end);
    if (end >= length()) {
        end_ptr = &at(length() - 1) + 1;
    }

    auto ret = tint::strconv::ParseDouble(std::string_view(&at(start), end - start));
    double value = ret == Success ? ret.Get() : 0.0;
    bool overflow =
        ret != Success && ret.Failure() == tint::strconv::ParseNumberError::kResultOutOfRange;

    // If the value didn't fit in a double, check for underflow as that is 0.0 in WGSL and not an
    // error.
    if (overflow) {
        // The exponent is negative, so treat as underflow
        if (negative_exponent) {
            overflow = false;
            value = 0.0;
        } else if (dot_position.has_value() && first_significant_digit_position.has_value() &&
                   first_significant_digit_position.value() > dot_position.value()) {
            // Parse the exponent from the float if provided
            size_t exp_value = 0;
            bool exp_conversion_succeeded = true;
            if (exponent_value_position.has_value()) {
                auto exp_end_ptr = end_ptr - (has_f_suffix || has_h_suffix ? 1 : 0);
                auto exp_ret = std::from_chars(&at(exponent_value_position.value()), exp_end_ptr,
                                               exp_value, 10);

                if (exp_ret.ec != std::errc{}) {
                    exp_conversion_succeeded = false;
                }
            }
            // If the exponent has gone negative, then this is an underflow case
            if (exp_conversion_succeeded && exp_value < zeros_before_digit) {
                overflow = false;
                value = 0.0;
            }
        }
    }

    advance(end - start);

    if (has_f_suffix) {
        auto f = core::CheckedConvert<f32>(AFloat(value));
        if (!overflow && f == Success) {
            advance(1);
            end_source(source);
            return Token{Token::Type::kFloatLiteral_F, source, static_cast<double>(f.Get())};
        }
        return Token{Token::Type::kError, source, "value cannot be represented as 'f32'"};
    }

    if (has_h_suffix) {
        auto f = core::CheckedConvert<f16>(AFloat(value));
        if (!overflow && f == Success) {
            advance(1);
            end_source(source);
            return Token{Token::Type::kFloatLiteral_H, source, static_cast<double>(f.Get())};
        }
        return Token{Token::Type::kError, source, "value cannot be represented as 'f16'"};
    }

    end_source(source);

    TINT_BEGIN_DISABLE_WARNING(FLOAT_EQUAL);
    if (overflow || value == HUGE_VAL || -value == HUGE_VAL) {
        return Token{Token::Type::kError, source,
                     "value cannot be represented as 'abstract-float'"};
    } else {
        return Token{Token::Type::kFloatLiteral, source, value};
    }
    TINT_END_DISABLE_WARNING(FLOAT_EQUAL);
}

std::optional<Token> Lexer::try_hex_float() {
    constexpr uint64_t kExponentBits = 11;
    constexpr uint64_t kMantissaBits = 52;
    constexpr uint64_t kTotalBits = 1 + kExponentBits + kMantissaBits;
    constexpr uint64_t kTotalMsb = kTotalBits - 1;
    constexpr uint64_t kMantissaMsb = kMantissaBits - 1;
    constexpr uint64_t kMantissaShiftRight = kTotalBits - kMantissaBits;
    constexpr int64_t kExponentBias = 1023;
    constexpr uint64_t kExponentMask = (1 << kExponentBits) - 1;
    constexpr int64_t kExponentMax = kExponentMask;  // Including NaN / inf
    constexpr uint64_t kExponentLeftShift = kMantissaBits;
    constexpr uint64_t kOne = 1;

    auto start = pos();
    auto end = pos();

    auto source = begin_source();

    // 0[xX]([0-9a-fA-F]*.?[0-9a-fA-F]+ | [0-9a-fA-F]+.[0-9a-fA-F]*)(p|P)(+|-)?[0-9]+  // NOLINT

    // 0[xX]
    if (matches(end, '0') && (matches(end + 1, 'x') || matches(end + 1, 'X'))) {
        end += 2;
    } else {
        return {};
    }

    uint64_t mantissa = 0;
    uint64_t exponent = 0;

    // TODO(dneto): Values in the normal range for the format do not explicitly
    // store the most significant bit.  The algorithm here works hard to eliminate
    // that bit in the representation during parsing, and then it backtracks
    // when it sees it may have to explicitly represent it, and backtracks again
    // when it sees the number is sub-normal (i.e. the exponent underflows).
    // I suspect the logic can be clarified by storing it during parsing, and
    // then removing it later only when needed.

    // `set_next_mantissa_bit_to` sets next `mantissa` bit starting from msb to
    // lsb to value 1 if `set` is true, 0 otherwise. Returns true on success, i.e.
    // when the bit can be accommodated in the available space.
    uint64_t mantissa_next_bit = kTotalMsb;
    auto set_next_mantissa_bit_to = [&](bool set, bool integer_part) -> bool {
        // If adding bits for the integer part, we can overflow whether we set the
        // bit or not. For the fractional part, we can only overflow when setting
        // the bit.
        const bool check_overflow = integer_part || set;
        // Note: mantissa_next_bit actually decrements, so comparing it as
        // larger than a positive number relies on wraparound.
        if (check_overflow && (mantissa_next_bit > kTotalMsb)) {
            return false;  // Overflowed mantissa
        }
        if (set) {
            mantissa |= (kOne << mantissa_next_bit);
        }
        --mantissa_next_bit;
        return true;
    };

    // Collect integer range (if any)
    auto integer_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        integer_range.second = ++end;
    }

    // .?
    bool hex_point = false;
    if (matches(end, '.')) {
        hex_point = true;
        end++;
    }

    // Collect fractional range (if any)
    auto fractional_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        fractional_range.second = ++end;
    }

    // Must have at least an integer or fractional part
    if ((integer_range.first == integer_range.second) &&
        (fractional_range.first == fractional_range.second)) {
        return {};
    }

    // Is the binary exponent present?  It's optional.
    const bool has_exponent = (matches(end, 'p') || matches(end, 'P'));
    if (has_exponent) {
        end++;
    }
    if (!has_exponent && !hex_point) {
        // It's not a hex float. At best it's a hex integer.
        return {};
    }

    // At this point, we know for sure our token is a hex float value,
    // or an invalid token.

    // Parse integer part
    // [0-9a-fA-F]*

    bool has_zero_integer = true;
    // The magnitude is zero if and only if seen_prior_one_bits is false.
    bool seen_prior_one_bits = false;
    for (auto i = integer_range.first; i < integer_range.second; ++i) {
        const auto nibble = hex_value(at(i));
        if (nibble != 0) {
            has_zero_integer = false;
        }

        for (int bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            // Skip leading 0s and the first 1
            if (seen_prior_one_bits) {
                if (!set_next_mantissa_bit_to(v != 0, true)) {
                    return Token{Token::Type::kError, source,
                                 "mantissa is too large for hex float"};
                }
                ++exponent;
            } else {
                if (v == 1) {
                    seen_prior_one_bits = true;
                }
            }
        }
    }

    // Parse fractional part
    // [0-9a-fA-F]*
    for (auto i = fractional_range.first; i < fractional_range.second; ++i) {
        auto nibble = hex_value(at(i));
        for (int bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            if (v == 1) {
                seen_prior_one_bits = true;
            }

            // If integer part is 0, we only start writing bits to the
            // mantissa once we have a non-zero fractional bit. While the fractional
            // values are 0, we adjust the exponent to avoid overflowing `mantissa`.
            if (!seen_prior_one_bits) {
                --exponent;
            } else {
                if (!set_next_mantissa_bit_to(v != 0, false)) {
                    return Token{Token::Type::kError, source,
                                 "mantissa is too large for hex float"};
                }
            }
        }
    }

    // Determine if the value of the mantissa is zero.
    // Note: it's not enough to check mantissa == 0 as we drop the initial bit,
    // whether it's in the integer part or the fractional part.
    const bool is_zero = !seen_prior_one_bits;
    TINT_ASSERT(!is_zero || mantissa == 0);

    // Parse the optional exponent.
    // ((p|P)(\+|-)?[0-9]+)?
    uint64_t input_exponent = 0;  // Defaults to 0 if not present
    int64_t exponent_sign = 1;
    // If the 'p' part is present, the rest of the exponent must exist.
    bool has_f_suffix = false;
    bool has_h_suffix = false;
    if (has_exponent) {
        // Parse the rest of the exponent.
        // (+|-)?
        if (matches(end, '+')) {
            end++;
        } else if (matches(end, '-')) {
            exponent_sign = -1;
            end++;
        }

        // Parse exponent from input
        // [0-9]+
        // Allow overflow (in uint64_t) when the floating point value magnitude is
        // zero.
        bool has_exponent_digits = false;
        while (end < length() && isdigit(at(end))) {
            has_exponent_digits = true;
            auto prev_exponent = input_exponent;
            input_exponent = (input_exponent * 10) + dec_value(at(end));
            // Check if we've overflowed input_exponent. This only matters when
            // the mantissa is non-zero.
            if (!is_zero && (prev_exponent > input_exponent)) {
                return Token{Token::Type::kError, source, "exponent is too large for hex float"};
            }
            end++;
        }

        // Parse optional 'f' or 'h' suffix.  For a hex float, it can only exist
        // when the exponent is present. Otherwise it will look like
        // one of the mantissa digits.
        if (end < length() && matches(end, 'f')) {
            has_f_suffix = true;
            end++;
        } else if (end < length() && matches(end, 'h')) {
            has_h_suffix = true;
            end++;
        }

        if (!has_exponent_digits) {
            return Token{Token::Type::kError, source, "expected an exponent value for hex float"};
        }
    }

    advance(end - start);
    end_source(source);

    if (is_zero) {
        // If value is zero, then ignore the exponent and produce a zero
        exponent = 0;
    } else {
        // Ensure input exponent is not too large; i.e. that it won't overflow when
        // adding the exponent bias.
        const uint64_t kIntMax = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
        const uint64_t kMaxInputExponent = kIntMax - kExponentBias;
        if (input_exponent > kMaxInputExponent) {
            return Token{Token::Type::kError, source, "exponent is too large for hex float"};
        }

        // Compute exponent so far
        exponent += static_cast<uint64_t>(static_cast<int64_t>(input_exponent) * exponent_sign);

        // Bias exponent if non-zero
        // After this, if exponent is <= 0, our value is a denormal
        exponent += kExponentBias;

        // We know the number is not zero.  The MSB is 1 (by construction), and
        // should be eliminated because it becomes the implicit 1 that isn't
        // explicitly represented in the binary32 format.  We'll bring it back
        // later if we find the exponent actually underflowed, i.e. the number
        // is sub-normal.
        if (has_zero_integer) {
            mantissa <<= 1;
            --exponent;
        }
    }

    // We can now safely work with exponent as a signed quantity, as there's no
    // chance to overflow
    int64_t signed_exponent = static_cast<int64_t>(exponent);

    // Shift mantissa to occupy the low 23 bits
    mantissa >>= kMantissaShiftRight;

    // If denormal, shift mantissa until our exponent is zero
    if (!is_zero) {
        // Denorm has exponent 0 and non-zero mantissa. We set the top bit here,
        // then shift the mantissa to make exponent zero.
        if (signed_exponent <= 0) {
            mantissa >>= 1;
            mantissa |= (kOne << kMantissaMsb);
        }

        while (signed_exponent < 0) {
            mantissa >>= 1;
            ++signed_exponent;

            // If underflow, clamp to zero
            if (mantissa == 0) {
                signed_exponent = 0;
            }
        }
    }

    if (signed_exponent >= kExponentMax || (signed_exponent == kExponentMax && mantissa != 0)) {
        std::string type = has_f_suffix ? "f32" : (has_h_suffix ? "f16" : "abstract-float");
        return Token{Token::Type::kError, source, "value cannot be represented as '" + type + "'"};
    }

    // Combine sign, mantissa, and exponent
    uint64_t result_u64 = 0;
    result_u64 |= mantissa;
    result_u64 |= (static_cast<uint64_t>(signed_exponent) & kExponentMask) << kExponentLeftShift;

    // Reinterpret as f16 and return
    double result_f64;
    std::memcpy(&result_f64, &result_u64, 8);

    if (has_f_suffix) {
        // Check value fits in f32
        if (result_f64 < static_cast<double>(f32::kLowestValue) ||
            result_f64 > static_cast<double>(f32::kHighestValue)) {
            return Token{Token::Type::kError, source, "value cannot be represented as 'f32'"};
        }
        // Check the value can be exactly represented, i.e. only high 23 mantissa bits are valid for
        // normal f32 values, and less for subnormal f32 values. The rest low mantissa bits must be
        // 0.
        int valid_mantissa_bits = 0;
        double abs_result_f64 = std::fabs(result_f64);
        if (abs_result_f64 >= static_cast<double>(f32::kSmallestValue)) {
            // The result shall be a normal f32 value.
            valid_mantissa_bits = 23;
        } else if (abs_result_f64 >= static_cast<double>(f32::kSmallestSubnormalValue)) {
            // The result shall be a subnormal f32 value, represented as double.
            // The smallest positive normal f32 is f32::kSmallestValue = 2^-126 = 0x1.0p-126, and
            // the
            //   smallest positive subnormal f32 is f32::kSmallestSubnormalValue = 2^-149. Thus, the
            //   value v in range 2^-126 > v >= 2^-149 must be represented as a subnormal f32
            //   number, but is still normal double (f64) number, and has a exponent in range -127
            //   to -149, inclusive.
            // A value v, if 2^-126 > v >= 2^-127, its binary32 representation will have binary form
            //   s_00000000_1xxxxxxxxxxxxxxxxxxxxxx, having mantissa of 1 leading 1 bit and 22
            //   arbitrary bits. Since this value is represented as normal double number, the
            //   leading 1 bit is omitted, only the highest 22 mantissia bits can be arbitrary, and
            //   the rest lowest 40 mantissa bits of f64 number must be zero.
            // 2^-127 > v >= 2^-128, binary32 s_00000000_01xxxxxxxxxxxxxxxxxxxxx, having mantissa of
            //   1 leading 0 bit, 1 leading 1 bit, and 21 arbitrary bits. The f64 representation
            //   omits the leading 0 and 1 bits, and only the highest 21 mantissia bits can be
            //   arbitrary.
            // 2^-128 > v >= 2^-129, binary32 s_00000000_001xxxxxxxxxxxxxxxxxxxx, 20 arbitrary bits.
            // ...
            // 2^-147 > v >= 2^-148, binary32 s_00000000_0000000000000000000001x, 1 arbitrary bit.
            // 2^-148 > v >= 2^-149, binary32 s_00000000_00000000000000000000001, 0 arbitrary bit.

            // signed_exponent must be in range -149 + 1023 = 874 to -127 + 1023 = 896, inclusive
            TINT_ASSERT((874 <= signed_exponent) && (signed_exponent <= 896));
            int unbiased_exponent =
                static_cast<int>(signed_exponent) - static_cast<int>(kExponentBias);
            TINT_ASSERT((-149 <= unbiased_exponent) && (unbiased_exponent <= -127));
            valid_mantissa_bits = unbiased_exponent + 149;  // 0 for -149, and 22 for -127
        } else if (abs_result_f64 != 0.0) {
            // The result is smaller than the smallest subnormal f32 value, but not equal to zero.
            // Such value will never be exactly represented by f32.
            return Token{Token::Type::kError, source,
                         "value cannot be exactly represented as 'f32'"};
        }
        // Check the low 52-valid_mantissa_bits mantissa bits must be 0.
        TINT_ASSERT((0 <= valid_mantissa_bits) && (valid_mantissa_bits <= 23));
        if (result_u64 & ((uint64_t(1) << (52 - valid_mantissa_bits)) - 1)) {
            return Token{Token::Type::kError, source,
                         "value cannot be exactly represented as 'f32'"};
        }
        return Token{Token::Type::kFloatLiteral_F, source, result_f64};
    } else if (has_h_suffix) {
        // Check value fits in f16
        if (result_f64 < static_cast<double>(f16::kLowestValue) ||
            result_f64 > static_cast<double>(f16::kHighestValue)) {
            return Token{Token::Type::kError, source, "value cannot be represented as 'f16'"};
        }
        // Check the value can be exactly represented, i.e. only high 10 mantissa bits are valid for
        // normal f16 values, and less for subnormal f16 values. The rest low mantissa bits must be
        // 0.
        int valid_mantissa_bits = 0;
        double abs_result_f64 = std::fabs(result_f64);
        if (abs_result_f64 >= static_cast<double>(f16::kSmallestValue)) {
            // The result shall be a normal f16 value.
            valid_mantissa_bits = 10;
        } else if (abs_result_f64 >= static_cast<double>(f16::kSmallestSubnormalValue)) {
            // The result shall be a subnormal f16 value, represented as double.
            // The smallest positive normal f16 is f16::kSmallestValue = 2^-14 = 0x1.0p-14, and the
            //   smallest positive subnormal f16 is f16::kSmallestSubnormalValue = 2^-24. Thus, the
            //   value v in range 2^-14 > v >= 2^-24 must be represented as a subnormal f16 number,
            //   but is still normal double (f64) number, and has a exponent in range -15 to -24,
            //   inclusive.
            // A value v, if 2^-14 > v >= 2^-15, its binary16 representation will have binary form
            //   s_00000_1xxxxxxxxx, having mantissa of 1 leading 1 bit and 9 arbitrary bits. Since
            //   this value is represented as normal double number, the leading 1 bit is omitted,
            //   only the highest 9 mantissia bits can be arbitrary, and the rest lowest 43 mantissa
            //   bits of f64 number must be zero.
            // 2^-15 > v >= 2^-16, binary16 s_00000_01xxxxxxxx, having mantissa of 1 leading 0 bit,
            //   1 leading 1 bit, and 8 arbitrary bits. The f64 representation omits the leading 0
            //   and 1 bits, and only the highest 8 mantissia bits can be arbitrary.
            // 2^-16 > v >= 2^-17, binary16 s_00000_001xxxxxxx, 7 arbitrary bits.
            // ...
            // 2^-22 > v >= 2^-23, binary16 s_00000_000000001x, 1 arbitrary bits.
            // 2^-23 > v >= 2^-24, binary16 s_00000_0000000001, 0 arbitrary bits.

            // signed_exponent must be in range -24 + 1023 = 999 to -15 + 1023 = 1008, inclusive
            TINT_ASSERT((999 <= signed_exponent) && (signed_exponent <= 1008));
            int unbiased_exponent =
                static_cast<int>(signed_exponent) - static_cast<int>(kExponentBias);
            TINT_ASSERT((-24 <= unbiased_exponent) && (unbiased_exponent <= -15));
            valid_mantissa_bits = unbiased_exponent + 24;  // 0 for -24, and 9 for -15
        } else if (abs_result_f64 != 0.0) {
            // The result is smaller than the smallest subnormal f16 value, but not equal to zero.
            // Such value will never be exactly represented by f16.
            return Token{Token::Type::kError, source,
                         "value cannot be exactly represented as 'f16'"};
        }
        // Check the low 52-valid_mantissa_bits mantissa bits must be 0.
        TINT_ASSERT((0 <= valid_mantissa_bits) && (valid_mantissa_bits <= 10));
        if (result_u64 & ((uint64_t(1) << (52 - valid_mantissa_bits)) - 1)) {
            return Token{Token::Type::kError, source,
                         "value cannot be exactly represented as 'f16'"};
        }
        return Token{Token::Type::kFloatLiteral_H, source, result_f64};
    }

    return Token{Token::Type::kFloatLiteral, source, result_f64};
}

Token Lexer::build_token_from_int_if_possible(Source source,
                                              uint32_t start,
                                              uint32_t prefix_count,
                                              int32_t base) {
    const char* start_ptr = &at(start);
    // The call to `from_chars` will return the pointer to just after the last parsed character.
    // We also need to tell it the maximum end character to parse. So, instead of walking all the
    // characters to find the last possible and using that, we just provide the end of the string.
    // We then calculate the count based off the provided end pointer and the start pointer. The
    // extra `prefix_count` is to handle a `0x` which is not included in the `start` value.
    const char* end_ptr = &at(length() - 1) + 1;

    int64_t value = 0;
    auto res = std::from_chars(start_ptr, end_ptr, value, base);
    const bool overflow = res.ec != std::errc();
    advance(static_cast<uint32_t>(res.ptr - start_ptr) + prefix_count);

    if (matches(pos(), 'u')) {
        if (!overflow && core::CheckedConvert<u32>(AInt(value)) == Success) {
            advance(1);
            end_source(source);
            return {Token::Type::kIntLiteral_U, source, value};
        }
        return {Token::Type::kError, source, "value cannot be represented as 'u32'"};
    }

    if (matches(pos(), 'i')) {
        if (!overflow && core::CheckedConvert<i32>(AInt(value)) == Success) {
            advance(1);
            end_source(source);
            return {Token::Type::kIntLiteral_I, source, value};
        }
        return {Token::Type::kError, source, "value cannot be represented as 'i32'"};
    }

    // Check this last in order to get the more specific sized error messages
    if (overflow) {
        return {Token::Type::kError, source, "value cannot be represented as 'abstract-int'"};
    }

    end_source(source);
    return {Token::Type::kIntLiteral, source, value};
}

std::optional<Token> Lexer::try_hex_integer() {
    auto start = pos();
    auto curr = start;

    auto source = begin_source();

    if (matches(curr, '0') && (matches(curr + 1, 'x') || matches(curr + 1, 'X'))) {
        curr += 2;
    } else {
        return {};
    }

    if (!is_hex(at(curr))) {
        return Token{Token::Type::kError, source,
                     "integer or float hex literal has no significant digits"};
    }

    return build_token_from_int_if_possible(source, curr, curr - start, 16);
}

std::optional<Token> Lexer::try_integer() {
    auto start = pos();
    auto curr = start;

    auto source = begin_source();

    if (curr >= length() || !is_digit(at(curr))) {
        return {};
    }

    // If the first digit is a zero this must only be zero as leading zeros
    // are not allowed.
    if (auto next = curr + 1; next < length()) {
        if (at(curr) == '0' && is_digit(at(next))) {
            return Token{Token::Type::kError, source, "integer literal cannot have leading 0s"};
        }
    }

    return build_token_from_int_if_possible(source, start, 0, 10);
}

std::optional<Token> Lexer::try_ident() {
    auto source = begin_source();
    auto start = pos();

    // Must begin with an XID_Source unicode character, or underscore
    {
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = tint::utf8::Decode(utf8, length() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return Token{Token::Type::kError, source, "invalid UTF-8"};
        }
        if (code_point != tint::CodePoint('_') && !code_point.IsXIDStart()) {
            return {};
        }
        // Consume start codepoint
        advance(static_cast<uint32_t>(n));
    }

    while (!is_eol()) {
        // Must continue with an XID_Continue unicode character
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = tint::utf8::Decode(utf8, line().size() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return Token{Token::Type::kError, source, "invalid UTF-8"};
        }
        if (!code_point.IsXIDContinue()) {
            break;
        }

        // Consume continuing codepoint
        advance(static_cast<uint32_t>(n));
    }

    auto str = substr(start, pos() - start);
    end_source(source);

    if (str.length() > 1 && substr(start, 2) == "__") {
        // Identifiers prefixed with two or more underscores are not allowed.
        return Token{Token::Type::kError, source,
                     "identifiers must not start with two or more underscores"};
    }

    if (auto t = parse_keyword(str); t.has_value()) {
        return Token{t.value(), source, str};
    }

    return Token{Token::Type::kIdentifier, source, str};
}

std::optional<Token> Lexer::try_punctuation() {
    auto source = begin_source();
    auto type = Token::Type::kUninitialized;

    if (matches(pos(), '@')) {
        type = Token::Type::kAttr;
        advance(1);
    } else if (matches(pos(), '(')) {
        type = Token::Type::kParenLeft;
        advance(1);
    } else if (matches(pos(), ')')) {
        type = Token::Type::kParenRight;
        advance(1);
    } else if (matches(pos(), '[')) {
        type = Token::Type::kBracketLeft;
        advance(1);
    } else if (matches(pos(), ']')) {
        type = Token::Type::kBracketRight;
        advance(1);
    } else if (matches(pos(), '{')) {
        type = Token::Type::kBraceLeft;
        advance(1);
    } else if (matches(pos(), '}')) {
        type = Token::Type::kBraceRight;
        advance(1);
    } else if (matches(pos(), '&')) {
        if (matches(pos() + 1, '&')) {
            type = Token::Type::kAndAnd;
            advance(2);
        } else if (matches(pos() + 1, '=')) {
            type = Token::Type::kAndEqual;
            advance(2);
        } else {
            type = Token::Type::kAnd;
            advance(1);
        }
    } else if (matches(pos(), '/')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kDivisionEqual;
            advance(2);
        } else {
            type = Token::Type::kForwardSlash;
            advance(1);
        }
    } else if (matches(pos(), '!')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kNotEqual;
            advance(2);
        } else {
            type = Token::Type::kBang;
            advance(1);
        }
    } else if (matches(pos(), ':')) {
        type = Token::Type::kColon;
        advance(1);
    } else if (matches(pos(), ',')) {
        type = Token::Type::kComma;
        advance(1);
    } else if (matches(pos(), '=')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kEqualEqual;
            advance(2);
        } else {
            type = Token::Type::kEqual;
            advance(1);
        }
    } else if (matches(pos(), '>')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kGreaterThanEqual;
            advance(2);
        } else if (matches(pos() + 1, '>')) {
            if (matches(pos() + 2, '=')) {
                type = Token::Type::kShiftRightEqual;
                advance(3);
            } else {
                type = Token::Type::kShiftRight;
                advance(2);
            }
        } else {
            type = Token::Type::kGreaterThan;
            advance(1);
        }
    } else if (matches(pos(), '<')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kLessThanEqual;
            advance(2);
        } else if (matches(pos() + 1, '<')) {
            if (matches(pos() + 2, '=')) {
                type = Token::Type::kShiftLeftEqual;
                advance(3);
            } else {
                type = Token::Type::kShiftLeft;
                advance(2);
            }
        } else {
            type = Token::Type::kLessThan;
            advance(1);
        }
    } else if (matches(pos(), '%')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kModuloEqual;
            advance(2);
        } else {
            type = Token::Type::kMod;
            advance(1);
        }
    } else if (matches(pos(), '-')) {
        if (matches(pos() + 1, '>')) {
            type = Token::Type::kArrow;
            advance(2);
        } else if (matches(pos() + 1, '-')) {
            type = Token::Type::kMinusMinus;
            advance(2);
        } else if (matches(pos() + 1, '=')) {
            type = Token::Type::kMinusEqual;
            advance(2);
        } else {
            type = Token::Type::kMinus;
            advance(1);
        }
    } else if (matches(pos(), '.')) {
        type = Token::Type::kPeriod;
        advance(1);
    } else if (matches(pos(), '+')) {
        if (matches(pos() + 1, '+')) {
            type = Token::Type::kPlusPlus;
            advance(2);
        } else if (matches(pos() + 1, '=')) {
            type = Token::Type::kPlusEqual;
            advance(2);
        } else {
            type = Token::Type::kPlus;
            advance(1);
        }
    } else if (matches(pos(), '|')) {
        if (matches(pos() + 1, '|')) {
            type = Token::Type::kOrOr;
            advance(2);
        } else if (matches(pos() + 1, '=')) {
            type = Token::Type::kOrEqual;
            advance(2);
        } else {
            type = Token::Type::kOr;
            advance(1);
        }
    } else if (matches(pos(), ';')) {
        type = Token::Type::kSemicolon;
        advance(1);
    } else if (matches(pos(), '*')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kTimesEqual;
            advance(2);
        } else {
            type = Token::Type::kStar;
            advance(1);
        }
    } else if (matches(pos(), '~')) {
        type = Token::Type::kTilde;
        advance(1);
    } else if (matches(pos(), '_')) {
        type = Token::Type::kUnderscore;
        advance(1);
    } else if (matches(pos(), '^')) {
        if (matches(pos() + 1, '=')) {
            type = Token::Type::kXorEqual;
            advance(2);
        } else {
            type = Token::Type::kXor;
            advance(1);
        }
    } else {
        return {};
    }

    end_source(source);

    return Token{type, source};
}

std::optional<Token::Type> Lexer::parse_keyword(std::string_view str) {
    if (str == "alias") {
        return Token::Type::kAlias;
    }
    if (str == "break") {
        return Token::Type::kBreak;
    }
    if (str == "case") {
        return Token::Type::kCase;
    }
    if (str == "const") {
        return Token::Type::kConst;
    }
    if (str == "const_assert") {
        return Token::Type::kConstAssert;
    }
    if (str == "continue") {
        return Token::Type::kContinue;
    }
    if (str == "continuing") {
        return Token::Type::kContinuing;
    }
    if (str == "diagnostic") {
        return Token::Type::kDiagnostic;
    }
    if (str == "discard") {
        return Token::Type::kDiscard;
    }
    if (str == "default") {
        return Token::Type::kDefault;
    }
    if (str == "else") {
        return Token::Type::kElse;
    }
    if (str == "enable") {
        return Token::Type::kEnable;
    }
    if (str == "fallthrough") {
        return Token::Type::kFallthrough;
    }
    if (str == "false") {
        return Token::Type::kFalse;
    }
    if (str == "fn") {
        return Token::Type::kFn;
    }
    if (str == "for") {
        return Token::Type::kFor;
    }
    if (str == "if") {
        return Token::Type::kIf;
    }
    if (str == "let") {
        return Token::Type::kLet;
    }
    if (str == "loop") {
        return Token::Type::kLoop;
    }
    if (str == "override") {
        return Token::Type::kOverride;
    }
    if (str == "return") {
        return Token::Type::kReturn;
    }
    if (str == "requires") {
        return Token::Type::kRequires;
    }
    if (str == "struct") {
        return Token::Type::kStruct;
    }
    if (str == "switch") {
        return Token::Type::kSwitch;
    }
    if (str == "true") {
        return Token::Type::kTrue;
    }
    if (str == "var") {
        return Token::Type::kVar;
    }
    if (str == "while") {
        return Token::Type::kWhile;
    }
    if (str == "_") {
        return Token::Type::kUnderscore;
    }
    return std::nullopt;
}

}  // namespace tint::wgsl::reader

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
