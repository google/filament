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

#include "GlslMinify.h"

#include <regex>

namespace glslminifier {

using std::regex;

// Slash slash comments:
// [\t ]*           check for tabs and spaces up until
// \/\/             two consecutive forward slashes
// .*               match the rest of the line
const static regex slashSlashCommentsRegex(R"regex([\t ]*\/\/.*)regex");

// Star comments:
// [\t ]*           check for tabs and spaces up until
// \/\*             the characters '/' + '*'
// [\w\W]*?         match any character, including newlines. The ? makes it "non-greedy"
// \*\/             stop the match at the first occurrence of '*' + '/'
const static regex starCommentsRegex(R"regex([\t ]*\/\*[\w\W]*?\*\/)regex");

// Empty lines:
// (\n|\r\n)+       match 1 or more newlines (either linux or Windows-style)
//                  The capture group will only capture a single newline sequence.
const static regex emptyLinesRegex(R"regex((\n|\r\n)+)regex");

// Indentation:
// (^|\n)           match the beginning of the file, or a newline character (C++14 does not support
//                  the multiline option, unfortunately)
// [ \t]+           match at least one leading space or tab character
const static regex indentationRegex(R"regex((^|\n)[ \t]+)regex");

std::string minifyGlsl(const std::string& glsl, GlslMinifyOptions options) noexcept {
    std::string minified = glsl;

    if (options & GlslMinifyOptions::STRIP_COMMENTS)  {
        minified = regex_replace(minified, slashSlashCommentsRegex, "");
        minified = regex_replace(minified, starCommentsRegex, "");
    }

    if (options & GlslMinifyOptions::STRIP_EMPTY_LINES) {
        // Replace the group of newlines with a single newline, which is in the first capture group.
        minified = regex_replace(minified, emptyLinesRegex, "$1");
    }

    if (options & GlslMinifyOptions::STRIP_INDENTATION) {
        // Replace the indentation with whatever was captured in the first capture group, which will
        // either be a newline or nothing at all (in the case of the first line).
        // We do this to preserve newlines when stripping indentation.
        minified = regex_replace(minified, indentationRegex, "$1");
    }

    return minified;
}

} // namespace glslminifier
