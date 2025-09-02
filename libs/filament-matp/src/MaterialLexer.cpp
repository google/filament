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

#include "MaterialLexer.h"

namespace matp {

bool MaterialLexer::readBlock() noexcept {
    size_t braceCount = 0;
     while (hasMore()) {
         skipWhiteSpace();

        // This can occur if the block is malformed.
        if (mCursor >= mEnd) {
            return false;
        }

         if (*mCursor == '{') {
             braceCount++;
         }  else if (*mCursor == '}') {
             braceCount--;
         }

         if (braceCount == 0) {
             consume();
             return true;
         }

         consume();
     }
     return true;
}

void MaterialLexer::readIdentifier() noexcept {
    while (hasMore() && isAphaNumericCharacter(*mCursor)) {
        consume();
    }
}

void MaterialLexer::readUnknown() noexcept {
    consume();
}

bool MaterialLexer::peek(MaterialType* type) const noexcept {
    if (!hasMore()) return false;

    char c = *mCursor;
    if (isAlphaCharacter(c)) {
        *type = MaterialType::IDENTIFIER;
    } else if (c == '{') {
        *type = MaterialType::BLOCK;
    } else {
        *type = MaterialType::UNKNOWN;
    }

    return true;
}

bool MaterialLexer::readLexeme() noexcept {
    skipWhiteSpace();

    MaterialType nextMaterialType;
    bool peeked = peek(&nextMaterialType);
    if (!peeked) {
        return true;
    }

    const char* lexemeStart = mCursor;
    size_t line = getLine();
    size_t cursor = getCursor();
    switch (nextMaterialType) {
        case MaterialType::BLOCK:
            if (!readBlock()) {
                nextMaterialType = MaterialType::UNKNOWN;
            }
            break;
        case MaterialType::IDENTIFIER: readIdentifier(); break;
        case MaterialType::UNKNOWN:    readUnknown();    break;
        default:
            break;
    }
    mLexemes.emplace_back(nextMaterialType, lexemeStart, mCursor - 1, line, cursor);

    return nextMaterialType != UNKNOWN;
}

} // namespace matp
