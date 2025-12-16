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
#include "MaterialDefinition.h"

#include "Froxelizer.h"
#include "MaterialParser.h"

#include <backend/DriverEnums.h>

#include <ds/ColorPassDescriptorSet.h>

#include <details/Engine.h>

#include <private/filament/PushConstantInfo.h>

#include <utils/Logger.h>
#include <utils/Panic.h>

namespace filament {

using namespace backend;
using namespace utils;

std::unique_ptr<MaterialParser> MaterialDefinition::createParser(Backend const backend,
        FixedCapacityVector<ShaderLanguage> languages, const void* data, size_t size) {
    // unique_ptr so we don't leak MaterialParser on failures below
    auto materialParser = std::make_unique<MaterialParser>(languages, data, size);

    MaterialParser::ParseResult const materialResult = materialParser->parse();

    if (UTILS_UNLIKELY(materialResult == MaterialParser::ParseResult::ERROR_MISSING_BACKEND)) {
        CString languageNames;
        for (auto it = languages.begin(); it != languages.end(); ++it) {
            languageNames.append(CString{shaderLanguageToString(*it)});
            if (std::next(it) != languages.end()) {
                languageNames.append(", ");
            }
        }

        FILAMENT_CHECK_POSTCONDITION(
                materialResult != MaterialParser::ParseResult::ERROR_MISSING_BACKEND)
                << "the material was not built for any of the " << to_string(backend)
                << " backend's supported shader languages (" << languageNames.c_str() << ")\n";
    }

    if (backend == Backend::NOOP) {
        return materialParser;
    }

    FILAMENT_CHECK_POSTCONDITION(materialResult == MaterialParser::ParseResult::SUCCESS)
            << "could not parse the material package";

    uint32_t version = 0;
    materialParser->getMaterialVersion(&version);
    FILAMENT_CHECK_POSTCONDITION(version == MATERIAL_VERSION)
            << "Material version mismatch. Expected " << MATERIAL_VERSION << " but received "
            << version << ".";

    assert_invariant(backend != Backend::DEFAULT && "Default backend has not been resolved.");

    return materialParser;
}

std::unique_ptr<MaterialDefinition> MaterialDefinition::create(FEngine& engine,
        std::unique_ptr<MaterialParser> parser) {
    // Try checking CRC32 value for the package and skip if it's unavailable.
    if (downcast(engine).features.material.check_crc32_after_loading) {
        uint32_t parsedCrc32 = 0;
        parser->getMaterialCrc32(&parsedCrc32);

        uint32_t expectedCrc32 = parser->computeCrc32();

        if (parsedCrc32 != expectedCrc32) {
            CString name;
            parser->getName(&name);
            LOG(ERROR) << "The material '" << name.c_str_safe()
                       << "' is corrupted: crc32_expected=" << expectedCrc32
                       << ", crc32_parsed=" << parsedCrc32;
            return nullptr;
        }
    }

    uint32_t v = 0;
    parser->getShaderModels(&v);
    bitset32 shaderModels;
    shaderModels.setValue(v);

    ShaderModel const shaderModel = downcast(engine).getShaderModel();
    if (!shaderModels.test(static_cast<uint32_t>(shaderModel))) {
        CString name;
        parser->getName(&name);
        char shaderModelsString[16];
        snprintf(shaderModelsString, sizeof(shaderModelsString), "%#x", shaderModels.getValue());
        LOG(ERROR) << "The material '" << name.c_str_safe() << "' was not built for "
                   << to_string(shaderModel) << ".";
        LOG(ERROR) << "Compiled material contains shader models " << shaderModelsString << ".";
        return nullptr;
    }

    // Print a warning if the material's stereo type doesn't align with the engine's
    // setting.
    MaterialDomain materialDomain;
    UserVariantFilterMask variantFilterMask;
    parser->getMaterialDomain(&materialDomain);
    parser->getMaterialVariantFilterMask(&variantFilterMask);
    bool const hasStereoVariants =
            !(variantFilterMask & UserVariantFilterMask(UserVariantFilterBit::STE));
    if (materialDomain == MaterialDomain::SURFACE && hasStereoVariants) {
        StereoscopicType const engineStereoscopicType = engine.getConfig().stereoscopicType;
        // Default materials are always compiled with either 'instanced' or 'multiview'.
        // So, we only verify compatibility if the engine is set up for stereo.
        if (engineStereoscopicType != StereoscopicType::NONE) {
            StereoscopicType materialStereoscopicType = StereoscopicType::NONE;
            parser->getStereoscopicType(&materialStereoscopicType);
            if (materialStereoscopicType != engineStereoscopicType) {
                CString name;
                parser->getName(&name);
                LOG(WARNING) << "The stereoscopic type in the compiled material '"
                             << name.c_str_safe() << "' is " << (int) materialStereoscopicType
                             << ", which is not compatible with the engine's setting "
                             << (int) engineStereoscopicType << ".";
            }
        }
    }

    return std::make_unique<MaterialDefinition>(engine, std::move(parser));
}

void MaterialDefinition::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    perViewDescriptorSetLayout.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    perViewDescriptorSetLayoutVsm.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    descriptorSetLayout.terminate(engine.getDescriptorSetLayoutFactory(), driver);
}

