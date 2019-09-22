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

#include "IBL.h"

#include <fstream>
#include <string>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/Skybox.h>

#include <image/KtxBundle.h>
#include <image/KtxUtility.h>

#include <stb_image.h>

#include <utils/Path.h>
#include <filament/IndirectLight.h>

using namespace filament;
using namespace image;
using namespace filament::math;
using namespace utils;

static constexpr float IBL_INTENSITY = 30000.0f;

IBL::IBL(Engine& engine) : mEngine(engine) {
}

IBL::~IBL() {
    mEngine.destroy(mIndirectLight);
    mEngine.destroy(mTexture);
    mEngine.destroy(mSkybox);
    mEngine.destroy(mSkyboxTexture);
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
        return new image::KtxBundle(contents.data(), contents.size());
    };

    KtxBundle* iblKtx = createKtx(iblPath);
    KtxBundle* skyKtx = createKtx(skyPath);

    mSkyboxTexture = ktx::createTexture(&mEngine, skyKtx, false);
    mTexture = ktx::createTexture(&mEngine, iblKtx, false);

    if (!iblKtx->getSphericalHarmonics(mBands)) {
        return false;
    }

    mIndirectLight = IndirectLight::Builder()
            .reflections(mTexture)
            .irradiance(3, mBands)
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

    // Read mip-mapped cubemap
    const std::string prefix = "m";
    if (!loadCubemapLevel(&mTexture, path, 0, prefix + "0_")) return false;

    size_t numLevels = mTexture->getLevels();
    for (size_t i = 1; i<numLevels; i++) {
        const std::string levelPrefix = prefix + std::to_string(i) + "_";
        if (!loadCubemapLevel(&mTexture, path, i, levelPrefix)) return false;
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
    Texture::FaceOffsets offsets;
    Texture::PixelBufferDescriptor buffer;
    bool success = loadCubemapLevel(texture, &buffer, &offsets, path, level, levelPrefix);
    if (!success) return false;
    (*texture)->setImage(mEngine, level, std::move(buffer), offsets);
    return true;
}

bool IBL::loadCubemapLevel(
        filament::Texture** texture,
        Texture::PixelBufferDescriptor* outBuffer,
        Texture::FaceOffsets* outOffsets,
        const utils::Path& path, size_t level, std::string const& levelPrefix) const {
    static const char* faceSuffix[6] = { "px", "nx", "py", "ny", "pz", "nz" };

    size_t size = 0;
    size_t numLevels = 1;

    { // this is just a scope to avoid variable name hidding below
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

    Texture::FaceOffsets offsets;
    Texture::PixelBufferDescriptor buffer(
            malloc(faceSize * 6), faceSize * 6,
            Texture::Format::RGB, Texture::Type::UINT_10F_11F_11F_REV,
            (Texture::PixelBufferDescriptor::Callback) &free);

    bool success = true;
    uint8_t* p = static_cast<uint8_t*>(buffer.buffer);

    for (size_t j = 0; j < 6; j++) {
        offsets[j] = faceSize * j;

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

        memcpy(p + offsets[j], data, w * h * sizeof(uint32_t));

        stbi_image_free(data);
    }

    if (!success) return false;

    *outBuffer = std::move(buffer);
    *outOffsets = offsets;

    return true;
}
