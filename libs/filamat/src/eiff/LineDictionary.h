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

#ifndef TNT_FILAMAT_LINEDICTIONARY_H
#define TNT_FILAMAT_LINEDICTIONARY_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace filamat {

// Establish a line <-> id mapping. Use for shader compression when each shader is sliced in lines
// and each line encoded into a 16 bit id.
class LineDictionary {
public:
    LineDictionary() = default;

    // Due to the presence of unique_ptr, disallow copy construction but allow move construction.
    LineDictionary(LineDictionary const&) = delete;
    LineDictionary(LineDictionary&&) = default;

    void addText(const std::string& text) noexcept;
    size_t getLineCount() const;

    bool isEmpty() const noexcept {
        return mStrings.empty();
    }

    std::string_view getString(size_t index) const noexcept;
    size_t getIndex(std::string_view s) const noexcept;

private:
    void addLine(const std::string&& line) noexcept;

    std::unordered_map<std::string_view, size_t> mLineIndices;
    std::vector<std::unique_ptr<std::string>> mStrings;
};

} // namespace filamat
#endif // TNT_FILAMAT_LINEDICTIONARY_H