MaterialDefinition::MaterialDefinition(FEngine& engine,
        std::unique_ptr<MaterialParser> materialParser)
        : mMaterialParser(std::move(materialParser)) {

    processMain();
    processBlendingMode();
    processSpecializationConstants(engine);
    processPushConstants();
    processDescriptorSets(engine);
}

void MaterialDefinition::processMain() {
    UTILS_UNUSED_IN_RELEASE bool const nameOk = mMaterialParser->getName(&name);
    assert_invariant(nameOk);

    featureLevel = [this]() -> FeatureLevel {
        // code written this way so the IDE will complain when/if we add a FeatureLevel
        uint8_t level = 1;
        mMaterialParser->getFeatureLevel(&level);
        assert_invariant(level <= 3);
        FeatureLevel featureLevel = FeatureLevel::FEATURE_LEVEL_1;
        switch (FeatureLevel(level)) {
            case FeatureLevel::FEATURE_LEVEL_0:
            case FeatureLevel::FEATURE_LEVEL_1:
            case FeatureLevel::FEATURE_LEVEL_2:
            case FeatureLevel::FEATURE_LEVEL_3:
                featureLevel = FeatureLevel(level);
                break;
        }
        return featureLevel;
    }();

    UTILS_UNUSED_IN_RELEASE bool success;

    success = mMaterialParser->getCacheId(&cacheId);
    assert_invariant(success);

    success = mMaterialParser->getSIB(&samplerInterfaceBlock);
    assert_invariant(success);

    success = mMaterialParser->getUIB(&uniformInterfaceBlock);
    assert_invariant(success);

    if (UTILS_UNLIKELY(mMaterialParser->getShaderLanguage() == ShaderLanguage::ESSL1)) {
        success = mMaterialParser->getAttributeInfo(&attributeInfo);
        assert_invariant(success);

        success = mMaterialParser->getBindingUniformInfo(&bindingUniformInfo);
        assert_invariant(success);
    }

    // Older materials will not have a subpass chunk; this should not be an error.
    if (!mMaterialParser->getSubpasses(&subpassInfo)) {
        subpassInfo.isValid = false;
    }

    mMaterialParser->getShading(&shading);
    mMaterialParser->getMaterialProperties(&materialProperties);
    mMaterialParser->getInterpolation(&interpolation);
    mMaterialParser->getVertexDomain(&vertexDomain);
    mMaterialParser->getMaterialDomain(&materialDomain);
    mMaterialParser->getMaterialVariantFilterMask(&variantFilterMask);
    mMaterialParser->getRequiredAttributes(&requiredAttributes);
    mMaterialParser->getRefractionMode(&refractionMode);
    mMaterialParser->getRefractionType(&refractionType);
    mMaterialParser->getReflectionMode(&reflectionMode);
    mMaterialParser->getTransparencyMode(&transparencyMode);
    mMaterialParser->getDoubleSided(&doubleSided);
    mMaterialParser->getCullingMode(&cullingMode);

    if (shading == Shading::UNLIT) {
        mMaterialParser->hasShadowMultiplier(&hasShadowMultiplier);
    }

    isVariantLit = shading != Shading::UNLIT || hasShadowMultiplier;

    // color write
    bool colorWrite = false;
    mMaterialParser->getColorWrite(&colorWrite);
    rasterState.colorWrite = colorWrite;

    // depth test
    bool depthTest = false;
    mMaterialParser->getDepthTest(&depthTest);
    rasterState.depthFunc = depthTest ? RasterState::DepthFunc::GE : RasterState::DepthFunc::A;

    // if doubleSided() was called we override culling()
    bool doubleSideSet = false;
    mMaterialParser->getDoubleSidedSet(&doubleSideSet);
    if (doubleSideSet) {
        doubleSidedCapability = true;
        rasterState.culling = doubleSided ? CullingMode::NONE : cullingMode;
    } else {
        rasterState.culling = cullingMode;
    }

    // specular anti-aliasing
    mMaterialParser->hasSpecularAntiAliasing(&specularAntiAliasing);
    if (specularAntiAliasing) {
        mMaterialParser->getSpecularAntiAliasingVariance(&specularAntiAliasingVariance);
        mMaterialParser->getSpecularAntiAliasingThreshold(&specularAntiAliasingThreshold);
    }

    mMaterialParser->hasCustomDepthShader(&hasCustomDepthShader);

    bool const isLit = isVariantLit || hasShadowMultiplier;
    bool const isSSR = reflectionMode == ReflectionMode::SCREEN_SPACE ||
            refractionMode == RefractionMode::SCREEN_SPACE;
    bool const hasFog = !(variantFilterMask & UserVariantFilterMask(UserVariantFilterBit::FOG));

    perViewLayoutIndex = ColorPassDescriptorSet::getIndex(isLit, isSSR, hasFog);

    mMaterialParser->getSourceShader(&source);
}

