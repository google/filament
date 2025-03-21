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

#include <filamat/MaterialBuilder.h>    // for MaterialBuilder:: enums

#include <private/filament/Variant.h>
#include <private/filament/SamplerInterfaceBlock.h>

#include "ShaderMinifier.h"

#include <spirv-tools/optimizer.hpp>

#include <ShaderLang.h>

#include <backend/DriverEnums.h>

#include <utils/FixedCapacityVector.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace filamat {

using SpirvBlob = std::vector<uint32_t>;
using BindingPointAndSib = std::pair<uint8_t, const filament::SamplerInterfaceBlock*>;
using SibVector = utils::FixedCapacityVector<BindingPointAndSib>;

using DescriptorInfo = std::tuple<
        utils::CString,
        filament::backend::DescriptorSetLayoutBinding,
        std::optional<filament::SamplerInterfaceBlock::SamplerInfo>>;
using DescriptorSetInfo = utils::FixedCapacityVector<DescriptorInfo>;
using DescriptorSets = std::array<DescriptorSetInfo, filament::backend::MAX_DESCRIPTOR_SET_COUNT>;

class GLSLPostProcessor {
public:
    enum Flags : uint32_t {
        PRINT_SHADERS = 1 << 0,
        GENERATE_DEBUG_INFO = 1 << 1,
    };

    GLSLPostProcessor(MaterialBuilder::Optimization optimization, uint32_t flags);

    ~GLSLPostProcessor();

    struct Config {
        filament::Variant variant;
        filament::UserVariantFilterMask variantFilter;
        MaterialBuilder::TargetApi targetApi;
        MaterialBuilder::TargetLanguage targetLanguage;
        filament::backend::ShaderStage shaderType;
        filament::backend::ShaderModel shaderModel;
        filament::backend::FeatureLevel featureLevel;
        filament::MaterialDomain domain;
        const filamat::MaterialInfo* materialInfo;
        bool hasFramebufferFetch;
        bool usesClipDistance;
        struct {
            std::vector<std::pair<uint32_t, uint32_t>> subpassInputToColorLocation;
        } glsl;
    };

    bool process(const std::string& inputShader, Config const& config,
            std::string* outputGlsl,
            SpirvBlob* outputSpirv,
            std::string* outputMsl,
            std::string* outputWgsl);

    // public so backend_test can also use it
    static void spirvToMsl(const SpirvBlob* spirv, std::string* outMsl,
            filament::backend::ShaderStage stage, filament::backend::ShaderModel shaderModel,
            bool useFramebufferFetch, const DescriptorSets& descriptorSets,
            const ShaderMinifier* minifier);

    static bool spirvToWgsl(SpirvBlob* spirv, std::string* outWsl);

private:
    struct InternalConfig {
        std::string* glslOutput = nullptr;
        SpirvBlob* spirvOutput = nullptr;
        std::string* mslOutput = nullptr;
        std::string* wgslOutput = nullptr;
        EShLanguage shLang = EShLangFragment;
        // use 100 for ES environment, 110 for desktop
         int langVersion = 0;
        ShaderMinifier minifier;
    };

    bool fullOptimization(const glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;

    bool preprocessOptimization(glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;

    /**
     * Retrieve an optimizer instance tuned for the given optimization level and shader configuration.
     */
    using OptimizerPtr = std::shared_ptr<spvtools::Optimizer>;

    static OptimizerPtr createOptimizer(
        MaterialBuilder::Optimization optimization,
        Config const &config);

    static OptimizerPtr createEmptyOptimizer();


    static void registerSizePasses(spvtools::Optimizer &optimizer, Config const &config);

    static void registerPerformancePasses(spvtools::Optimizer &optimizer, Config const &config);

    static void optimizeSpirv(OptimizerPtr optimizer, SpirvBlob &spirv);

    static void rebindImageSamplerForWGSL(std::vector<uint32_t>& spirv);

    void fixupClipDistance(SpirvBlob& spirv, GLSLPostProcessor::Config const& config) const;

    const MaterialBuilder::Optimization mOptimization;
    const bool mPrintShaders;
    const bool mGenerateDebugInfo;
};

} // namespace filamat

#endif //TNT_GLSLPOSTPROCESSOR_H
