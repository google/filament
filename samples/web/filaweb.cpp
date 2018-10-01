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

#include <utils/Path.h>

#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include <stb_image.h>

#include <emscripten.h>

#include <image/KtxBundle.h>

#include <string>
#include <sstream>

using namespace filament;
using namespace image;
using namespace std;
using namespace utils;

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

void Application::run(SetupCallback setup, AnimCallback animation, ImGuiCallback imgui) {
    mAnimation = animation;
    mGuiCallback = imgui;
    mEngine = Engine::create(Engine::Backend::OPENGL);
    mSwapChain = mEngine->createSwapChain(nullptr);
    mScene = mEngine->createScene();
    mRenderer = mEngine->createRenderer();
    mView = mEngine->createView();
    mView->setScene(mScene);
    mGuiCam = mEngine->createCamera();
    mGuiView = mEngine->createView();
    mGuiView->setClearTargets(false, false, false);
    mGuiView->setRenderTarget(View::TargetBufferFlags::DEPTH_AND_STENCIL);
    mGuiView->setPostProcessingEnabled(false);
    mGuiView->setShadowsEnabled(false);
    mGuiView->setCamera(mGuiCam);
    mGuiHelper = new filagui::ImGuiHelper(mEngine, mGuiView, "");
    setup(mEngine, mView, mScene);

    // File I/O in WebAssembly does not exist, so tell ImGui to not bother with the ini file.
    ImGui::GetIO().IniFilename = nullptr;
}

void Application::resize(uint32_t width, uint32_t height, double pixelRatio) {
    mPixelRatio = pixelRatio;
    mView->setViewport({0, 0, width, height});
    mGuiView->setViewport({0, 0, width, height});
    mManipulator.setViewport(width, height);
    mGuiCam->setProjection(filament::Camera::Projection::ORTHO,
            0.0, width / pixelRatio,
            height / pixelRatio, 0.0,
            0.0, 1.0);
    mGuiHelper->setDisplaySize(width / pixelRatio, height / pixelRatio, pixelRatio, pixelRatio);
}

void Application::mouse(uint32_t x, uint32_t y, int32_t wx, int32_t wy, uint16_t buttons) {
    // First, pass the current pointer state to ImGui.
    auto& io = ImGui::GetIO();
    if (wx > 0) io.MouseWheelH += 1;
    if (wx < 0) io.MouseWheelH -= 1;
    if (wy > 0) io.MouseWheel += 1;
    if (wy < 0) io.MouseWheel -= 1;
    io.MousePos.x = x;
    io.MousePos.y = y;
    io.MouseDown[0] = buttons & 1;
    io.MouseDown[1] = buttons & 2;
    io.MouseDown[2] = buttons & 4;

    // Negate Y before pushing values to the manipulator.
    y = -y;
    wy = -wy;

    // Pass values to the camera manipulator to enable dolly and rotate.
    // We do not call call track() because two-button mouse usage is less useful on web.
    using namespace math;
    static double2 previousMousePosition = double2(x, y);
    static uint16_t previousMouseButtons = buttons;
    double2 delta = double2(x, y) - previousMousePosition;
    previousMousePosition = double2(x, y);
    mManipulator.dolly(wy);
    if (!io.WantCaptureMouse && buttons == 1 && buttons == previousMouseButtons) {
        mManipulator.rotate(delta);
    }
    previousMouseButtons = buttons;
}

void Application::render() {
    using namespace std::chrono;

    mManipulator.updateCameraTransform();

    auto milliseconds_since_epoch = system_clock::now().time_since_epoch() / milliseconds(1);
    mAnimation(mEngine, mView, milliseconds_since_epoch / 1000.0);

    double now = milliseconds_since_epoch / 1000.0;
    static double previous = now;
    mGuiHelper->render(now - previous, mGuiCallback);
    previous = now;

    if (mRenderer->beginFrame(mSwapChain)) {
        mRenderer->render(mView);
        mRenderer->render(mGuiView);
        mRenderer->endFrame();
    }
    mEngine->execute();
}

Asset getRawFile(const char* name) {
    // Obtain size and URL from JavaScript.
    Asset result = {};
    EM_ASM({
        var nbytes = $0 >> 2;
        var name = UTF8ToString($1);
        HEAP32[nbytes] = assets[name].data.byteLength;
        stringToUTF8(assets[name].url, $2, $3);
    }, &result.rawSize, name, &result.rawUrl[0], sizeof(result.rawUrl));

    // Move the data from JavaScript.
    uint8_t* data = new uint8_t[result.rawSize];
    EM_ASM({
        var data = $0;
        var name = UTF8ToString($1);
        HEAPU8.set(assets[name].data, data);
        assets[name].data = null;
    }, data, name);
    result.rawData = decltype(Asset::rawData)(data);
    return result;
}

static Asset getPngTexture(const Asset& rawfile) {
    int width, height, ncomp;
    stbi_info_from_memory(rawfile.rawData.get(), rawfile.rawSize, &width, &height, &ncomp);
    const uint32_t nbytes = width * height * 4;
    stbi_uc* decoded = stbi_load_from_memory(rawfile.rawData.get(), rawfile.rawSize, &width,
            &height, &ncomp, 4);
    Asset result = {};
    result.texture.reset(new KtxBundle(1, 1, false));
    result.texture->setBlob({}, decoded, nbytes);
    stbi_image_free(decoded);
    return result;

}