void MaterialDefinition::processBlendingMode() {
    mMaterialParser->getBlendingMode(&blendingMode);

    if (blendingMode == BlendingMode::MASKED) {
        mMaterialParser->getMaskThreshold(&maskThreshold);
    }

    if (blendingMode == BlendingMode::CUSTOM) {
        mMaterialParser->getCustomBlendFunction(&customBlendFunctions);
    }

    // blending mode
    switch (blendingMode) {
        // Do not change the MASKED behavior without checking for regressions with
        // AlphaBlendModeTest and TextureLinearInterpolationTest, with and without
        // View::BlendMode::TRANSLUCENT.
        case BlendingMode::MASKED:
        case BlendingMode::OPAQUE:
            rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            rasterState.blendFunctionDstRGB   = BlendFunction::ZERO;
            rasterState.blendFunctionDstAlpha = BlendFunction::ZERO;
            rasterState.depthWrite = true;
            break;
        case BlendingMode::TRANSPARENT:
        case BlendingMode::FADE:
            rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            rasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_ALPHA;
            rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
            rasterState.depthWrite = false;
            break;
        case BlendingMode::ADD:
            rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            rasterState.blendFunctionDstRGB   = BlendFunction::ONE;
            rasterState.blendFunctionDstAlpha = BlendFunction::ONE;
            rasterState.depthWrite = false;
            break;
        case BlendingMode::MULTIPLY:
            rasterState.blendFunctionSrcRGB   = BlendFunction::ZERO;
            rasterState.blendFunctionSrcAlpha = BlendFunction::ZERO;
            rasterState.blendFunctionDstRGB   = BlendFunction::SRC_COLOR;
            rasterState.blendFunctionDstAlpha = BlendFunction::SRC_COLOR;
            rasterState.depthWrite = false;
            break;
        case BlendingMode::SCREEN:
            rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            rasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_COLOR;
            rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_COLOR;
            rasterState.depthWrite = false;
            break;
        case BlendingMode::CUSTOM:
            rasterState.blendFunctionSrcRGB   = customBlendFunctions[0];
            rasterState.blendFunctionSrcAlpha = customBlendFunctions[1];
            rasterState.blendFunctionDstRGB   = customBlendFunctions[2];
            rasterState.blendFunctionDstAlpha = customBlendFunctions[3];
            rasterState.depthWrite = false;
    }

    // depth write
    bool depthWriteSet = false;
    mMaterialParser->getDepthWriteSet(&depthWriteSet);
    if (depthWriteSet) {
        bool depthWrite = false;
        mMaterialParser->getDepthWrite(&depthWrite);
        rasterState.depthWrite = depthWrite;
    }

    // alpha to coverage
    bool alphaToCoverageSet = false;
    mMaterialParser->getAlphaToCoverageSet(&alphaToCoverageSet);
    if (alphaToCoverageSet) {
        bool alphaToCoverage = false;
        mMaterialParser->getAlphaToCoverage(&alphaToCoverage);
        rasterState.alphaToCoverage = alphaToCoverage;
    } else {
        rasterState.alphaToCoverage = blendingMode == BlendingMode::MASKED;
    }
}

