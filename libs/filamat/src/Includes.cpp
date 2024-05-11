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
#include <utils/compiler.h>
#include <utils/sstream.h>

#include <string>

namespace filamat {

static bool isWhitespace(char c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

struct CommentRange {
    size_t begin;
    size_t end;

    CommentRange(size_t begin, size_t end) :
        begin(begin), end(end) {}
};

template<size_t BTSize, size_t ETSize>
static void findCommentRanges(const char (&beginToken)[BTSize], const char (&endToken)[ETSize],
        std::string& source, std::vector<CommentRange>& comments) {
    // BTSize and ETSize include the null terminator.
    constexpr size_t beginTokenSize = BTSize - 1;
    constexpr size_t endTokenSize = ETSize - 1;

    // Find the first occurance of the begin token.
    size_t r = source.find(beginToken);
    while (r != std::string::npos) {
        // This is the start of a new comment.
        const size_t commentStart = r;

        // Advance the pointer past the token.
        r += beginTokenSize;

        // Find the terminating token for this comment.
        r = source.find(endToken, r);
        if (r != std::string::npos) {
            // Advance the pointer past the token.
            r += endTokenSize;

            // The end of the comment is the last character of the end token.
            size_t commentEnd = r - 1;

            comments.emplace_back(commentStart, commentEnd);

            // Find the next comment.
            r = source.find(beginToken, r);
        } else {
            // This comment was never terminated- consider the rest of the source to a comment.
            // This also solves the case of a single-line source with a // comment but no
            // terminating newline.
            comments.emplace_back(commentStart, source.size() - 1);
        }
    }
}


bool resolveIncludes(IncludeResult& root, IncludeCallback callback,
        const ResolveOptions& options, size_t depth) {
    if (depth > 30) {
        // This is probably an include cycle. Stop here and report an error so we don't overflow.
        utils::slog.e << "Include depth > 30. Include cycle?" << utils::io::endl;
        return false;
    }

    const size_t lineNumberOffset = root.lineNumberOffset;
    utils::CString& text = root.text;

    std::vector<FoundInclude> includes = parseForIncludes(text);
    bool sourceDirty = false;

    // If we weren't given an include name, use "0", which is default when no #line directives are
    // used.
    const char* rootIncludeName = root.includeName.empty() ? "0" : root.includeName.c_str();

    auto insertLineDirective = [&options](utils::io::ostream& stream, size_t line, const char* filename) {
        if (options.insertLineDirectiveCheck) {
            stream << "#if defined(GL_GOOGLE_cpp_style_line_directive)\n";
            // The #endif itself will count as a line, so subtact 1.
            line--;
        }
        stream << "#line " << line << " \"" << filename << '\"';
        if (options.insertLineDirectiveCheck) {
            stream << "\n#endif";
        }
    };

    // The #line directive must be on its own line and works like so:
    // #line 10 "file.h"
    // any code on this line is now considered line 10 of file.h

    // Add #line directives before / after each #include. We work backwards, otherwise we'd
    // invalidate the offsets in FoundInclude.
    for (auto it = includes.rbegin(); it < includes.rend() && options.insertLineDirectives; ++it) {
        const auto include = *it;

        // Remember that text editors consider the first line of a file to be line 1.
        // Consider the following file, called "root.h":
        // 1
        // 2 #include "foo.h"
        // 3

        // We want to insert an opening and closing #line directive:
        // 1
        //   #line 1 "foo.h"
        // 2 #include "foo.h"
        //   #line 3 "root.h"
        // 3

        // We want to insert a closing directive with a line number of 3.
        // In this example, include.line is 2 and lineNumberOffset is 0.
        // So, the math works out as such:
        const size_t lineDirectiveLine = include.line + lineNumberOffset + 1;

        utils::io::sstream closingDirective;

        // This first newline is to ensure that the #line directive falls on a fresh line.
        closingDirective << '\n';

        // If there's a newline after the include, we'll use that to terminate the #line directive.
        const size_t newlineCharacter = include.startPosition + include.length;
        if (text.length() > newlineCharacter && text[newlineCharacter] == '\n') {
            insertLineDirective(closingDirective, lineDirectiveLine, rootIncludeName);
        } else {
            // If there isn't one, be sure to add one. The included source might not have an
            // newline at the end of the file.

            // We subtract 1 to handle additional code after the include directive.
            // E.g., this include statement:
            // #include "foobar.h"  more code on same line
            //
            // should get translated to:
            // #line 1
            // #include "foobar.h"
            // #line 1
            //  more code on same line

            insertLineDirective(closingDirective, lineDirectiveLine - 1, rootIncludeName);
            closingDirective << '\n';
        }

        text.insert(include.startPosition + include.length, utils::CString(closingDirective.c_str()));

        // The included source always starts on line 1.
        utils::io::sstream openingDirective;
        insertLineDirective(openingDirective, 1, include.name.c_str());
        openingDirective << '\n';
        text.insert(include.startPosition, utils::CString(openingDirective.c_str()));
        sourceDirty = true;
    }

    // Add a line directive on the first line for the root include.
    if (options.insertLineDirectives && depth == 0) {
        utils::io::sstream lineDirective;
        insertLineDirective(lineDirective, lineNumberOffset + 1, rootIncludeName);
        lineDirective << '\n';
        text.insert(0, utils::CString(lineDirective.c_str()));
        sourceDirty = true;
    }

    // Re-parse for includes. If we've inserted any #line directives, then the line numbers have
    // changed.
    if (UTILS_LIKELY(sourceDirty)) {
        includes = parseForIncludes(text);
    }

    while (!includes.empty() && options.resolveIncludes) {
        const auto include = includes[0];
        // Ask the includer to resolve this include.
        if (!callback) {
            return false;
        }
        IncludeResult resolved {
            .includeName = include.name
        };
        if (!callback(root.name, resolved)) {
            utils::slog.e << "The included file \"" << include.name.c_str()
                          << "\" could not be found." << utils::io::endl;
            return false;
        }

        // Recursively resolve all of its includes.
        if (!resolveIncludes(resolved, callback, options, depth + 1)) {
            return false;
        }

        text.replace(include.startPosition, include.length, resolved.text);

        includes = parseForIncludes(text);
    }

    return true;
}

std::vector<FoundInclude> parseForIncludes(const utils::CString& source) {
    std::vector<FoundInclude> results;

    if (source.empty()) {
        return results;
    }
    std::string sourceString = source.c_str();

    std::vector<CommentRange> commentRanges;
    findCommentRanges("/*", "*/", sourceString, commentRanges);
    findCommentRanges("//", "\n", sourceString, commentRanges);

    auto isInsideComment = [&commentRanges] (size_t pos) {
        for (auto r : commentRanges) {
            if (pos >= r.begin && pos <= r.end) {
                return true;
            }
        }
        return false;
    };

    size_t result = sourceString.find("#include");

    while(result != std::string::npos) {
        const size_t includeStart = result;

        // Move to the character immediately after #include
        result += 8;

        // If this include is within a comment, ignore it.
        if (isInsideComment(includeStart)) {
            continue;
        }

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
        while (result < sourceString.length() && sourceString[result] != '"') {
            result++;
        }

        // check we're not at the end of the line -- this would be a malformed include directive.
        if (result >= sourceString.length()) {
            result = sourceString.find("#include", result);
            continue;
        }


        const size_t nameEnd = result - 1;

        const size_t includeEnd = result;

        // Move on to the next character
        result++;

        // Grab the include name.
        const auto includeName = sourceString.substr(nameStart, nameEnd - nameStart + 1);

        // Calculate the line number of the include.
        size_t lineNumber = 1;
        for (size_t i = 0; i < includeStart; i++) {
            if (source[i] == '\n') {
                lineNumber++;
            }
        }

        results.push_back({utils::CString(includeName.c_str()), includeStart,
                includeEnd - includeStart + 1, lineNumber});

        // Find next occurrence.
        result = sourceString.find("#include", result);
    }

    return results;
}

} // namespace filamat
