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


#ifndef TNT_MATERIALPARSER_H
#define TNT_MATERIALPARSER_H

#include <string>
#include <unordered_map>

#include "Config.h"

namespace filamat {
class MaterialBuilder;
}
class TestMaterialParser;

namespace matp {

class JsonishValue;
class MaterialLexeme;

class MaterialParser {
public:
    MaterialParser();

    // Parses a string material so that it can be used in MaterialBuilder.
    // Call MaterialBuilder::init before passing in the builder; call MaterialBuilder::build to
    // create filamat::Package after.
    bool parse(
            filamat::MaterialBuilder& builder,
            const Config& config,
            Config::Input* input, ssize_t& size, std::unique_ptr<const char[]>& buffer);
    // Replaces macro keywords with user specified ones. Must be called before parse.
    bool processTemplateSubstitutions(
            const Config& config, ssize_t& size, std::unique_ptr<const char[]>& buffer);
private:
    friend class ::TestMaterialParser;

    bool parseMaterial(const char* buffer, size_t size,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processMaterial(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processVertexShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processFragmentShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processComputeShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool ignoreLexeme(const MaterialLexeme&, filamat::MaterialBuilder& builder) const noexcept;

    bool parseMaterialAsJSON(const char* buffer, size_t size,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processMaterialJSON(const JsonishValue*,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processVertexShaderJSON(const JsonishValue*,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processFragmentShaderJSON(const JsonishValue*,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processComputeShaderJSON(const JsonishValue*,
            filamat::MaterialBuilder& builder) const noexcept;
    bool ignoreLexemeJSON(const JsonishValue*, filamat::MaterialBuilder& builder) const noexcept;
    bool isValidJsonStart(const char* buffer, size_t size) const noexcept;

    bool processMaterialParameters(filamat::MaterialBuilder& builder, const Config& config) const;

    // Member function pointer type, this is used to implement a Command design
    // pattern.
    using MaterialConfigProcessor = bool (MaterialParser::*)
            (const MaterialLexeme&, filamat::MaterialBuilder& builder) const;
    // Map used to store Command pattern function pointers.
    std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;

    // The same, but for pure JSON syntax
    using MaterialConfigProcessorJSON = bool (MaterialParser::*)
            (const JsonishValue*, filamat::MaterialBuilder& builder) const;
    std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;
};

} // namespace matp


#endif // TNT_MATERIALPARSER_H
