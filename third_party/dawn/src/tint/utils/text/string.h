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

#ifndef SRC_TINT_UTILS_TEXT_STRING_H_
#define SRC_TINT_UTILS_TEXT_STRING_H_

#include <string>
#include <variant>

#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/text_style.h"

/// Forward declaration
namespace tint {
class StyledText;
}

namespace tint {

/// @param str the string to apply replacements to
/// @param substr the string to search for
/// @param replacement the replacement string to use instead of `substr`
/// @returns `str` with all occurrences of `substr` replaced with `replacement`
[[nodiscard]] inline std::string ReplaceAll(std::string str,
                                            std::string_view substr,
                                            std::string_view replacement) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string_view::npos) {
        str.replace(pos, substr.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

/// @copydoc ReplaceAll(std::string, std::string_view, std::string_view)
[[nodiscard]] inline std::string ReplaceAll(std::string_view str,
                                            std::string_view substr,
                                            std::string_view replacement) {
    return ReplaceAll(std::string(str), substr, replacement);
}

/// @copydoc ReplaceAll(std::string, std::string_view, std::string_view)
[[nodiscard]] inline std::string ReplaceAll(const char* str,
                                            std::string_view substr,
                                            std::string_view replacement) {
    return ReplaceAll(std::string(str), substr, replacement);
}

/// @param value the boolean value to be printed as a string
/// @returns value printed as a string via the stream `<<` operator
inline std::string ToString(bool value) {
    return value ? "true" : "false";
}

/// @param value the value to be printed as a string
/// @returns value printed as a string via the stream `<<` operator
template <typename T>
std::string ToString(const T& value) {
    StringStream s;
    s << value;
    return s.str();
}

/// @param value the variant to be printed as a string
/// @returns value printed as a string via the stream `<<` operator
template <typename... TYs>
std::string ToString(const std::variant<TYs...>& value) {
    StringStream s;
    s << std::visit([&](auto& v) { return ToString(v); }, value);
    return s.str();
}

/// @param str the input string
/// @param prefix the prefix string
/// @returns true iff @p str has the prefix @p prefix
inline bool HasPrefix(std::string_view str, std::string_view prefix) {
    return str.length() >= prefix.length() && str.substr(0, prefix.length()) == prefix;
}

/// @param str the input string
/// @param suffix the suffix string
/// @returns true iff @p str has the suffix @p suffix
inline bool HasSuffix(std::string_view str, std::string_view suffix) {
    return str.length() >= suffix.length() && str.substr(str.length() - suffix.length()) == suffix;
}

/// @param a the first string
/// @param b the second string
/// @returns the Levenshtein distance between @p a and @p b
size_t Distance(std::string_view a, std::string_view b);

/// Options for SuggestAlternatives()
struct SuggestAlternativeOptions {
    /// The prefix to apply to the strings when printing
    std::string_view prefix;
    /// The text style for alternatives
    TextStyle alternatives_style = style::Code;
    /// List all the possible values
    bool list_possible_values = true;
};

/// Suggest alternatives for an unrecognized string from a list of possible values.
/// @param got the unrecognized string
/// @param strings the list of possible values
/// @param ss the stream to write the suggest and list of possible values to
/// @param options options for the suggestion
void SuggestAlternatives(std::string_view got,
                         Slice<const std::string_view> strings,
                         StyledText& ss,
                         const SuggestAlternativeOptions& options = {});

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the start of
/// the string.
template <typename PREDICATE>
std::string_view TrimLeft(std::string_view str, PREDICATE&& pred) {
    while (!str.empty() && pred(str.front())) {
        str = str.substr(1);
    }
    return str;
}

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the end of
/// the string.
template <typename PREDICATE>
std::string_view TrimRight(std::string_view str, PREDICATE&& pred) {
    while (!str.empty() && pred(str.back())) {
        str = str.substr(0, str.length() - 1);
    }
    return str;
}

/// @param str the input string
/// @param prefix the prefix to trim from @p str
/// @return @p str with the prefix removed, if @p str has the prefix.
inline std::string_view TrimPrefix(std::string_view str, std::string_view prefix) {
    return HasPrefix(str, prefix) ? str.substr(prefix.length()) : str;
}

/// @param str the input string
/// @param suffix the suffix to trim from @p str
/// @return @p str with the suffix removed, if @p str has the suffix.
inline std::string_view TrimSuffix(std::string_view str, std::string_view suffix) {
    return HasSuffix(str, suffix) ? str.substr(0, str.length() - suffix.length()) : str;
}

/// @param str the input string
/// @param pred the predicate function
/// @return @p str with characters passing the predicate function @p pred removed from the start and
/// end of the string.
template <typename PREDICATE>
std::string_view Trim(std::string_view str, PREDICATE&& pred) {
    return TrimLeft(TrimRight(str, pred), pred);
}

/// @param c the character to test
/// @returns true if @p c is one of the following:
/// * space
/// * form feed
/// * line feed
/// * carriage return
/// * horizontal tab
/// * vertical tab
inline bool IsSpace(char c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

/// @param str the input string
/// @return @p str with all whitespace (' ') removed from the start and end of the string.
inline std::string_view TrimSpace(std::string_view str) {
    return Trim(str, IsSpace);
}

/// @param str the input string
/// @param delimiter the delimiter
/// @return @p str split at each occurrence of @p delimiter
inline Vector<std::string_view, 8> Split(std::string_view str, std::string_view delimiter) {
    Vector<std::string_view, 8> out;
    while (str.length() > delimiter.length()) {
        auto pos = str.find(delimiter);
        if (pos == std::string_view::npos) {
            break;
        }
        out.Push(str.substr(0, pos));
        str = str.substr(pos + delimiter.length());
    }
    out.Push(str);
    return out;
}

/// @param str the string to quote
/// @returns @p str quoted with <code>'</code>
inline std::string Quote(std::string_view str) {
    return "'" + std::string(str) + "'";
}

/// @param parts the input parts
/// @param delimiter the delimiter
/// @return @p parts joined as a string, delimited with @p delimiter
template <typename T>
inline std::string Join(VectorRef<T> parts, std::string_view delimiter) {
    StringStream s;
    for (auto& part : parts) {
        if (part != parts.Front()) {
            s << delimiter;
        }
        s << part;
    }
    return s.str();
}

/// @param parts the input parts
/// @param delimiter the delimiter
/// @return @p parts joined as a string, delimited with @p delimiter
template <typename T, size_t N>
inline std::string Join(const Vector<T, N>& parts, std::string_view delimiter) {
    return Join(VectorRef<T>(parts), delimiter);
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_STRING_H_