static Asset getKtxTexture(const Asset& rawfile) {
    Asset result = {};
    result.texture.reset(new KtxBundle(rawfile.rawData.get(), rawfile.rawSize));
    return result;
}

Asset getTexture(const char* name) {
    Asset rawfile = getRawFile(name);
    string extension = Path(rawfile.rawUrl).getExtension();
    fflush(stdout);
    if (extension == "png" || extension == "rgbm") {
        return getPngTexture(rawfile);
    }
    // Do not check for KTX here, sometimes we use an alternate file extension.
    return getKtxTexture(rawfile);
}

Asset getCubemap(const char* name) {
    string prefix = string(name) + "/";

    // Deserialize the KtxBundle for the IBL.
    Asset* envFaces = new Asset;
    string key = prefix + "ibl";
    *envFaces = getKtxTexture(getRawFile(key.c_str()));

    // Ditto but for the blurry sky.
    Asset* skyFaces = new Asset;
    key = prefix + "skybox";
    *skyFaces = getKtxTexture(getRawFile(key.c_str()));

    Asset result = {};
    result.envIBL.reset(envFaces);
    result.envSky.reset(skyFaces);
    return result;
}

SkyLight getSkyLight(Engine& engine, const char* name) {
    SkyLight result;
    using size_t = std::size_t;

    // Pull the data out of JavaScript.
    static auto asset = filaweb::getCubemap(name);

    // Parse the coefficients.
    const char* shtext = asset.envIBL->texture->getMetadata("sh");
    if (!shtext) {
        cerr << name << " is missing spherical harmonics coefficients." << endl;
        abort();
    }
    std::istringstream shReader(shtext);
    shReader >> std::skipws;
    for (size_t i = 0; i < 9; i++) {
        shReader >> result.bands[i].r >> result.bands[i].g >> result.bands[i].b;
    }

    // Copy over the miplevels for the indirect light.
    auto info = asset.envIBL->texture->getInfo();
    uint32_t nmips = asset.envIBL->texture->getNumMipLevels();
    Texture* texture = Texture::Builder()
        .width(info.pixelWidth)
        .height(info.pixelHeight)
        .levels(nmips)
        .format(Texture::InternalFormat::RGBM)
        .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
        .build(engine);
    size_t size = info.pixelWidth;
    uint32_t i = 0;
    for (uint32_t mip = 0; mip < nmips; ++mip, size >>= 1) {
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
        uint8_t* pbdPixels = static_cast<uint8_t*>(buffer.buffer);
        uint8_t* ktxPixels;
        uint32_t ktxSize;
        asset.envIBL->texture->getBlob({mip}, &ktxPixels, &ktxSize);
        memcpy(pbdPixels, ktxPixels, faceSize * 6);
        texture->setImage(engine, mip, std::move(buffer), offsets);
    }
    asset.envIBL->texture.reset();

    result.indirectLight = IndirectLight::Builder()
        .reflections(texture)
        .irradiance(3, result.bands)
        .intensity(30000.0f)
        .build(engine);

    // Copy a single miplevel for the blurry skybox
    info = asset.envSky->texture->getInfo();
    size = info.pixelWidth;
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
        uint8_t* pbdPixels = static_cast<uint8_t*>(buffer.buffer);
        uint8_t* ktxPixels;
        uint32_t ktxSize;
        asset.envSky->texture->getBlob({}, &ktxPixels, &ktxSize);
        memcpy(pbdPixels, ktxPixels, faceSize * 6);
        skybox->setImage(engine, 0, std::move(buffer), offsets);
    }
    asset.envSky->texture.reset();
    result.skybox = Skybox::Builder().environment(skybox).build(engine);

    return result;
}

filament::driver::CompressedPixelDataType toPixelDataType(uint32_t format) {
    using DstFormat = filament::driver::CompressedPixelDataType;
    switch (format) {
        case KtxBundle::RGB_S3TC_DXT1: return DstFormat::DXT1_RGB;
        case KtxBundle::RGBA_S3TC_DXT1: return DstFormat::DXT1_RGBA;
        case KtxBundle::RGBA_S3TC_DXT3: return DstFormat::DXT3_RGBA;
        case KtxBundle::RGBA_S3TC_DXT5: return DstFormat::DXT5_RGBA;
    }
    return (filament::driver::CompressedPixelDataType) 0xffff;
}

filament::driver::TextureFormat toTextureFormat(uint32_t format) {
    using DstFormat = filament::driver::TextureFormat;
    switch (format) {
        case KtxBundle::RGB_S3TC_DXT1: return DstFormat::DXT1_RGB;
        case KtxBundle::RGBA_S3TC_DXT1: return DstFormat::DXT1_RGBA;
        case KtxBundle::RGBA_S3TC_DXT3: return DstFormat::DXT3_RGBA;
        case KtxBundle::RGBA_S3TC_DXT5: return DstFormat::DXT5_RGBA;
    }
    return (filament::driver::TextureFormat) 0xffff;
}

}  // namespace filaweb
