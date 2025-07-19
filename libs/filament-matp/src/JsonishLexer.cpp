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

#include <filament-matp/JsonishLexer.h>

#include <string.h>

namespace matp {

JsonType JsonishLexer::readIdentifier() noexcept {
    const char* lexemeStart = mCursor;

    while (hasMore() && isIdentifierCharacter(*mCursor)) {
        consume();
    }

    size_t lexemeSize = mCursor - lexemeStart;

    // Check what kind of keyword we got here.
    if (strncmp("true", lexemeStart, lexemeSize) == 0) {
        return BOOLEAN;
    } else if (strncmp("false", lexemeStart, lexemeSize) == 0) {
        return BOOLEAN;
    } else if (strncmp("null", lexemeStart, lexemeSize) == 0) {
        return NUll;
    } else {
        // Relax constraint where string MUST be surrounded by " characters.
        // All identifiers are treated as STRINGs.
        return STRING;
    }
}

void JsonishLexer::readString() noexcept {
    consume(); // Skip first double quote character.
    while (hasMore() && *mCursor != '"') {
        consume();
    }
    if (hasMore()) {
        consume(); // Skip the last double quote character.
    }
}

JsonType JsonishLexer::readPunctuation() noexcept {
    char punctuation = consume();

    // Add lexeme.
    switch (punctuation) {
        case ',': return COMMA;
        case ':': return COLUMN;
        case '[': return ARRAY_START;
        case ']': return ARRAY_END;
        case '{': return BLOCK_START;
        case '}': return BLOCK_END;
        default : return readIdentifier(); // Treat everything else as an identifier.
    }
}

void JsonishLexer::readNumber() noexcept {
    if (hasMore() && (isNumericCharacter(*mCursor) || *mCursor == '-')) {
        consume();
    }

    while(hasMore() && isNumericCharacter(*mCursor)) {
        consume();
    }

    if (!hasMore()) {
        return;
    }

    // Read fraction if present (TODO: Handle exp).
    if (*mCursor != '.') {
        return;
    }

    // There is a fractional part.
    consume();
    while (hasMore() && isNumericCharacter(*mCursor)) {
        consume();
    }
}

bool JsonishLexer::readLexeme() noexcept {
    skipWhiteSpace();

    if (!hasMore()) {
        return true;
    }

    const char* lexemeStart = mCursor;
    size_t line = getLine();
    size_t cursor = getCursor();

    JsonType lexemeType;
    switch (char c = peek()) {
        case '"' :
            readString();
            lexemeType = STRING;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            readNumber();
            lexemeType = NUMBER;
            break;
        case ',':
        case ':':
        case '[':
        case ']':
        case '{':
        case '}':
            lexemeType = readPunctuation();
            break;
        default:
            if (isIdentifierCharacter(c)) {
                lexemeType = readIdentifier();
            } else {
                return false;
            }
            break;
    }

    JsonLexeme lexeme(lexemeType, lexemeStart, mCursor - 1, line, cursor);
    lexeme = lexeme.trim();
    mLexemes.push_back(lexeme);

    return true;
}

} // namespace matp