void MaterialDefinition::processSpecializationConstants(FEngine& engine) {
    // Older materials won't have a constants chunk, but that's okay.
    mMaterialParser->getConstants(&materialConstants);

    // Initialize the default specialization constant values.
    const int size = materialConstants.size() + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
    specializationConstants.reserve(size);
    specializationConstants.resize(size);

    specializationConstants[+ReservedSpecializationConstants::BACKEND_FEATURE_LEVEL] =
            int(engine.getSupportedFeatureLevel());

    // Feature level 0 doesn't support instancing.
    specializationConstants[+ReservedSpecializationConstants::CONFIG_MAX_INSTANCES] =
            int((engine.getActiveFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0)
                    ? 1 : CONFIG_MAX_INSTANCES);

    specializationConstants
            [+ReservedSpecializationConstants::CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND] =
                    engine.getDriverApi().isWorkaroundNeeded(
                            Workaround::METAL_STATIC_TEXTURE_TARGET_ERROR);

    specializationConstants[+ReservedSpecializationConstants::CONFIG_SRGB_SWAPCHAIN_EMULATION] =
            engine.getDriverApi().isWorkaroundNeeded(Workaround::EMULATE_SRGB_SWAPCHAIN);

    // The 16u below denotes the 16 bytes in a uvec4, which is how the froxel buffer is stored.
    specializationConstants[+ReservedSpecializationConstants::CONFIG_FROXEL_BUFFER_HEIGHT] =
            int(Froxelizer::getFroxelBufferByteCount(engine.getDriverApi()) / 16u);

    specializationConstants[+ReservedSpecializationConstants::CONFIG_POWER_VR_SHADER_WORKAROUNDS] =
            engine.getDriverApi().isWorkaroundNeeded(Workaround::POWER_VR_SHADER_WORKAROUNDS);

    specializationConstants[+ReservedSpecializationConstants::CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP] =
            engine.debug.shadowmap.debug_directional_shadowmap;

    specializationConstants[+ReservedSpecializationConstants::CONFIG_DEBUG_FROXEL_VISUALIZATION] =
            engine.debug.lighting.debug_froxel_visualization;

    specializationConstants[+ReservedSpecializationConstants::CONFIG_STEREO_EYE_COUNT] =
            int(engine.getConfig().stereoscopicEyeCount);

    // Actual value set in FMaterial::processSpecializationConstants.
    specializationConstants[+ReservedSpecializationConstants::CONFIG_SH_BANDS_COUNT] = 0;

    // Actual value set in FMaterial::processSpecializationConstants.
    specializationConstants[+ReservedSpecializationConstants::CONFIG_SHADOW_SAMPLING_METHOD] = 0;

    specializationConstants[+ReservedSpecializationConstants::CONFIG_FROXEL_RECORD_BUFFER_HEIGHT] =
            int(Froxelizer::getFroxelRecordBufferByteCount(engine.getDriverApi()) / 16u);

    // Initialize the rest of the reserved constants with a dummy value.
    for (size_t i = CONFIG_NEXT_RESERVED_SPEC_CONSTANT; i < CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
            i++) {
        specializationConstants[i] = 0;
    }

    // Initialize the remaining constants and specializationConstantsNameToIndex.
    for (size_t i = 0, c = materialConstants.size(); i < c; i++) {
        auto& item = materialConstants[i];

        // the key can be a string_view because mMaterialConstant owns the CString
        std::string_view const key{ item.name.data(), item.name.size() };
        specializationConstantsNameToIndex[key] = i;

        // Copy the default value to the corresponding specializationConstants entry.
        const size_t id = CONFIG_MAX_RESERVED_SPEC_CONSTANTS + i;
        switch (item.type) {
            case ConstantType::INT:
                specializationConstants[id] = item.defaultValue.i;
                break;
            case ConstantType::FLOAT:
                specializationConstants[id] = item.defaultValue.f;
                break;
            case ConstantType::BOOL:
                specializationConstants[id] = item.defaultValue.b;
                break;
        }
    }
}

