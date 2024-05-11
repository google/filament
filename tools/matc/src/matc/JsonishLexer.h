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

#ifndef TNT_JSONLEXER_H
#define TNT_JSONLEXER_H

#include "Lexer.h"
#include "JsonishLexeme.h"

namespace matc {

// Breaks down a stream of chars into JsonLexemes.
class JsonishLexer final: public Lexer<JsonLexeme> {
public:
    virtual bool readLexeme() noexcept;
private:
    JsonType readIdentifier() noexcept;
    void readNumber() noexcept;
    void readString() noexcept;
    JsonType readPunctuation() noexcept;
};

} // namespace matc

#endif //TNT_JSONLEXER_H
