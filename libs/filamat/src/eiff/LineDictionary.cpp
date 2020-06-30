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

#include <assert.h>

namespace filamat {

    LineDictionary::LineDictionary() : mStorageSize(0){
}

const std::string& LineDictionary::getString(size_t index) const noexcept {
    assert(index < mStrings.size());
    return mStrings[index];
}

size_t LineDictionary::getLineCount() const {
    return mStrings.size();
}

size_t LineDictionary::getIndex(const std::string& s) const noexcept {
    if (mLineIndices.find(s) == mLineIndices.end()) {
        return SIZE_MAX;
    }
    return mLineIndices.at(s);
}

void LineDictionary::addText(const std::string& line) noexcept {
    const char* s = line.c_str();

    assert(s != nullptr);

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

    mLineIndices[line] = mStrings.size();
    size_t size = line.size();
    mStrings.push_back(std::move(line));
    mStorageSize += size + 1;
}

} // namespace filamat
