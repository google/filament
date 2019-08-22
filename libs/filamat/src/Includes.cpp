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

#include "Includes.h"

#include <utils/Log.h>

#include <string>

namespace filamat {

static bool isWhitespace(char c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

bool resolveIncludes(const utils::CString& rootName, utils::CString& source,
        IncludeCallback callback, size_t depth) {
    if (depth > 30) {
        // This is probably an include cycle. Stop here and report an error so we don't overflow.
        utils::slog.e << "Include depth > 30. Include cycle?" << utils::io::endl;
        return false;
    }

    std::vector<FoundInclude> includes = parseForIncludes(source);
    while (!includes.empty()) {
        const auto include = includes[0];
        // Ask the includer to resolve this include.
        if (!callback) {
            return false;
        }
        IncludeResult result;
        if (!callback(include.name, rootName, result)) {
            utils::slog.e << "The included file \"" << include.name.c_str()
                          << "\" could not be found." << utils::io::endl;
            return false;
        }

        // Recursively resolve all of its includes.
        if (!resolveIncludes(result.name, result.source, callback, depth + 1)) {
            return false;
        }

        source.replace(include.startPosition, include.length, result.source);

        includes = parseForIncludes(source);
    }

    return true;
}

std::vector<FoundInclude> parseForIncludes(const utils::CString& source) {
    std::string sourceString = source.c_str();
    std::vector<FoundInclude> results;

    size_t result = sourceString.find("#include");

    while(result != std::string::npos) {
        const size_t includeStart = result;

        // Move to the character immediately after #include
        result += 8;

        // Eat up any whitespace after "#include"
        while (result < sourceString.length() && isWhitespace(sourceString[result])) {
            result++;
        }

        // The next character must be a "
        if (result >= sourceString.length() || sourceString[result] != '"') {
            result = sourceString.find("#include", result);
            continue;
        }
        result++;

        const size_t nameStart = result;

        // Increment until we reach the next "
        while (sourceString[result] != '"') {
            result++;
        }

        const size_t nameEnd = result - 1;

        const size_t includeEnd = result;

        // Move on to the next character
        result++;

        // Grab the include name.
        const auto includeName = sourceString.substr(nameStart, nameEnd - nameStart + 1);

        results.push_back({utils::CString(includeName.c_str()), includeStart, includeEnd - includeStart + 1});

        // Find next occurrence.
        result = sourceString.find("#include", result);
    }

    return results;
}

} // namespace filamat
