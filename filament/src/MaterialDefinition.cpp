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
#include "filament/MaterialEnums.h"

#include <backend/DriverEnums.h>

#include <ds/ColorPassDescriptorSet.h>

#include <details/Engine.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/PushConstantInfo.h>

#include <utils/Hash.h>
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


Handle<HwProgram> MaterialDefinition::compileProgram(FEngine& engine,
        ProgramSpecialization const& specialization,
        backend::CompilerPriorityQueue const priorityQueue) const noexcept {
    assert_invariant(engine.hasFeatureLevel(featureLevel));
    Program pb(&engine.getMaterialCache().getSpecializationConstantsInternPool());
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            pb = getSurfaceProgram(engine, specialization);
            break;
        case MaterialDomain::POST_PROCESS:
            pb = getProgramWithVariants(engine, specialization, specialization.variant,
                    specialization.variant);
            break;
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            PANIC_PRECONDITION("Compute shaders not yet supported");
    }
    pb.priorityQueue(priorityQueue);

    auto const program = engine.getDriverApi().createProgram(std::move(pb),
            ImmutableCString{ name.c_str_safe() });
    assert_invariant(program);
    return program;
}

Program MaterialDefinition::getSurfaceProgram(FEngine& engine,
        ProgramSpecialization const& specialization) const noexcept {
    // filterVariant() has already been applied in generateCommands(), shouldn't be needed here
    // if we're unlit, we don't have any bits that correspond to lit materials
    assert_invariant(specialization.variant ==
                     Variant::filterVariant(specialization.variant, isVariantLit));

    assert_invariant(!Variant::isReserved(specialization.variant));

    Variant const vertexVariant   = Variant::filterVariantVertex(specialization.variant);
    Variant const fragmentVariant = Variant::filterVariantFragment(specialization.variant);

    Program pb = getProgramWithVariants(engine, specialization, vertexVariant, fragmentVariant);
    pb.multiview(
            engine.getConfig().stereoscopicType == StereoscopicType::MULTIVIEW &&
            Variant::isStereoVariant(specialization.variant));
    return pb;
}

Program MaterialDefinition::getProgramWithVariants(FEngine const& engine,
        ProgramSpecialization const& specialization,
        Variant vertexVariant,
        Variant fragmentVariant) const {
    const ShaderModel sm = engine.getShaderModel();
    const bool isNoop = engine.getBackend() == Backend::NOOP;
    const Variant variant = specialization.variant;

    /*
     * Vertex shader
     */

    MaterialParser const& parser = getMaterialParser();

    filaflat::ShaderContent& vsBuilder = engine.getVertexShaderContent();

    UTILS_UNUSED_IN_RELEASE bool const vsOK = parser.getShader(vsBuilder, sm,
            vertexVariant, ShaderStage::VERTEX);

    FILAMENT_CHECK_POSTCONDITION(isNoop || (vsOK && !vsBuilder.empty()))
            << "The material '" << name.c_str()
            << "' has not been compiled to include the required GLSL or SPIR-V chunks for the "
               "vertex shader (variant="
            << +variant.key << ", filtered=" << +vertexVariant.key << ").";

    /*
     * Fragment shader
     */

    filaflat::ShaderContent& fsBuilder = engine.getFragmentShaderContent();

    UTILS_UNUSED_IN_RELEASE bool const fsOK = parser.getShader(fsBuilder, sm,
            fragmentVariant, ShaderStage::FRAGMENT);

    FILAMENT_CHECK_POSTCONDITION(isNoop || (fsOK && !fsBuilder.empty()))
            << "The material '" << name.c_str()
            << "' has not been compiled to include the required GLSL or SPIR-V chunks for the "
               "fragment shader (variant="
            << +variant.key << ", filtered=" << +fragmentVariant.key << ").";

    Program program(&engine.getMaterialCache().getSpecializationConstantsInternPool());
    program.shader(ShaderStage::VERTEX, vsBuilder.data(), vsBuilder.size())
            .shader(ShaderStage::FRAGMENT, fsBuilder.data(), fsBuilder.size())
            .shaderLanguage(parser.getShaderLanguage())
            .diagnostics(name,
                    [variant, vertexVariant, fragmentVariant](utils::CString const& name,
                            io::ostream& out) -> io::ostream& {
                        return out << name.c_str_safe() << ", variant=(" << io::hex << +variant.key
                                   << io::dec << "), vertexVariant=(" << io::hex
                                   << +vertexVariant.key << io::dec << "), fragmentVariant=("
                                   << io::hex << +fragmentVariant.key << io::dec << ")";
                    });

    if (UTILS_UNLIKELY(parser.getShaderLanguage() == ShaderLanguage::ESSL1)) {
        assert_invariant(!bindingUniformInfo.empty());
        for (auto const& [index, name, uniforms] : bindingUniformInfo) {
            program.uniforms(uint32_t(index), name, uniforms);
        }
        program.attributes(attributeInfo);
    }

    program.descriptorBindings(+DescriptorSetBindingPoints::PER_VIEW,
            programDescriptorBindings[+DescriptorSetBindingPoints::PER_VIEW]);
    program.descriptorBindings(+DescriptorSetBindingPoints::PER_RENDERABLE,
            programDescriptorBindings[+DescriptorSetBindingPoints::PER_RENDERABLE]);
    program.descriptorBindings(+DescriptorSetBindingPoints::PER_MATERIAL,
            programDescriptorBindings[+DescriptorSetBindingPoints::PER_MATERIAL]);
    program.specializationConstants(specialization.specializationConstants);

    program.pushConstants(ShaderStage::VERTEX, pushConstants[uint8_t(ShaderStage::VERTEX)]);
    program.pushConstants(ShaderStage::FRAGMENT, pushConstants[uint8_t(ShaderStage::FRAGMENT)]);

    // TODO(exv): we'll probably eventually want to replace this with the hash of the
    // specialization, but there may be clients which depend on this value being stable across
    // versions.
    program.cacheId(hash::combine(size_t(cacheId), variant.key));

    return program;
}


