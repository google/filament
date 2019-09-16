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

#include <math/mat4.h>

#include "gltfresources.h"

using namespace filament;
using namespace filament::math;
using namespace gltfio;
using namespace utils;

namespace {

class UbershaderLoader : public MaterialProvider {
public:
    UbershaderLoader(filament::Engine* engine);
    ~UbershaderLoader() {}

    MaterialSource getSource() const noexcept override { return LOAD_UBERSHADERS; }

    filament::MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label) override;

    size_t getMaterialsCount() const noexcept override;
    const filament::Material* const* getMaterials() const noexcept override;
    void destroyMaterials() override;

    Material* getMaterial(const MaterialKey& config) const;

    enum ShadingMode {
        UNLIT = 0,
        LIT = 1,
        SPECULAR_GLOSSINESS = 2,
    };

    mutable Material* mMaterials[18] = {};
    Texture* mDummyTexture = nullptr;

    filament::Engine* mEngine;
};

#define CREATE_MATERIAL(name) Material::Builder() \
    .package(GLTFRESOURCES_ ## name ## _DATA, GLTFRESOURCES_ ## name ## _SIZE) \
    .build(*mEngine);

#define MATINDEX(shading, alpha, twosided) (int(shading) + 3 * int(alpha) + (twosided ? 9 : 0))

UbershaderLoader::UbershaderLoader(Engine* engine) : mEngine(engine) {
    #ifdef EMSCRIPTEN
    unsigned char texels[4] = {};
    mDummyTexture = Texture::Builder()
            .width(1).height(1)
            .format(Texture::InternalFormat::RGBA8)
            .build(*mEngine);
    Texture::PixelBufferDescriptor pbd(texels, sizeof(texels), Texture::Format::RGBA,
            Texture::Type::UBYTE);
    mDummyTexture->setImage(*mEngine, 0, std::move(pbd));
    #endif
}

size_t UbershaderLoader::getMaterialsCount() const noexcept {
    return sizeof(mMaterials) / sizeof(mMaterials[0]);
}

const Material* const* UbershaderLoader::getMaterials() const noexcept {
    return &mMaterials[0];
}

void UbershaderLoader::destroyMaterials() {
    for (auto& material : mMaterials) {
        mEngine->destroy(material);
        material = nullptr;
    }
    mEngine->destroy(mDummyTexture);
}

Material* UbershaderLoader::getMaterial(const MaterialKey& config) const {
    const ShadingMode shading = config.unlit ? UNLIT :
            (config.useSpecularGlossiness ? SPECULAR_GLOSSINESS : LIT);
    const int matindex = MATINDEX(shading, config.alphaMode, config.doubleSided);
    if (mMaterials[matindex] != nullptr) {
        return mMaterials[matindex];
    }
    switch (matindex) {
        case MATINDEX(LIT, AlphaMode::OPAQUE, false): mMaterials[matindex] = CREATE_MATERIAL(LIT_OPAQUE_FALSE); break;
        case MATINDEX(LIT, AlphaMode::MASK, false): mMaterials[matindex] = CREATE_MATERIAL(LIT_MASKED_FALSE); break;
        case MATINDEX(LIT, AlphaMode::BLEND, false): mMaterials[matindex] = CREATE_MATERIAL(LIT_FADE_FALSE); break;
        case MATINDEX(UNLIT, AlphaMode::OPAQUE, false): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_OPAQUE_FALSE); break;
        case MATINDEX(UNLIT, AlphaMode::MASK, false): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_MASKED_FALSE); break;
        case MATINDEX(UNLIT, AlphaMode::BLEND, false): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_FADE_FALSE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::OPAQUE, false): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_OPAQUE_FALSE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::MASK, false): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_MASKED_FALSE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::BLEND, false): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_FADE_FALSE); break;
        case MATINDEX(LIT, AlphaMode::OPAQUE, true): mMaterials[matindex] = CREATE_MATERIAL(LIT_OPAQUE_TRUE); break;
        case MATINDEX(LIT, AlphaMode::MASK, true): mMaterials[matindex] = CREATE_MATERIAL(LIT_MASKED_TRUE); break;
        case MATINDEX(LIT, AlphaMode::BLEND, true): mMaterials[matindex] = CREATE_MATERIAL(LIT_FADE_TRUE); break;
        case MATINDEX(UNLIT, AlphaMode::OPAQUE, true): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_OPAQUE_TRUE); break;
        case MATINDEX(UNLIT, AlphaMode::MASK, true): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_MASKED_TRUE); break;
        case MATINDEX(UNLIT, AlphaMode::BLEND, true): mMaterials[matindex] = CREATE_MATERIAL(UNLIT_FADE_TRUE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::OPAQUE, true): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_OPAQUE_TRUE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::MASK, true): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_MASKED_TRUE); break;
        case MATINDEX(SPECULAR_GLOSSINESS, AlphaMode::BLEND, true): mMaterials[matindex] = CREATE_MATERIAL(SPECULARGLOSSINESS_FADE_TRUE); break;
    }
    return mMaterials[matindex];
}

MaterialInstance* UbershaderLoader::createMaterialInstance(MaterialKey* config, UvMap* uvmap,
        const char* label) {
    // Diagnostics are not supported with LOAD_UBERSHADERS, please use GENERATE_SHADERS instead.
    if (config->enableDiagnostics) {
        return nullptr;
    }
    gltfio::details::constrainMaterial(config, uvmap);
    auto getUvIndex = [uvmap](uint8_t srcIndex, bool hasTexture) -> int {
        return hasTexture ? int(uvmap->at(srcIndex)) - 1 : -1;
    };
    Material* material = getMaterial(*config);
    MaterialInstance* mi = material->createInstance();
    mi->setParameter("baseColorIndex",
            getUvIndex(config->baseColorUV, config->hasBaseColorTexture));
    mi->setParameter("normalIndex", getUvIndex(config->normalUV, config->hasNormalTexture));
    mi->setParameter("metallicRoughnessIndex",
            getUvIndex(config->metallicRoughnessUV, config->hasMetallicRoughnessTexture));
    mi->setParameter("aoIndex", getUvIndex(config->aoUV, config->hasOcclusionTexture));
    mi->setParameter("emissiveIndex", getUvIndex(config->emissiveUV, config->hasEmissiveTexture));

    mat3f identity;
    mi->setParameter("baseColorUvMatrix", identity);
    mi->setParameter("metallicRoughnessUvMatrix", identity);
    mi->setParameter("normalUvMatrix", identity);
    mi->setParameter("occlusionUvMatrix", identity);
    mi->setParameter("emissiveUvMatrix", identity);

    // Some WebGL implementations emit a warning at draw call time if the shader declares a sampler
    // that has not been bound to a texture, even if the texture lookup is conditional. Therefore we
    // need to ensure that every sampler parameter is bound to a dummy texture, even if it is never
    // actually sampled from.
    #ifdef EMSCRIPTEN
    TextureSampler sampler;
    mi->setParameter("normalMap", mDummyTexture, sampler);
    mi->setParameter("baseColorMap", mDummyTexture, sampler);
    mi->setParameter("metallicRoughnessMap", mDummyTexture, sampler);
    mi->setParameter("occlusionMap", mDummyTexture, sampler);
    mi->setParameter("emissiveMap", mDummyTexture, sampler);
    #endif

    return mi;
}

} // anonymous namespace

namespace gltfio {

MaterialProvider* createUbershaderLoader(filament::Engine* engine) {
    return new UbershaderLoader(engine);
}

} // namespace gltfio
