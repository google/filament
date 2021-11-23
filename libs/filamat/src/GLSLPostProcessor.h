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

#ifndef TNT_GLSLPOSTPROCESSOR_H
#define TNT_GLSLPOSTPROCESSOR_H

#include <string>
#include <vector>

#include <backend/DriverEnums.h>

#include "filamat/MaterialBuilder.h"    // for MaterialBuilder:: enums

#include "ShaderMinifier.h"

#include <ShaderLang.h>

#include <spirv-tools/optimizer.hpp>

#include <memory>

namespace filamat {

using SpirvBlob = std::vector<uint32_t>;

class GLSLPostProcessor {
public:
    enum Flags : uint32_t {
        PRINT_SHADERS = 1 << 0,
        GENERATE_DEBUG_INFO = 1 << 1,
    };

    GLSLPostProcessor(MaterialBuilder::Optimization optimization, uint32_t flags);

    ~GLSLPostProcessor();

    struct Config {
        filament::backend::ShaderType shaderType;
        filament::backend::ShaderModel shaderModel;
        filament::MaterialDomain domain;
        bool hasFramebufferFetch;
        struct {
            std::vector<std::pair<uint32_t, uint32_t>> subpassInputToColorLocation;
        } glsl;
    };

    bool process(const std::string& inputShader, Config const& config,
            std::string* outputGlsl,
            SpirvBlob* outputSpirv,
            std::string* outputMsl);

private:
    struct InternalConfig {
        std::string* glslOutput = nullptr;
        SpirvBlob* spirvOutput = nullptr;
        std::string* mslOutput = nullptr;
        EShLanguage shLang = EShLangFragment;
        int langVersion = 0;
        ShaderMinifier minifier;
    };

    void fullOptimization(const glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;
    void preprocessOptimization(glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;

    /**
     * Retrieve an optimizer instance tuned for the given optimization level and shader configuration.
     */
    using OptimizerPtr = std::shared_ptr<spvtools::Optimizer>;
    static OptimizerPtr createOptimizer(
            MaterialBuilder::Optimization optimization,
            Config const& config);

    static void registerSizePasses(spvtools::Optimizer& optimizer, Config const& config);
    static void registerPerformancePasses(spvtools::Optimizer& optimizer, Config const& config);

    void optimizeSpirv(OptimizerPtr optimizer, SpirvBlob& spirv) const;
    void spirvToToMsl(const SpirvBlob *spirv, std::string *outMsl, const Config &config,
            ShaderMinifier& minifier) const;

    const MaterialBuilder::Optimization mOptimization;
    const bool mPrintShaders;
    const bool mGenerateDebugInfo;
};

} // namespace filamat

#endif //TNT_GLSLPOSTPROCESSOR_H
