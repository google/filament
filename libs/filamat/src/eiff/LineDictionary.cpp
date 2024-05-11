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

#include "LineDictionary.h"

namespace filamat {

std::string_view LineDictionary::getString(size_t index) const noexcept {
    return *mStrings[index];
}

size_t LineDictionary::getLineCount() const {
    return mStrings.size();
}

size_t LineDictionary::getIndex(std::string_view s) const noexcept {
    if (auto iter = mLineIndices.find(s); iter != mLineIndices.end()) {
        return iter->second;
    }
    return SIZE_MAX;
}

void LineDictionary::addText(const std::string& line) noexcept {
    const char* s = line.c_str();

    size_t cur = 0;
    size_t pos = 0;
    size_t len = 0;

    while (s[cur] != '\0') {
        pos = cur;
        len = 0;
        while (s[cur] != '\n') {
            cur++;
            len++;
        }
        std::string newLine(s + pos, len);
        addLine(std::move(newLine));
        cur++;
    }
}

void LineDictionary::addLine(const std::string&& line) noexcept {
    // Never add a line twice.
    if (mLineIndices.find(line) != mLineIndices.end()) {
        return;
    }
    mStrings.emplace_back(std::make_unique<std::string>(line));
    mLineIndices.emplace(*mStrings.back(), mStrings.size() - 1);
}

} // namespace filamat
