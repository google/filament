/*
 * Copyright 2018 The Android Open Source Project
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

#include <string>

#ifndef TNT_GLSLMINIFY_H
#define TNT_GLSLMINIFY_H

namespace glslminifier {

enum GlslMinifyOptions : uint32_t {
    NONE                = 0x0,

    // Remove comments, both slash-slash (//) and slash-asterisk (/**/)
    STRIP_COMMENTS      = 0x1,

    // Removes empty lines (two or more consecutive newlines are turned into one).
    STRIP_EMPTY_LINES   = 0x2,

    // Removes indentation.
    STRIP_INDENTATION   = 0x4,

    ALL                 = 0xFFFFFFFF
};

std::string minifyGlsl(const std::string& glsl,
        GlslMinifyOptions options = GlslMinifyOptions::ALL) noexcept;

} // namespace glslminifier

#endif //TNT_GLSLMINIFY_H
