/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <filamentapp/IBL.h>

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>

#include <ktxreader/Ktx1Reader.h>

#include <filament-iblprefilter/IBLPrefilterContext.h>

#include <stb_image.h>

#include <utils/Path.h>

#include <fstream>
#include <iostream>
#include <string>

#include <string.h>

using namespace filament;
using namespace filament::math;
using namespace ktxreader;
using namespace utils;

static constexpr float IBL_INTENSITY = 30000.0f;

IBL::IBL(Engine& engine) : mEngine(engine) {
}

IBL::~IBL() {
    mEngine.destroy(mIndirectLight);
    mEngine.destroy(mTexture);
    mEngine.destroy(mSkybox);
    mEngine.destroy(mSkyboxTexture);
    mEngine.destroy(mFogTexture);
}

bool IBL::loadFromEquirect(Path const& path) {
    if (!path.exists()) {
        return false;
    }

    int w, h;
    stbi_info(path.getAbsolutePath().c_str(), &w, &h, nullptr);
    if (w != h * 2) {
        std::cerr << "not an equirectangular image!" << std::endl;
        return false;
    }

    // load image as float
    int n;
    const size_t size = w * h * sizeof(float3);
    float3* const data = (float3*)stbi_loadf(path.getAbsolutePath().c_str(), &w, &h, &n, 3);
    if (data == nullptr || n != 3) {
        std::cerr << "Could not decode image " << std::endl;
        return false;
    }

    // now load texture
    Texture::PixelBufferDescriptor buffer(
            data, size,Texture::Format::RGB, Texture::Type::FLOAT,
            [](void* buffer, size_t size, void* user) { stbi_image_free(buffer); });

    Texture* const equirect = Texture::Builder()
            .width((uint32_t)w)
            .height((uint32_t)h)
            .levels(0xff)
            .format(Texture::InternalFormat::R11F_G11F_B10F)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .build(mEngine);

    equirect->setImage(mEngine, 0, std::move(buffer));

    IBLPrefilterContext context(mEngine);
    IBLPrefilterContext::EquirectangularToCubemap equirectangularToCubemap(context);
    IBLPrefilterContext::SpecularFilter specularFilter(context);
    IBLPrefilterContext::IrradianceFilter irradianceFilter(context);

    mSkyboxTexture = equirectangularToCubemap(equirect);

    mEngine.destroy(equirect);

    mTexture = specularFilter(mSkyboxTexture);

    mFogTexture = irradianceFilter({ .generateMipmap=false }, mSkyboxTexture);
    mFogTexture->generateMipmaps(mEngine);

    mIndirectLight = IndirectLight::Builder()
            .reflections(mTexture)
            .intensity(IBL_INTENSITY)
            .build(mEngine);

    mSkybox = Skybox::Builder()
            .environment(mSkyboxTexture)
            .showSun(true)
            .build(mEngine);

    return true;
}

bool IBL::loadFromKtx(const std::string& prefix) {
    Path iblPath(prefix + "_ibl.ktx");
    if (!iblPath.exists()) {
        return false;
    }
    Path skyPath(prefix + "_skybox.ktx");
    if (!skyPath.exists()) {
        return false;
    }

    auto createKtx = [] (Path path) {
        using namespace std;
        ifstream file(path.getPath(), ios::binary);
        vector<uint8_t> contents((istreambuf_iterator<char>(file)), {});
        return new image::Ktx1Bundle(contents.data(), contents.size());
    };

    Ktx1Bundle* iblKtx = createKtx(iblPath);
    Ktx1Bundle* skyKtx = createKtx(skyPath);

    mSkyboxTexture = Ktx1Reader::createTexture(&mEngine, skyKtx, false);
    mTexture = Ktx1Reader::createTexture(&mEngine, iblKtx, false);

    // TODO: create the fog texture, it's a bit complicated because IBLPrefilter requires
    //       the source image to have miplevels, and it's not guaranteed here and also
    //       not guaranteed we can generate them (e.g. texture could be compressed)
    //IBLPrefilterContext context(mEngine);
    //IBLPrefilterContext::IrradianceFilter irradianceFilter(context);
    //mFogTexture = irradianceFilter({ .generateMipmap=false }, mSkyboxTexture);
    //mFogTexture->generateMipmaps(mEngine);


    if (!iblKtx->getSphericalHarmonics(mBands)) {
        return false;
    }
    mHasSphericalHarmonics = true;

    mIndirectLight = IndirectLight::Builder()
            .reflections(mTexture)
            .intensity(IBL_INTENSITY)
            .build(mEngine);

    mSkybox = Skybox::Builder().environment(mSkyboxTexture).showSun(true).build(mEngine);

    return true;
}

