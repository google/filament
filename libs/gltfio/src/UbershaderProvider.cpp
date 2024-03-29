/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gltfio/MaterialProvider.h>

#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <math/mat4.h>

#include <utils/Log.h>

#include "ArchiveCache.h"

using namespace filament;
using namespace filament::math;
using namespace filament::uberz;
using namespace filament::gltfio;
using namespace utils;

#if !defined(NDEBUG)
io::ostream& operator<<(io::ostream& out, const ArchiveRequirements& reqs);
#endif

namespace {

static void prepareConfig(MaterialKey* config, const char* label) {
    if (config->hasVolume && config->hasSheen) {
        slog.w << "Volume and sheen are not supported together in ubershader mode,"
                  " removing sheen (" << label << ")." << io::endl;
        config->hasSheen = false;
    }

    if (config->hasTransmission && config->hasSheen) {
        slog.w << "Transmission and sheen are not supported together in ubershader mode,"
                  " removing sheen (" << label << ")." << io::endl;
        config->hasSheen = false;
    }

    const bool clearCoatConflict = config->hasVolume || config->hasTransmission ||
            config->hasSheen || config->hasIOR;

    // Due to sampler overload, disable transmission if necessary and print a friendly warning.
    if (config->hasClearCoat && clearCoatConflict) {
        slog.w << "Volume, transmission, sheen and IOR are not supported in ubershader mode for clearcoat"
                  " materials (" << label << ")." << io::endl;
        config->hasVolume = false;
        config->hasTransmission = false;
        config->hasSheen = false;
        config->hasIOR = false;
    }
}

class UbershaderProvider : public MaterialProvider {
public:
    UbershaderProvider(Engine* engine, const void* archive, size_t archiveByteCount);
    ~UbershaderProvider() {}

    MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label, const char* extras) override;

    Material* getMaterial(MaterialKey* config, UvMap* uvmap, const char* label) override;

    size_t getMaterialsCount() const noexcept override;
    const Material* const* getMaterials() const noexcept override;
    void destroyMaterials() override;

    bool needsDummyData(VertexAttribute attrib) const noexcept override {
        switch (attrib) {
            case VertexAttribute::UV0:
            case VertexAttribute::UV1:
            case VertexAttribute::COLOR:
                return true;
            default:
                return false;
        }
    }

    Material* getMaterial(const MaterialKey& config) const;

    mutable ArchiveCache mMaterials;
    Texture* mDummyTexture = nullptr;

    Engine* const mEngine;
};

UbershaderProvider::UbershaderProvider(Engine* engine, const void* archive, size_t archiveByteCount)
        : mMaterials(*engine), mEngine(engine) {
    unsigned char texels[4] = {};
    mDummyTexture = Texture::Builder()
            .width(1).height(1)
            .format(Texture::InternalFormat::RGBA8)
            .build(*mEngine);
    Texture::PixelBufferDescriptor pbd(texels, sizeof(texels), Texture::Format::RGBA,
            Texture::Type::UBYTE);
    mDummyTexture->setImage(*mEngine, 0, std::move(pbd));
    mMaterials.load(archive, archiveByteCount);
}

size_t UbershaderProvider::getMaterialsCount() const noexcept {
    return mMaterials.getMaterialsCount();
}

const Material* const* UbershaderProvider::getMaterials() const noexcept {
    return mMaterials.getMaterials();
}

void UbershaderProvider::destroyMaterials() {
    mMaterials.destroyMaterials();
    mEngine->destroy(mDummyTexture);
}

