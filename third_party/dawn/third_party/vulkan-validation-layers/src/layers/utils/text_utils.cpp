/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "text_utils.h"
#include <algorithm>
#include <cassert>
#include <vector>

namespace text {

std::string VFormat(const char *format, va_list argptr) {
    // Counts actual characters. Null terminator is accounted separately.
    const int initial_max_symbol_count = 1024;

    // Use vector as the output buffer for c-style formatting routines.
    // Do not write directly to std::string internal storage because its
    // structure and null terminator convention is not standardized.
    std::vector<char> buffer(initial_max_symbol_count + 1 /*null terminator*/);

    // The va_list will be modified by the call to vsnprintf. Use a copy in case we need to try again.
    va_list argptr2;
    va_copy(argptr2, argptr);
    const int symbol_count = vsnprintf(buffer.data(), buffer.size(), format, argptr2);
    va_end(argptr2);

    if (symbol_count < 0) {
        assert(false && "unexpected vsnprintf error");
        return {};
    }
    if (symbol_count > initial_max_symbol_count) {
        buffer.resize(symbol_count + 1 /*null terminator*/);
        vsnprintf(buffer.data(), buffer.size(), format, argptr);
    }
    std::string str(buffer.data());
    return str;
}

std::string Format(const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    std::string str = VFormat(format, argptr);
    va_end(argptr);
    return str;
}

void ToLower(std::string &str) {
    // std::tolower() returns int which can cause compiler warnings
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });
}

void ToUpper(std::string &str) {
    // std::toupper() returns int which can cause compiler warnings
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return static_cast<char>(std::toupper(c)); });
}

}  // namespace text
