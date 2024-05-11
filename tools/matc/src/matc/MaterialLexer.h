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

#ifndef TNT_MATERIALLEXER_H
#define TNT_MATERIALLEXER_H

#include "Lexer.h"
#include "MaterialLexeme.h"

namespace matc {

class MaterialLexer final: public Lexer<MaterialLexeme> {
public:
private:

    virtual bool readLexeme() noexcept override;

    bool peek(MaterialType* type) const noexcept;

    bool readBlock() noexcept;
    void readIdentifier() noexcept;
    void readUnknown() noexcept;
};

}

#endif //TNT_MATERIALLEXER_H
