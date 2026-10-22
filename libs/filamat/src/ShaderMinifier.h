/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_SHADERMINIFIER_H
#define TNT_SHADERMINIFIER_H

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace filamat {

// Simple minifier for monolithic GLSL or MSL strings.
//
// Note that we already use a third party minifier, but it applies only to GLSL fragments.
// This custom minifier is designed for generated code such as uniform structs.
class ShaderMinifier {
    public:
        std::string removeWhitespace(const std::string& source, bool mergeBraces = false) const;
        std::string renameStructFields(const std::string& source);

    private:
        using RenameEntry = std::pair<std::string, std::string>;

        void buildFieldMapping();
        std::string applyFieldMapping() const;

        // These fields do not need to be members, but they allow clients to reduce malloc churn
        // by persisting the minifier object.
        std::vector<std::string_view> mCodelines;
        std::vector<RenameEntry> mStructFieldMap;
        std::unordered_map<std::string, std::string> mStructDefnMap;
};

} // namespace filamat

#endif //TNT_SHADERMINIFIER_H
