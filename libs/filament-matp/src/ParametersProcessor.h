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

#ifndef TNT_PARAMETERSPROCESSOR_H
#define TNT_PARAMETERSPROCESSOR_H

#include <unordered_map>
#include <string>
#include <variant>

#include "JsonishLexeme.h"
#include "JsonishParser.h"

#include <filamat/MaterialBuilder.h>

namespace matp {

class ParametersProcessor {

public:
    ParametersProcessor();
    ~ParametersProcessor() = default;
    bool process(filamat::MaterialBuilder& builder, const JsonishObject& jsonObject);
    bool process(filamat::MaterialBuilder& builder, const std::string& key, const std::string& value);

private:

    using Callback = bool (*)(filamat::MaterialBuilder& builder, const JsonishValue& value);

    struct ParameterInfo {
        Callback callback;
        JsonishValue::Type rootAssert;
    };

    std::unordered_map<std::string, ParameterInfo> mParameters;
};

} // namespace matp

#endif //TNT_PARAMETERSPROCESSOR_H
