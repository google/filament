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
#ifndef TNT_FILAMENT_MATERIALDEFINITION_H
#define TNT_FILAMENT_MATERIALDEFINITION_H

#include <private/filament/Variant.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/ConstantInfo.h>

#include <ds/DescriptorSetLayout.h>

#include <backend/Program.h>

namespace filament {

class FEngine;
class MaterialParser;

/** A MaterialDefinition is a parsed, unmarshalled material file, containing no state.
 *
 * Given that this is a pure read-only class, nearly all members are public without getters.
 */
struct MaterialDefinition {
    using BlendingMode = filament::BlendingMode;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;
    using VertexDomain = filament::VertexDomain;
    using TransparencyMode = filament::TransparencyMode;
    using CullingMode = backend::CullingMode;

    using AttributeInfoContainer = utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>;

    using BindingUniformInfoContainer = utils::FixedCapacityVector<
        std::tuple<uint8_t, utils::CString, backend::Program::UniformInfo>>;

    // public only due to std::make_unique().
    MaterialDefinition(FEngine& engine, std::unique_ptr<MaterialParser> parser);

    // Free GPU resources owned by this MaterialDefinition.
    void terminate(FEngine& engine);

    MaterialParser const& getMaterialParser() const noexcept { return *mMaterialParser; }

    // try to order by frequency of use
    DescriptorSetLayout perViewDescriptorSetLayout;
    DescriptorSetLayout perViewDescriptorSetLayoutVsm;
    DescriptorSetLayout descriptorSetLayout;
    backend::Program::DescriptorSetInfo programDescriptorBindings;

    backend::RasterState rasterState;
    TransparencyMode transparencyMode = TransparencyMode::DEFAULT;
    bool isVariantLit = false;
    backend::FeatureLevel featureLevel = backend::FeatureLevel::FEATURE_LEVEL_1;
    Shading shading = Shading::UNLIT;

    BlendingMode blendingMode = BlendingMode::OPAQUE;
    std::array<backend::BlendFunction, 4> customBlendFunctions = {};
    Interpolation interpolation = Interpolation::SMOOTH;
    VertexDomain vertexDomain = VertexDomain::OBJECT;
    MaterialDomain materialDomain = MaterialDomain::SURFACE;
    CullingMode cullingMode = CullingMode::NONE;
    AttributeBitset requiredAttributes;
    UserVariantFilterMask variantFilterMask = 0;
    RefractionMode refractionMode = RefractionMode::NONE;
    RefractionType refractionType = RefractionType::SOLID;
    ReflectionMode reflectionMode = ReflectionMode::DEFAULT;
    uint64_t materialProperties = 0;
    uint8_t perViewLayoutIndex = 0;

    float maskThreshold = 0.4f;
    float specularAntiAliasingVariance = 0.0f;
    float specularAntiAliasingThreshold = 0.0f;

    bool doubleSided = false;
    bool doubleSidedCapability = false;
    bool hasShadowMultiplier = false;
    bool hasCustomDepthShader = false;
    bool specularAntiAliasing = false;

    SamplerInterfaceBlock samplerInterfaceBlock;
    BufferInterfaceBlock uniformInterfaceBlock;
    SubpassInfo subpassInfo;

    BindingUniformInfoContainer bindingUniformInfo;
    AttributeInfoContainer attributeInfo;

    // Constants defined by this Material
    utils::FixedCapacityVector<MaterialConstant> materialConstants;
    // A map from the Constant name to the materialConstants index
    std::unordered_map<std::string_view, uint32_t> specializationConstantsNameToIndex;

    utils::CString name;
    uint64_t cacheId = 0;

private:
    friend class MaterialCache;
    friend class FMaterial;

    static std::unique_ptr<MaterialParser> createParser(backend::Backend const backend,
            utils::FixedCapacityVector<backend::ShaderLanguage> languages,
            const void* UTILS_NONNULL data, size_t size);

    static std::unique_ptr<MaterialDefinition> create(FEngine& engine,
            std::unique_ptr<MaterialParser> parser);

    void processMain();
    void processBlendingMode();
    void processSpecializationConstants();
    void processDescriptorSets(FEngine& engine);

    std::unique_ptr<MaterialParser> mMaterialParser;
};

} // namespace filament

#endif  // TNT_FILAMENT_MATERIALDEFINITION_H
