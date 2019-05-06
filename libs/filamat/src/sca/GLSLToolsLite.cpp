/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "GLSLToolsLite.h"

#include <filamat/Enums.h>

using namespace filament::backend;

namespace filamat {

static bool isVariableCharacter(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

bool GLSLToolsLite::findProperties(const utils::CString& material,
                    MaterialBuilder::PropertyList& properties) const noexcept {
    const std::string shaderCode(material.c_str());

    size_t start = 0, end = 0;

    const auto p = Enums::map<Property>();

    // Find all occurrences of "material.someProperty" in the shader string.
    size_t loc;
    while ((loc = shaderCode.find("material.", start)) != std::string::npos) {
        // Set start and end to the index of the first character after "material."
        start = loc + 9;
        end = start;

        // Increment end until we reach a non-variable character.
        while (end != shaderCode.length() && isVariableCharacter(shaderCode[end])) {
            end++;
        }

        std::string foundProperty = shaderCode.substr(start, end - start);

        // Check to see if this property matches any in the property list.
        for (const auto& i : p) {
            const std::string& propertySymbol = i.first;
            Property prop = i.second;

            if (foundProperty == propertySymbol) {
                properties[size_t(prop)] = true;
            }
        }
    }

    return true;
}

} // namespace filamat
