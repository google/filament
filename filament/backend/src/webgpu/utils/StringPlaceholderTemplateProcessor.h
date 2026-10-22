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

#ifndef TNT_FILAMENT_BACKEND_UTIL_STRINGPLACEHOLDERTEMPLATEPROCESSOR_H
#define TNT_FILAMENT_BACKEND_UTIL_STRINGPLACEHOLDERTEMPLATEPROCESSOR_H

#include <string>
#include <string_view>
#include <unordered_map>

namespace filament::backend::webgpuutils {

/**
 * Given the string template, finds all placeholders (beginning with placeholderPrefix and ending
 * placeholderSuffix, where the placeholder name is between the prefix and suffix, e.g.
 * {{MY_PLACEHOLDER}} where the prefix is "{{", the suffix "}}", and the placeholder name
 * "MY_PLACEHOLDER"), and replaces the placeholders with values as provided in a given map.
 * @param stringTemplate The whole template to generate an output string
 * @param placeholderPrefix Indicates the beginning of a placeholder. This string should not be
 * present in the template for any other purpose. A typical choice might be "{{". If you
 * choose "{{" any occurrence of "{{" should not exist in the template other than to indicate the
 * beginning of a placeholder, otherwise you will have problems.
 * @param placeholderSuffix Indicates the ending of a placeholder. This string should not be
 * present in the template for any other purpose. A typical choice might be "}}". If you
 * choose "}}" any occurrence of "}}" should not exist in the template other than to indicate the
 * ending of a placeholder, otherwise you will have problems.
 * @param valueByPlaceholderName A map of placeholder names present in the template (between
 * placeholderPrefix and placeholderSuffix strings) to the string value to replace the whole
 * placeholder (include prefix and suffix). If a placeholder name is found in the template but not
 * in this map, the function will panic.
 * @return the stringTemplate, where all placeholders are replaced based on the
 * given valueByPlaceholderName map.
 */
[[nodiscard]] std::string processPlaceholderTemplate(std::string_view const& stringTemplate,
        std::string_view const& placeholderPrefix, std::string_view const& placeholderSuffix,
        std::unordered_map<std::string_view, std::string_view> const&
                valueByPlaceholderName);

} // namespace filament::backend::webgpuutils

#endif // TNT_FILAMENT_BACKEND_UTIL_STRINGPLACEHOLDERTEMPLATEPROCESSOR_H