Material* UbershaderProvider::getMaterial(const MaterialKey& config) const {
    const Shading shading = config.unlit ? Shading::UNLIT :
            (config.useSpecularGlossiness ? Shading::SPECULAR_GLOSSINESS : Shading::LIT);

    BlendingMode blending;
    switch (config.alphaMode) {
        case AlphaMode::OPAQUE: blending = BlendingMode::OPAQUE; break;
        case AlphaMode::MASK:   blending = BlendingMode::MASKED; break;
        case AlphaMode::BLEND:  blending = BlendingMode::FADE; break;
    }

    ArchiveRequirements requirements = { shading, blending };
    requirements.features["Sheen"] = config.hasSheen;
    requirements.features["Transmission"] = config.hasTransmission;
    requirements.features["Volume"] = config.hasVolume;
    requirements.features["Ior"] = config.hasIOR;
    requirements.features["VertexColors"] = config.hasVertexColors;
    requirements.features["BaseColorTexture"] = config.hasBaseColorTexture;
    requirements.features["NormalTexture"] = config.hasNormalTexture;
    requirements.features["OcclusionTexture"] = config.hasOcclusionTexture;
    requirements.features["EmissiveTexture"] = config.hasEmissiveTexture;
    requirements.features["MetallicRoughnessTexture"] = config.hasMetallicRoughnessTexture;
    requirements.features["ClearCoatTexture"] = config.hasClearCoatTexture;
    requirements.features["ClearCoatRoughnessTexture"] = config.hasClearCoatRoughnessTexture;
    requirements.features["ClearCoatNormalTexture"] = config.hasClearCoatNormalTexture;
    requirements.features["ClearCoat"] = config.hasClearCoat;
    requirements.features["TextureTransforms"] = config.hasTextureTransforms;
    requirements.features["TransmissionTexture"] = config.hasTransmissionTexture;
    requirements.features["SheenColorTexture"] = config.hasSheenColorTexture;
    requirements.features["SheenRoughnessTexture"] = config.hasSheenRoughnessTexture;
    requirements.features["VolumeThicknessTexture"] = config.hasVolumeThicknessTexture;

    if (Material* mat = mMaterials.getMaterial(requirements); mat != nullptr) {
        return mat;
    }

#ifndef NDEBUG
    slog.w << "Failed to find material with features:\n" << requirements << io::endl;
#endif

    return nullptr;
}

Material* UbershaderProvider::getMaterial(MaterialKey* config, UvMap* uvmap, const char* label) {
    prepareConfig(config, label);
    constrainMaterial(config, uvmap);
    Material* material = getMaterial(*config);
    if (material == nullptr) {
#ifndef NDEBUG
        slog.w << "Using fallback material for " << label << "." << io::endl;
#endif
        material = mMaterials.getDefaultMaterial();
    }
    return material;
}


