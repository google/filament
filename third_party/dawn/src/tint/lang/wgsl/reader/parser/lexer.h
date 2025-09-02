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

#ifndef SRC_TINT_LANG_WGSL_READER_PARSER_LEXER_H_
#define SRC_TINT_LANG_WGSL_READER_PARSER_LEXER_H_

#include <optional>
#include <vector>

#include "src/tint/lang/wgsl/reader/parser/token.h"

namespace tint::wgsl::reader {

/// Converts the input stream into a series of Tokens
class Lexer {
  public:
    /// Creates a new Lexer
    /// @param file the source file
    explicit Lexer(const Source::File* file);
    ~Lexer();

    /// @return the token list.
    std::vector<Token> Lex();

  private:
    /// Returns the next token in the input stream.
    /// @return Token
    Token next();

    /// Advances past blankspace and comments, if present at the current position.
    /// @returns error token, EOF, or uninitialized
    std::optional<Token> skip_blankspace_and_comments();
    /// Advances past a comment at the current position, if one exists.
    /// Returns an error if there was an unterminated block comment,
    /// or a null character was present.
    /// @returns uninitialized token on success, or error
    std::optional<Token> skip_comment();

    Token build_token_from_int_if_possible(Source source,
                                           uint32_t start,
                                           uint32_t prefix_count,
                                           int32_t base);

    std::optional<Token::Type> parse_keyword(std::string_view);

    /// The try_* methods have the following in common:
    /// - They assume there is at least one character to be consumed,
    ///   i.e. the input has not yet reached end of file.
    /// - They return an initialized token when they match and consume
    ///   a token of the specified kind.
    /// - Some can return an error token.
    /// - Otherwise they return an uninitialized token when they did not
    ///   match a token of the specfied kind.
    std::optional<Token> try_float();
    std::optional<Token> try_hex_float();
    std::optional<Token> try_hex_integer();
    std::optional<Token> try_ident();
    std::optional<Token> try_integer();
    std::optional<Token> try_punctuation();

    Source begin_source() const;
    void end_source(Source&) const;

    /// @returns view of current line
    std::string_view line() const;
    /// @returns position in current line
    uint32_t pos() const;
    /// @returns length of current line
    uint32_t length() const;
    /// @returns reference to character at `pos` within current line
    const char& at(uint32_t pos) const;
    /// @returns a point to the character just beyond the end of the current line, similar to how
    /// std::end works
    const char* line_end() const;
    /// @returns substring view at `offset` within current line of length `count`
    std::string_view substr(uint32_t offset, uint32_t count);
    /// advances current position by `offset` within current line
    void advance(uint32_t offset = 1);
    /// sets current position to `pos` within current line
    void set_pos(uint32_t pos);
    /// advances current position to next line
    void advance_line();
    /// @returns true if the current position contains a BOM
    bool is_bom() const;
    /// @returns true if the end of the input has been reached.
    bool is_eof() const;
    /// @returns true if the end of the current line has been reached.
    bool is_eol() const;
    /// @returns true if there is another character on the input and
    /// it is not null.
    bool is_null() const;
    /// @param ch a character
    /// @returns true if 'ch' is a decimal digit
    bool is_digit(char ch) const;
    /// @param ch a character
    /// @returns true if 'ch' is a hexadecimal digit
    bool is_hex(char ch) const;
    /// @returns true if string at `pos` matches `substr`
    bool matches(uint32_t pos, std::string_view substr);
    /// @returns true if char at `pos` matches `ch`
    bool matches(uint32_t pos, char ch);
    /// The source file content
    Source::File const* const file_;
    /// The current location within the input
    Source::Location location_;
};

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_PARSER_LEXER_H_
