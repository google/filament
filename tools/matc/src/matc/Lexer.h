/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_LEXER_H
#define TNT_LEXER_H

#include <cstddef>

#include "Lexeme.h"

namespace matc {

// Templated on the class of Lexemes to parse: Material lexeme, JSON Lexemes, GLSL lexemes.
template <class T>
class Lexer {
public:
    virtual ~Lexer() = default;

    void lex(const char* text, size_t size, size_t lineOffset = 1) {
        mCursor = text;
        mEnd = mCursor + size;
        mLineCounter = lineOffset;
        while (hasMore()) {
            if (!readLexeme()) {
                return;
            }
        }
    }

    const std::vector<T>& getLexemes() {
        return mLexemes;
    }

protected:

    size_t getLine() const noexcept {
        return mLineCounter;
    }

    size_t getCursor() const noexcept {
        return mLinePosition;
    }

    inline bool isWhiteSpaceCharacter(char c) const noexcept {
        return c < 33 || c > 127;
    }

    inline bool isAlphaCharacter(char c) const noexcept {
        return (c >= 'A' && c <= 'Z') ||  (c >= 'a' && c <= 'z');
    }

    inline bool isNumericCharacter(char c) const noexcept {
        return (c >= '0' && c <= '9');
    }

    inline bool isAphaNumericCharacter(char c) const noexcept {
        return  isAlphaCharacter(c) || isNumericCharacter(c);
    }

    inline bool isIdentifierCharacter(char c) const noexcept {
        return isAphaNumericCharacter(c) || (c == '_');
    }

    inline bool hasMore() const noexcept {
        return mCursor < mEnd;
    }

    inline void skipUntilEndOfLine() {
        while (hasMore() && *mCursor != '\n') {
            consume();
        }
    }

    inline char consume() noexcept {
        if (*mCursor == '\n') {
            mLineCounter++;
            mLinePosition = 0;
        }
        mLinePosition++;
        return *mCursor++;
    }

    inline char peek() const noexcept {
        return *mCursor;
    }

    // Read the next Lexeme. It will be of the type peek() returned just before
    // this method was called. This method can return false if it wishes lexing to stop.
    virtual bool readLexeme() noexcept = 0;

    void skipWhiteSpace() {
        while (mCursor < mEnd && isWhiteSpaceCharacter(*mCursor)) {
            consume();
        }

        // If a comment marker is detected "//", skip until the next return
        // carriage is encountered.
        if (hasMore() && *mCursor == '/' && mCursor < (mEnd - 1) && *(mCursor + 1) == '/') {
            skipUntilEndOfLine();
            skipWhiteSpace();
        }
    }

    const char* mEnd = nullptr;
    const char* mCursor = nullptr;
    std::vector<T> mLexemes;

    size_t mLineCounter = 1;
    size_t mLinePosition = 0;
};
}

#endif //TNT_LEXER_H
