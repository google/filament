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

#include <stdlib.h>

using namespace filament::backend;

namespace filamat {

static bool isVariableCharacter(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

static bool isWhitespace(char c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

static std::string stripComments(const std::string& code) {
    char* temp = (char*) malloc(code.size() + 1);

    size_t r = 0;
    bool insideSlashSlashComment = false, insideMultilineComment = false;

    for (size_t loc = 0; loc < code.size(); loc++) {
        char c = code[loc];
        char lookahead = (loc + 1) < code.size() ? code[loc + 1] : 0;

        // Handle slash slash comments.
        if (!insideSlashSlashComment && c == '/' && lookahead == '/') {
            insideSlashSlashComment = true;
        }
        if (insideSlashSlashComment && c == '\n') {
            insideSlashSlashComment = false;
        }

        // Handle multiline comments.
        if (!insideMultilineComment && c == '/' && lookahead == '*') {
            insideMultilineComment = true;
        }
        if (insideMultilineComment && c == '*' && lookahead == '/') {
            insideMultilineComment = false;
        }

        if (insideSlashSlashComment || insideMultilineComment) {
            continue;
        }

        temp[r++] = c;
    }

    temp[r] = 0;
    std::string result(temp);
    free(temp);

    return result;
}

bool GLSLToolsLite::findProperties(
        filament::backend::ShaderType type,
        const utils::CString& material,
        MaterialBuilder::PropertyList& properties) const noexcept {
    if (material.empty()) {
        return true;
    }

    const std::string shaderCode = stripComments(material.c_str());

    size_t start = 0, end = 0;

    const auto p = Enums::map<Property>();

    // Find all occurrences of "material.someProperty" in the shader string.
    // TODO: We should find the name of the structure and search for <theName>.someProperty
    size_t loc;
    while ((loc = shaderCode.find("material", start)) != std::string::npos) {
        // Set start to the index of the first character after "material"
        start = loc + 8;

        // Eat up any whitespace after "material"
        while (start != shaderCode.length() && isWhitespace(shaderCode[start])) {
            start++;
        }

        // If the next character isn't a '.', then this isn't a property set.
        if (start == shaderCode.length() || shaderCode[start] != '.') {
            continue;
        }
        start++;

        // Eat up any whitespace after the '.'.
        while (start != shaderCode.length() && isWhitespace(shaderCode[start])) {
            start++;
        }

        // Increment end until we reach a non-variable character.
        end = start;
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
