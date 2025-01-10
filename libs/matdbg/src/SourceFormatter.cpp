/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "SourceFormatter.h"

#include <utils/Log.h>

#include <cstdio>
#include <sstream>

namespace filament::matdbg {

#if defined(__linux__) || defined(__APPLE__)
std::string SourceFormatter::format(char const* source) {
    std::string const TMP_FILENAME = "/tmp/matdbg-tmp-src.cpp";

    std::string original(source);
    FILE* fp;
    fp = fopen(TMP_FILENAME.c_str(), "w");
    if (!fp) {
        return original;
    }
    fputs(original.c_str(), fp);
    fflush(fp);
    pclose(fp);

    std::string const CLANG_FORMAT_OPTIONS =
            "-style='{"
            "BasedOnStyle: Google, "
            "IndentWidth: 4, "
            "MaxEmptyLinesToKeep: 2"
            "}'";

    fp = popen(("clang-format " + CLANG_FORMAT_OPTIONS + "< " + TMP_FILENAME).c_str(), "r");

    if (!fp) {
        std::call_once(mClangWarningFlag, []() {
            utils::slog.w << "[matdbg] unable to run clang-format to format shader file. "
                          << "Please make sure it's installed.";
        });
        return original;
    }

    char output[1024];
    std::stringstream outStream;
    while (fgets(output, 1024, fp) != NULL) {
        outStream << output;
    }

    int status = pclose(fp);
    if (WEXITSTATUS(status)) {
        utils::slog.w << "[matdbg] clang-format failed with code=" << WEXITSTATUS(status)
                      << utils::io::endl;
    }
    return outStream.str();
}
#else
std::string SourceFormatter::format(char const* source) {
    std::call_once(mClangWarningFlag, []() {    
        utils::slog.w <<"[matdbg]: source formatting is not available on this platform" <<
                utils::io::endl;
    });
    return "";
}
#endif

} // filament::matdbg
