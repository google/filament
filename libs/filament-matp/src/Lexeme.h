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

#ifndef TNT_LEXEME_H
#define TNT_LEXEME_H

#include <cstddef>
#include <string>
#include <vector>

namespace matp {

// A lexeme MUST be templated on the family, e.g: Material config, json or glsl.
template <class T>
class Lexeme {
public:

    Lexeme(T type, const char* start, const char* end, size_t line, size_t pos) :
            mStart(start), mEnd(end), mLineNumber(line), mPosition(pos), mType(type) {
    }

    const char* getStart() const noexcept {
        return mStart;
    }
    const char* getEnd() const noexcept {
        return mEnd;
    }

    const size_t getSize() const noexcept {
        return mEnd - mStart + 1;
    }

    const T getType() const noexcept {
        return mType;
    }
    const size_t getLine() const noexcept {
        return mLineNumber;
    }
    const size_t getLinePosition() const noexcept {
        return mPosition;
    }

    const std::string getStringValue() const noexcept {
        return std::string(mStart, mEnd - mStart + 1);
    }

protected:
    const char* mStart; // included
    const char* mEnd;   // included
    size_t mLineNumber;
    size_t mPosition; // The offset in the line.
    T mType;
};

} // namespace matp

#endif // TNT_LEXEME_H
