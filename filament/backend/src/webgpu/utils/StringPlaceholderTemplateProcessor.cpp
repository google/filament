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

#include "StringPlaceholderTemplateProcessor.h"

#include <utils/Panic.h>
#include <utils/debug.h>

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace filament::backend::webgpuutils {

std::string processPlaceholderTemplate(std::string_view const& stringTemplate,
        std::string_view const& placeholderPrefix, std::string_view const& placeholderSuffix,
        std::unordered_map<std::string_view, std::string_view> const& valueByPlaceholderName) {
    const char* const sourceData{ stringTemplate.data() };
    std::stringstream out{};
    size_t positionCursorInTemplateString{ 0 };
    while (positionCursorInTemplateString < stringTemplate.size()) {
        const size_t positionOfNextPlaceholder{ stringTemplate.find(placeholderPrefix,
                positionCursorInTemplateString) };
        if (positionOfNextPlaceholder == std::string::npos) {
            // no more placeholders, so just stream the rest of the source string
            out << std::string_view(sourceData + positionCursorInTemplateString,
                    stringTemplate.size() - positionCursorInTemplateString);
            break;
        }
        const size_t positionOfPlaceholder{ positionOfNextPlaceholder + placeholderPrefix.size() };
        // stream up to the placeholder...
        out << std::string_view(sourceData + positionCursorInTemplateString,
                positionOfNextPlaceholder - positionCursorInTemplateString);
        // stream the value in place of the placeholder...
        const size_t positionAfterPlaceholder{ stringTemplate.find(placeholderSuffix,
                positionOfPlaceholder) };
        assert_invariant(positionAfterPlaceholder != std::string::npos &&
                         "Malformed source with missing suffix to placeholder");
        const std::string_view placeholderName{ std::string_view(sourceData + positionOfPlaceholder,
                positionAfterPlaceholder - positionOfPlaceholder) };
        if (const auto iter{ valueByPlaceholderName.find(placeholderName) };
                iter == valueByPlaceholderName.end()) {
            // wrapping placeholderName in a std::string to null terminate the C string....
            PANIC_POSTCONDITION("Found placeholder '%s' in template, but this is not present in "
                                "valueByPlaceholderName",
                    std::string{ placeholderName }.data());
        } else {
            const std::string_view value{ iter->second };
            out << value;
        }
        // update the cursor for after the placeholder...
        positionCursorInTemplateString = positionAfterPlaceholder + placeholderSuffix.size();
    }
    return out.str();
}

} // namespace filament::backend::webgpuutils