bool IBL::loadFromDirectory(const utils::Path& path) {
    // First check if KTX files are available.
    if (loadFromKtx(Path::concat(path, path.getName()))) {
        return true;
    }
    // Read spherical harmonics
    Path sh(Path::concat(path, "sh.txt"));
    if (sh.exists()) {
        std::ifstream shReader(sh);
        shReader >> std::skipws;

        std::string line;
        for (float3& band : mBands) {
            std::getline(shReader, line);
            int n = sscanf(line.c_str(), "(%f,%f,%f)", &band.r, &band.g, &band.b); // NOLINT(cert-err34-c)
            if (n != 3) return false;
        }
    } else {
        return false;
    }
    mHasSphericalHarmonics = true;

    // Read mip-mapped cubemap
    const std::string prefix = "m";
    if (!loadCubemapLevel(&mTexture, path, 0, prefix + "0_")) return false;

    size_t numLevels = mTexture->getLevels();
    for (size_t i = 1; i<numLevels; i++) {
        const std::string levelPrefix = prefix + std::to_string(i) + "_";
        loadCubemapLevel(&mTexture, path, i, levelPrefix);
    }

    if (!loadCubemapLevel(&mSkyboxTexture, path)) return false;

    mIndirectLight = IndirectLight::Builder()
            .reflections(mTexture)
            .irradiance(3, mBands)
            .intensity(IBL_INTENSITY)
            .build(mEngine);

    mSkybox = Skybox::Builder().environment(mSkyboxTexture).showSun(true).build(mEngine);

    return true;
}

bool IBL::loadCubemapLevel(filament::Texture** texture, const utils::Path& path, size_t level,
        std::string const& levelPrefix) const {
    uint32_t dim;
    Texture::PixelBufferDescriptor buffer;
    if (loadCubemapLevel(texture, &buffer, &dim, path, level, levelPrefix)) {
        (*texture)->setImage(mEngine, level, 0, 0, 0, dim, dim, 6, std::move(buffer));
        return true;
    }
    return false;
}

bool IBL::loadCubemapLevel(
        filament::Texture** texture,
        Texture::PixelBufferDescriptor* outBuffer,
        uint32_t* dim,
        const utils::Path& path, size_t level, std::string const& levelPrefix) const {
    static const char* faceSuffix[6] = { "px", "nx", "py", "ny", "pz", "nz" };

    size_t size = 0;
    size_t numLevels = 1;

    { // this is just a scope to avoid variable name hiding below
        int w, h;
        std::string faceName = levelPrefix + faceSuffix[0] + ".rgb32f";
        Path facePath(Path::concat(path, faceName));
        if (!facePath.exists()) {
            std::cerr << "The face " << faceName << " does not exist" << std::endl;
            return false;
        }
        stbi_info(facePath.getAbsolutePath().c_str(), &w, &h, nullptr);
        if (w != h) {
            std::cerr << "width != height" << std::endl;
            return false;
        }

        size = (size_t)w;

        if (!levelPrefix.empty()) {
            numLevels = (size_t)std::log2(size) + 1;
        }

        if (level == 0) {
            *texture = Texture::Builder()
                    .width((uint32_t)size)
                    .height((uint32_t)size)
                    .levels((uint8_t)numLevels)
                    .format(Texture::InternalFormat::R11F_G11F_B10F)
                    .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
                    .build(mEngine);
        }
    }

    // RGB_10_11_11_REV encoding: 4 bytes per pixel
    const size_t faceSize = size * size * sizeof(uint32_t);
    *dim = size;

    Texture::PixelBufferDescriptor buffer(
            malloc(faceSize * 6), faceSize * 6,
            Texture::Format::RGB, Texture::Type::UINT_10F_11F_11F_REV,
            (Texture::PixelBufferDescriptor::Callback) &free);

    bool success = true;
    uint8_t* p = static_cast<uint8_t*>(buffer.buffer);

    for (size_t j = 0; j < 6; j++) {
        std::string faceName = levelPrefix + faceSuffix[j] + ".rgb32f";
        Path facePath(Path::concat(path, faceName));
        if (!facePath.exists()) {
            std::cerr << "The face " << faceName << " does not exist" << std::endl;
            success = false;
            break;
        }

        int w, h, n;
        unsigned char* data = stbi_load(facePath.getAbsolutePath().c_str(), &w, &h, &n, 4);
        if (w != h || w != size) {
            std::cerr << "Face " << faceName << "has a wrong size " << w << " x " << h <<
            ", instead of " << size << " x " << size << std::endl;
            success = false;
            break;
        }

        if (data == nullptr || n != 4) {
            std::cerr << "Could not decode face " << faceName << std::endl;
            success = false;
            break;
        }

        memcpy(p + faceSize * j, data, w * h * sizeof(uint32_t));

        stbi_image_free(data);
    }

    if (!success) return false;

    *outBuffer = std::move(buffer);

    return true;
}
