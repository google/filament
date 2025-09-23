#include "MaterialDefinition.h"

#include "Froxelizer.h"
#include "MaterialParser.h"

#include <backend/DriverEnums.h>

#include <ds/ColorPassDescriptorSet.h>

#include <details/Engine.h>

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
    processSpecializationConstants();
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

void MaterialDefinition::processSpecializationConstants() {
    // Older materials won't have a constants chunk, but that's okay.
    mMaterialParser->getConstants(&materialConstants);
    for (size_t i = 0, c = materialConstants.size(); i < c; i++) {
        auto& item = materialConstants[i];
        // the key can be a string_view because mMaterialConstant owns the CString
        std::string_view const key{ item.name.data(), item.name.size() };
        specializationConstantsNameToIndex[key] = i;
    }
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
