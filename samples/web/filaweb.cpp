/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "filaweb.h"

#include <string>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include <stb_image.h>

using namespace filament;
using namespace std;

extern "C" void render() {
    filaweb::Application::get()->render();
}

extern "C" void resize(uint32_t width, uint32_t height, double pixelRatio) {
    filaweb::Application::get()->resize(width, height, pixelRatio);
}

extern "C" void mouse(int x, int y, int wx, int wy, int buttons) {
    // We are careful not to pass down negative numbers, doing so would cause a numeric
    // representation exception in WebAssembly.
    x = std::max(0, x);
    y = std::max(0, y);
    filaweb::Application::get()->mouse(x, y, wx, wy, buttons);
}

namespace filaweb {

Asset getRawFile(const char* name) {
    // Obtain size from JavaScript.
    uint32_t nbytes;
    EM_ASM({
        var nbytes = $0 >> 2;
        var name = UTF8ToString($1);
        HEAP32[nbytes] = assets[name].data.byteLength;
    }, &nbytes, name);

    // Move the data from JavaScript.
    uint8_t* data = new uint8_t[nbytes];
    EM_ASM({
        var data = $0;
        var name = UTF8ToString($1);
        HEAPU8.set(assets[name].data, data);
        assets[name].data = null;
    }, data, name);
    return {
        .data = decltype(Asset::data)(data),
        .nbytes = nbytes
    };
}

Asset getTexture(const char* name) {
    Asset result = getRawFile(name);
    int width, height, ncomp;
    stbi_info_from_memory(result.data.get(), result.nbytes, &width, &height, &ncomp);
    const uint32_t nbytes = width * height * 4;
    uint8_t* texels = new uint8_t[nbytes];
    stbi_uc* decoded = stbi_load_from_memory(result.data.get(), result.nbytes, &width, &height,
            &ncomp, 4);
    memcpy(texels, decoded, nbytes);
    stbi_image_free(decoded);
    return {
        .data = decltype(Asset::data)(texels),
        .nbytes = nbytes,
        .width = uint32_t(width),
        .height = uint32_t(height)
    };
}

Asset getCubemap(const char* name) {
    // Obtain number of miplevels and prefix string.
    uint32_t nmips;
    char prefix[128] = {};
    EM_ASM({
        var nmips = $0 >> 2;
        var name = UTF8ToString($1);
        var prefix = $2;
        stringToUTF8(assets[name].name, prefix, 127);
        HEAP32[nmips] = assets[name].nmips;
    }, &nmips, name, &prefix[0]);

    // Build a flat list of mips for each cubemap face.
    Asset* envFaces = new Asset[nmips * 6];
    for (uint32_t mip = 0, i = 0; mip < nmips; ++mip) {
        const string mipPrefix = string(prefix) + string("m") + to_string(mip) + "_";
        auto get = [&](const char* suffix) {
            string key = mipPrefix + suffix;
            envFaces[i++] = getTexture(key.c_str());
        };
        get("px.rgbm");
        get("nx.rgbm");
        get("py.rgbm");
        get("ny.rgbm");
        get("pz.rgbm");
        get("nz.rgbm");
    }

    // Ditto but for the blurry sky.
    Asset* skyFaces = new Asset[6];
    uint32_t i = 0;
    auto get = [&](const char* suffix) {
        string key = string(prefix) + suffix;
        skyFaces[i++] = getTexture(key.c_str());
    };
    get("px.rgbm");
    get("nx.rgbm");
    get("py.rgbm");
    get("ny.rgbm");
    get("pz.rgbm");
    get("nz.rgbm");

    // Load the spherical harmonics coefficients.
    Asset* shCoeffs = new Asset;
    string key = string(prefix) + string("sh.txt");
    *shCoeffs = getRawFile(key.c_str());

    return {
        .data = decltype(Asset::data)(),
        .nbytes = 0,
        .width = envFaces[0].width,
        .height = envFaces[0].height,
        .envMipCount = nmips,
        .envShCoeffs = decltype(Asset::envShCoeffs)(shCoeffs),
        .envFaces = decltype(Asset::envFaces)(envFaces),
        .skyFaces = decltype(Asset::skyFaces)(skyFaces),
    };
}

SkyLight getSkyLight(Engine& engine, const char* name) {
    SkyLight result;

    // Pull the data out of JavaScript.
    static auto asset = filaweb::getCubemap(name);
    printf("%s: %d x %d, %d mips\n", name,  asset.width, asset.height, asset.envMipCount);

    // Parse the coefficients.
    std::istringstream shReader((const char*) asset.envShCoeffs->data.get());
    shReader >> std::skipws;
    std::string line;
    for (size_t i = 0; i < 9; i++) {
        std::getline(shReader, line);
        int n = sscanf(line.c_str(), "(%f,%f,%f)",
                &result.bands[i].r, &result.bands[i].g, &result.bands[i].b);
        if (n != 3) {
            abort();
        }
    }

    // Copy over the miplevels for the indirect light.
    Texture* texture = Texture::Builder()
        .width(asset.width)
        .height(asset.height)
        .levels(asset.envMipCount)
        .format(Texture::InternalFormat::RGBM)
        .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
        .build(engine);
    size_t size = asset.width;
    uint32_t i = 0;
    for (uint32_t mip = 0; mip < asset.envMipCount; ++mip, size >>= 1) {
        const size_t faceSize = size * size * 4;
        Texture::FaceOffsets offsets;
        offsets.px = faceSize * 0;
        offsets.nx = faceSize * 1;
        offsets.py = faceSize * 2;
        offsets.ny = faceSize * 3;
        offsets.pz = faceSize * 4;
        offsets.nz = faceSize * 5;
        Texture::PixelBufferDescriptor buffer(
                malloc(faceSize * 6), faceSize * 6,
                Texture::Format::RGBM, Texture::Type::UBYTE,
                [](void* buffer, size_t size, void* user) {
                    free(buffer);
                }, /* user = */ nullptr);
        uint8_t* pixels = static_cast<uint8_t*>(buffer.buffer);
        auto& px = asset.envFaces[i++];
        auto& nx = asset.envFaces[i++];
        auto& py = asset.envFaces[i++];
        auto& ny = asset.envFaces[i++];
        auto& pz = asset.envFaces[i++];
        auto& nz = asset.envFaces[i++];
        memcpy(pixels + offsets.px, px.data.get(), faceSize);
        memcpy(pixels + offsets.nx, nx.data.get(), faceSize);
        memcpy(pixels + offsets.py, py.data.get(), faceSize);
        memcpy(pixels + offsets.ny, ny.data.get(), faceSize);
        memcpy(pixels + offsets.pz, pz.data.get(), faceSize);
        memcpy(pixels + offsets.nz, nz.data.get(), faceSize);
        px.data.reset();
        nx.data.reset();
        py.data.reset();
        ny.data.reset();
        pz.data.reset();
        nz.data.reset();
        texture->setImage(engine, mip, std::move(buffer), offsets);
    }

    result.indirectLight = IndirectLight::Builder()
        .reflections(texture)
        .irradiance(3, result.bands)
        .intensity(30000.0f)
        .build(engine);

    // Copy a single miplevel for the blurry skybox
    size = asset.skyFaces[0].width;
    Texture* skybox = Texture::Builder()
        .width(size)
        .height(size)
        .levels(1)
        .format(Texture::InternalFormat::RGBM)
        .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
        .build(engine);
    {
        const size_t faceSize = size * size * 4;
        Texture::FaceOffsets offsets;
        offsets.px = faceSize * 0;
        offsets.nx = faceSize * 1;
        offsets.py = faceSize * 2;
        offsets.ny = faceSize * 3;
        offsets.pz = faceSize * 4;
        offsets.nz = faceSize * 5;
        Texture::PixelBufferDescriptor buffer(
                malloc(faceSize * 6), faceSize * 6,
                Texture::Format::RGBA, Texture::Type::UBYTE,
                [](void* buffer, size_t size, void* user) {
                    free(buffer);
                }, /* user = */ nullptr);
        uint8_t* pixels = static_cast<uint8_t*>(buffer.buffer);
        i = 0;
        auto& px = asset.skyFaces[i++];
        auto& nx = asset.skyFaces[i++];
        auto& py = asset.skyFaces[i++];
        auto& ny = asset.skyFaces[i++];
        auto& pz = asset.skyFaces[i++];
        auto& nz = asset.skyFaces[i++];
        memcpy(pixels + offsets.px, px.data.get(), faceSize);
        memcpy(pixels + offsets.nx, nx.data.get(), faceSize);
        memcpy(pixels + offsets.py, py.data.get(), faceSize);
        memcpy(pixels + offsets.ny, ny.data.get(), faceSize);
        memcpy(pixels + offsets.pz, pz.data.get(), faceSize);
        memcpy(pixels + offsets.nz, nz.data.get(), faceSize);
        px.data.reset();
        nx.data.reset();
        py.data.reset();
        ny.data.reset();
        pz.data.reset();
        nz.data.reset();
        skybox->setImage(engine, 0, std::move(buffer), offsets);
    }
    result.skybox = Skybox::Builder().environment(skybox).build(engine);

    return result;
}

}  // namespace filaweb