MaterialInstance* UbershaderProvider::createMaterialInstance(MaterialKey* config, UvMap* uvmap,
        const char* label, const char* extras) {
    // Diagnostics are not supported with LOAD_UBERSHADERS, please use GENERATE_SHADERS instead.
    if (config->enableDiagnostics) {
        return nullptr;
    }

    Material* material = getMaterial(config, uvmap, label);

    auto getUvIndex = [uvmap](uint8_t srcIndex, bool hasTexture) -> int {
        return hasTexture ? int(uvmap->at(srcIndex)) - 1 : -1;
    };

    MaterialInstance* mi = material->createInstance(label);
    mi->setParameter("baseColorIndex",
            getUvIndex(config->baseColorUV, config->hasBaseColorTexture));
    mi->setParameter("normalIndex", getUvIndex(config->normalUV, config->hasNormalTexture));
    mi->setParameter("metallicRoughnessIndex",
            getUvIndex(config->metallicRoughnessUV, config->hasMetallicRoughnessTexture));
    mi->setParameter("aoIndex", getUvIndex(config->aoUV, config->hasOcclusionTexture));
    mi->setParameter("emissiveIndex", getUvIndex(config->emissiveUV, config->hasEmissiveTexture));

    mi->setDoubleSided(config->doubleSided);

    mi->setCullingMode(config->doubleSided ?
            MaterialInstance::CullingMode::NONE :
            MaterialInstance::CullingMode::BACK);

    mi->setTransparencyMode(config->doubleSided ?
            MaterialInstance::TransparencyMode::TWO_PASSES_TWO_SIDES :
            MaterialInstance::TransparencyMode::DEFAULT);

    mat3f identity;
    mi->setParameter("baseColorUvMatrix", identity);
    mi->setParameter("metallicRoughnessUvMatrix", identity);
    mi->setParameter("normalUvMatrix", identity);
    mi->setParameter("occlusionUvMatrix", identity);
    mi->setParameter("emissiveUvMatrix", identity);

    if (config->hasClearCoat) {
        mi->setParameter("clearCoatIndex",
                getUvIndex(config->clearCoatUV, config->hasClearCoatTexture));
        mi->setParameter("clearCoatRoughnessIndex",
                getUvIndex(config->clearCoatRoughnessUV, config->hasClearCoatRoughnessTexture));
        mi->setParameter("clearCoatNormalIndex",
                getUvIndex(config->clearCoatNormalUV, config->hasClearCoatNormalTexture));
        mi->setParameter("clearCoatUvMatrix", identity);
        mi->setParameter("clearCoatRoughnessUvMatrix", identity);
        mi->setParameter("clearCoatNormalUvMatrix", identity);
    } else {
        if (config->hasSheen) {
            mi->setParameter("sheenColorIndex",
                    getUvIndex(config->sheenColorUV, config->hasSheenColorTexture));
            mi->setParameter("sheenRoughnessIndex",
                    getUvIndex(config->sheenRoughnessUV, config->hasSheenRoughnessTexture));
            mi->setParameter("sheenColorUvMatrix", identity);
            mi->setParameter("sheenRoughnessUvMatrix", identity);

        }
        if (config->hasVolume) {
            mi->setParameter("volumeThicknessUvMatrix", identity);
            mi->setParameter("volumeThicknessIndex",
                    getUvIndex(config->transmissionUV, config->hasVolumeThicknessTexture));
        }
        if (config->hasTransmission) {
            mi->setParameter("transmissionUvMatrix", identity);
            mi->setParameter("transmissionIndex",
                    getUvIndex(config->transmissionUV, config->hasTransmissionTexture));
        }
    }

    TextureSampler sampler;
    mi->setParameter("normalMap", mDummyTexture, sampler);
    mi->setParameter("baseColorMap", mDummyTexture, sampler);
    mi->setParameter("metallicRoughnessMap", mDummyTexture, sampler);
    mi->setParameter("occlusionMap", mDummyTexture, sampler);
    mi->setParameter("emissiveMap", mDummyTexture, sampler);

    FeatureMap features = mMaterials.getFeatureMap(material);
    const auto& needsTexture = [&features](std::string_view featureName) {
        auto iter = features.find(featureName);
        return iter != features.end() && iter.value() != ArchiveFeature::UNSUPPORTED;
    };

    if (needsTexture("ClearCoatTexture")) {
        mi->setParameter("clearCoatMap", mDummyTexture, sampler);
    }

    if (needsTexture("ClearCoatRoughnessTexture")) {
        mi->setParameter("clearCoatRoughnessMap", mDummyTexture, sampler);
    }

    if (needsTexture("ClearCoatNormalTexture")) {
        mi->setParameter("clearCoatNormalMap", mDummyTexture, sampler);
    }

    if (needsTexture("VolumeThicknessTexture")) {
        mi->setParameter("volumeThicknessMap", mDummyTexture, sampler);
    }

    if (needsTexture("TransmissionTexture")) {
        mi->setParameter("transmissionMap", mDummyTexture, sampler);
    }

    if (needsTexture("SheenColorTexture")) {
        mi->setParameter("sheenColorMap", mDummyTexture, sampler);
    }

    if (needsTexture("SheenRoughnessTexture")) {
        mi->setParameter("sheenRoughnessMap", mDummyTexture, sampler);
    }

    if (mi->getMaterial()->hasParameter("ior")) {
        mi->setParameter("ior", 1.5f);
    }
    if (mi->getMaterial()->hasParameter("reflectance")) {
        mi->setParameter("reflectance", 0.5f);
    }

    return mi;
}

} // anonymous namespace

namespace filament::gltfio {

MaterialProvider* createUbershaderProvider(Engine* engine, const void* archive,
        size_t archiveByteCount) {
    return new UbershaderProvider(engine, archive, archiveByteCount);
}

} // namespace filament::gltfio


#if !defined(NDEBUG)

inline
const char* toString(Shading shadingModel) noexcept {
    switch (shadingModel) {
        case Shading::UNLIT: return "unlit";
        case Shading::LIT: return "lit";
        case Shading::SUBSURFACE: return "subsurface";
        case Shading::CLOTH: return "cloth";
        case Shading::SPECULAR_GLOSSINESS: return "specularGlossiness";
    }
}

inline
const char* toString(BlendingMode blendingMode) noexcept {
    switch (blendingMode) {
        case BlendingMode::OPAQUE: return "opaque";
        case BlendingMode::TRANSPARENT: return "transparent";
        case BlendingMode::ADD: return "add";
        case BlendingMode::MASKED: return "masked";
        case BlendingMode::FADE: return "fade";
        case BlendingMode::MULTIPLY: return "multiply";
        case BlendingMode::SCREEN: return "screen";
        case BlendingMode::CUSTOM: return "custom";
    }
}

#if !defined(NDEBUG)
io::ostream& operator<<(io::ostream& out, const ArchiveRequirements& reqs) {
    out << "    ShadingModel = " << toString(reqs.shadingModel) << '\n'
        << "    BlendingMode = " << toString(reqs.blendingMode) << '\n';
    for (const auto& pair : reqs.features) {
        out << "    " << pair.first.c_str() << " = " << (pair.second ? "true" : "false") << '\n';
    }
    return out;
}
#endif

#endif