void MaterialDefinition::acquirePrograms(FEngine& engine,
        utils::Slice<Handle<HwProgram>> programCache,
        utils::Slice<const backend::Program::SpecializationConstant> specializationConstants,
        bool isDefaultMaterial) const {
    MaterialCache& materialCache = engine.getMaterialCache();
    ShaderModel const shaderModel = engine.getShaderModel();
    bool const isStereoSupported = engine.getDriverApi().isStereoSupported();

    ProgramSpecialization specialization = {
        .programCacheId = cacheId,
        .specializationConstants = specializationConstants,
    };

    // We acquire an entry for all variants in the program cache, but we don't compile them.
    // Programs that are acquired but aren't compiled simply hold onto an empty entry in the program
    // cache which is initialized later.
    for (auto variant: getVariants()) {
        if (UTILS_LIKELY(hasVariant(variant, shaderModel, isStereoSupported))) {
            specialization.variant = variant;
            programCache[variant.key] = materialCache.acquireProgram(specialization);
        }
    }

    if (UTILS_UNLIKELY(isDefaultMaterial || hasCustomDepthShader)) {
        // Do depth variants. We only do these if this is a default material or if the material has
        // custom depth variants.
        if (UTILS_UNLIKELY(isDefaultMaterial && !engine.getDriverApi().isWorkaroundNeeded(
                    Workaround::DISABLE_DEPTH_PRECACHE_FOR_DEFAULT_MATERIAL))) {
            // Precache depth programs.
            for (auto variant: getDepthVariants()) {
                if (UTILS_LIKELY(hasVariant(variant, shaderModel, isStereoSupported))) {
                    specialization.variant = variant;
                    programCache[variant.key] = materialCache.acquireAndPrepareProgram(engine,
                            *this, specialization, CompilerPriorityQueue::HIGH);
                }
            }
        } else {
            // Don't precache depth programs.
            for (auto variant: getDepthVariants()) {
                if (UTILS_LIKELY(hasVariant(variant, shaderModel, isStereoSupported))) {
                    specialization.variant = variant;
                    programCache[variant.key] = materialCache.acquireProgram(specialization);
                }
            }
        }
    }
}

void MaterialDefinition::releasePrograms(FEngine& engine,
        utils::Slice<Handle<HwProgram>> programCache,
        utils::Slice<const backend::Program::SpecializationConstant> specializationConstants,
        bool isDefaultMaterial) const {
    MaterialCache& materialCache = engine.getMaterialCache();
    ShaderModel const shaderModel = engine.getShaderModel();
    bool const isStereoSupported = engine.getDriverApi().isStereoSupported();

    ProgramSpecialization specialization = {
            .programCacheId = cacheId,
            .specializationConstants = specializationConstants,
    };

    for (auto variant : getVariants()) {
        if (UTILS_LIKELY(hasVariant(variant, shaderModel, isStereoSupported))) {
            specialization.variant = variant;
            materialCache.releaseProgram(engine, specialization);
            programCache[variant.key].clear();
        }
    }

    if (UTILS_UNLIKELY(isDefaultMaterial || hasCustomDepthShader)) {
        for (auto variant: getDepthVariants()) {
            if (UTILS_LIKELY(hasVariant(variant, shaderModel, isStereoSupported))) {
                specialization.variant = variant;
                materialCache.releaseProgram(engine, specialization);
                programCache[variant.key].clear();
            }
        }
    }
}

bool MaterialDefinition::hasVariant(Variant const variant,
        ShaderModel const sm, bool isStereoSupported) const noexcept {
    if (!isStereoSupported && Variant::isStereoVariant(variant)) {
        return false;
    }

    Variant vertexVariant, fragmentVariant;
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            vertexVariant = Variant::filterVariantVertex(variant);
            fragmentVariant = Variant::filterVariantFragment(variant);
            break;
        case MaterialDomain::POST_PROCESS:
            vertexVariant = fragmentVariant = variant;
            break;
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            return false;
    }
    if (!mMaterialParser->hasShader(sm, vertexVariant, ShaderStage::VERTEX)) {
        return false;
    }
    if (!mMaterialParser->hasShader(sm, fragmentVariant, ShaderStage::FRAGMENT)) {
        return false;
    }
    return true;
}

utils::Slice<const Variant> MaterialDefinition::getVariants() const noexcept {
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            return isVariantLit ? VariantUtils::getLitVariants()
                                : VariantUtils::getUnlitVariants();
        case MaterialDomain::POST_PROCESS:
            return VariantUtils::getPostProcessVariants();
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            PANIC_PRECONDITION("Compute shaders not yet supported");
    }
}

utils::Slice<const Variant> MaterialDefinition::getDepthVariants() const noexcept {
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            return isVariantLit ? VariantUtils::getLitDepthVariants()
                                : VariantUtils::getUnlitDepthVariants();
        case MaterialDomain::POST_PROCESS:
            return {};
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            PANIC_PRECONDITION("Compute shaders not yet supported");
    }
}

} // namespace filament
