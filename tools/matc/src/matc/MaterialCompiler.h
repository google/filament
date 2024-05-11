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

#ifndef TNT_MATERIALCOMPILER_H
#define TNT_MATERIALCOMPILER_H

#include <string>
#include <unordered_map>

#include "Compiler.h"
#include "MaterialLexeme.h"

namespace filamat {
class MaterialBuilder;
}
class TestMaterialCompiler;

namespace matc {

class JsonishValue;
class MaterialCompiler final: public Compiler {
public:
    MaterialCompiler();

    bool run(const Config& config) override;

    bool checkParameters(const Config& config) override;

private:
    friend class ::TestMaterialCompiler;

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

    bool compileRawShader(const char* glsl, size_t size, bool isDebug, Config::Output* output,
                const char* ext) const noexcept;

    bool processMaterialParameters(filamat::MaterialBuilder& builder, const Config& config) const;

    // Member function pointer type, this is used to implement a Command design
    // pattern.
    using MaterialConfigProcessor = bool (MaterialCompiler::*)
            (const MaterialLexeme&, filamat::MaterialBuilder& builder) const;
    // Map used to store Command pattern function pointers.
    std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;

    // The same, but for pure JSON syntax
    using MaterialConfigProcessorJSON = bool (MaterialCompiler::*)
            (const JsonishValue*, filamat::MaterialBuilder& builder) const;
    std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;
};

} // namespace matc
#endif //TNT_MATERIALCOMPILER_H