void MaterialDefinition::processPushConstants() {
    FixedCapacityVector<Program::PushConstant>& vertexConstants =
            pushConstants[uint8_t(ShaderStage::VERTEX)];
    FixedCapacityVector<Program::PushConstant>& fragmentConstants =
            pushConstants[uint8_t(ShaderStage::FRAGMENT)];

    CString structVarName;
    FixedCapacityVector<MaterialPushConstant> pushConstants;
    mMaterialParser->getPushConstants(&structVarName, &pushConstants);

    vertexConstants.reserve(pushConstants.size());
    fragmentConstants.reserve(pushConstants.size());

    constexpr size_t MAX_NAME_LEN = 60;
    char buf[MAX_NAME_LEN];
    uint8_t vertexCount = 0, fragmentCount = 0;

    std::for_each(pushConstants.cbegin(), pushConstants.cend(),
            [&](MaterialPushConstant const& constant) {
                snprintf(buf, sizeof(buf), "%s.%s", structVarName.c_str(), constant.name.c_str());

                switch (constant.stage) {
                    case ShaderStage::VERTEX:
                        vertexConstants.push_back({CString(buf), constant.type});
                        vertexCount++;
                        break;
                    case ShaderStage::FRAGMENT:
                        fragmentConstants.push_back({CString(buf), constant.type});
                        fragmentCount++;
                        break;
                    case ShaderStage::COMPUTE:
                        break;
                }
            });
}

void MaterialDefinition::processDescriptorSets(FEngine& engine) {
    UTILS_UNUSED_IN_RELEASE bool success;

    success = mMaterialParser->getDescriptorBindings(&programDescriptorBindings);
    assert_invariant(success);

    backend::DescriptorSetLayout descriptorSetLayout;
    success = mMaterialParser->getDescriptorSetLayout(&descriptorSetLayout);
    assert_invariant(success);

    // get the PER_VIEW descriptor binding info
    bool const isLit = isVariantLit || hasShadowMultiplier;
    bool const isSSR = reflectionMode == ReflectionMode::SCREEN_SPACE ||
            refractionMode == RefractionMode::SCREEN_SPACE;
    bool const hasFog = !(variantFilterMask & UserVariantFilterMask(UserVariantFilterBit::FOG));

    auto perViewDescriptorSetLayout = descriptor_sets::getPerViewDescriptorSetLayout(
            materialDomain, isLit, isSSR, hasFog, false);

    auto perViewDescriptorSetLayoutVsm = descriptor_sets::getPerViewDescriptorSetLayout(
            materialDomain, isLit, isSSR, hasFog, true);

    // set the labels
    descriptorSetLayout.label = CString{ name }.append("_perMat");
    perViewDescriptorSetLayout.label = CString{ name }.append("_perView");
    perViewDescriptorSetLayoutVsm.label = CString{ name }.append("_perViewVsm");

    // get the PER_RENDERABLE and PER_VIEW descriptor binding info
    for (auto&& [bindingPoint, descriptorSetLayout] : {
            std::pair{ DescriptorSetBindingPoints::PER_RENDERABLE,
                    descriptor_sets::getPerRenderableLayout() },
            std::pair{ DescriptorSetBindingPoints::PER_VIEW,
                    perViewDescriptorSetLayout }}) {
        Program::DescriptorBindingsInfo& descriptors = programDescriptorBindings[+bindingPoint];
        descriptors.reserve(descriptorSetLayout.bindings.size());
        for (auto const& entry: descriptorSetLayout.bindings) {
            auto const& name = descriptor_sets::getDescriptorName(bindingPoint, entry.binding);
            descriptors.push_back({ name, entry.type, entry.binding });
        }
    }

    this->descriptorSetLayout = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(descriptorSetLayout) };

    this->perViewDescriptorSetLayout = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(perViewDescriptorSetLayout) };

    this->perViewDescriptorSetLayoutVsm = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(perViewDescriptorSetLayoutVsm) };
}

} // namespace filament
