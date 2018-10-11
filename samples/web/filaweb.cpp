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

static Asset getKtxTexture(const Asset& rawfile) {
    Asset result = {};
    result.texture.reset(new KtxBundle(rawfile.rawData.get(), rawfile.rawSize));
    return result;
}

Asset getTexture(const char* name) {
    Asset rawfile = getRawFile(name);
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

static Texture* createCubemapTexture(filament::Engine& engine, image::KtxBundle* ktxbundle) {
    auto info = ktxbundle->getInfo();
    uint32_t nmips = ktxbundle->getNumMipLevels();
    auto builder = Texture::Builder()
            .width(info.pixelWidth)
            .height(info.pixelHeight)
            .levels(nmips)
            .rgbm(true)
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP);

    // Compressed textures in KTX always have a glFormat of 0.
    if (info.glFormat == 0) {
        builder.format(filaweb::toTextureFormat(info.glInternalFormat));
    } else {
        builder.format(Texture::InternalFormat::RGBA8);
    }
    Texture* texture = builder.build(engine);
    size_t size = info.pixelWidth;
    uint32_t i = 0;
    for (uint32_t mip = 0; mip < nmips; ++mip, size >>= 1) {

        uint8_t* ktxPixels;
        uint32_t faceSize;
        ktxbundle->getBlob({mip}, &ktxPixels, &faceSize);

        Texture::FaceOffsets offsets;
        offsets.px = faceSize * 0;
        offsets.nx = faceSize * 1;
        offsets.py = faceSize * 2;
        offsets.ny = faceSize * 3;
        offsets.pz = faceSize * 4;
        offsets.nz = faceSize * 5;

        // If this is a compressed texture, use a special PixelBufferDescriptor constructor.
        Texture::PixelBufferDescriptor* buffer;
        if (info.glFormat == 0) {
            auto datatype = filaweb::toPixelDataType(info.glInternalFormat);
            buffer = new Texture::PixelBufferDescriptor(
                    malloc(faceSize * 6), faceSize * 6,
                    datatype, faceSize,
                    [](void* buffer, size_t size, void* user) {
                        free(buffer);
                    }, /* user = */ nullptr);
        } else {
            buffer = new Texture::PixelBufferDescriptor(
                    malloc(faceSize * 6), faceSize * 6,
                    Texture::Format::RGBM, Texture::Type::UBYTE,
                    [](void* buffer, size_t size, void* user) {
                        free(buffer);
                    }, /* user = */ nullptr);
        }

        // Copy 6 faces from the KTX bundle into the Filament pixel buffer.
        uint8_t* pbdPixels = static_cast<uint8_t*>(buffer->buffer);
        memcpy(pbdPixels, ktxPixels, faceSize * 6);
        texture->setImage(engine, mip, std::move(*buffer), offsets);
        delete buffer;
    }
    return texture;
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
    Texture* texture = createCubemapTexture(engine, asset.envIBL->texture.get());
    asset.envIBL->texture.reset();

    result.indirectLight = IndirectLight::Builder()
        .reflections(texture)
        .irradiance(3, result.bands)
        .intensity(30000.0f)
        .build(engine);

    // Copy a single miplevel for the blurry skybox
    Texture* skybox = createCubemapTexture(engine, asset.envSky->texture.get());
    asset.envSky->texture.reset();
    result.skybox = Skybox::Builder().environment(skybox).build(engine);
    return result;
}

template<typename T>
T toFilamentEnum(uint32_t format) {
    switch (format) {
        case KtxBundle::RGB_S3TC_DXT1: return T::DXT1_RGB;
        case KtxBundle::RGBA_S3TC_DXT1: return T::DXT1_RGBA;
        case KtxBundle::RGBA_S3TC_DXT3: return T::DXT3_RGBA;
        case KtxBundle::RGBA_S3TC_DXT5: return T::DXT5_RGBA;
        case KtxBundle::RGBA_ASTC_4x4: return T::RGBA_ASTC_4x4;
        case KtxBundle::RGBA_ASTC_5x4: return T::RGBA_ASTC_5x4;
        case KtxBundle::RGBA_ASTC_5x5: return T::RGBA_ASTC_5x5;
        case KtxBundle::RGBA_ASTC_6x5: return T::RGBA_ASTC_6x5;
        case KtxBundle::RGBA_ASTC_6x6: return T::RGBA_ASTC_6x6;
        case KtxBundle::RGBA_ASTC_8x5: return T::RGBA_ASTC_8x5;
        case KtxBundle::RGBA_ASTC_8x6: return T::RGBA_ASTC_8x6;
        case KtxBundle::RGBA_ASTC_8x8: return T::RGBA_ASTC_8x8;
        case KtxBundle::RGBA_ASTC_10x5: return T::RGBA_ASTC_10x5;
        case KtxBundle::RGBA_ASTC_10x6: return T::RGBA_ASTC_10x6;
        case KtxBundle::RGBA_ASTC_10x8: return T::RGBA_ASTC_10x8;
        case KtxBundle::RGBA_ASTC_10x10: return T::RGBA_ASTC_10x10;
        case KtxBundle::RGBA_ASTC_12x10: return T::RGBA_ASTC_12x10;
        case KtxBundle::RGBA_ASTC_12x12: return T::RGBA_ASTC_12x12;
        case KtxBundle::SRGB8_ALPHA8_ASTC_4x4: return T::SRGB8_ALPHA8_ASTC_4x4;
        case KtxBundle::SRGB8_ALPHA8_ASTC_5x4: return T::SRGB8_ALPHA8_ASTC_5x4;
        case KtxBundle::SRGB8_ALPHA8_ASTC_5x5: return T::SRGB8_ALPHA8_ASTC_5x5;
        case KtxBundle::SRGB8_ALPHA8_ASTC_6x5: return T::SRGB8_ALPHA8_ASTC_6x5;
        case KtxBundle::SRGB8_ALPHA8_ASTC_6x6: return T::SRGB8_ALPHA8_ASTC_6x6;
        case KtxBundle::SRGB8_ALPHA8_ASTC_8x5: return T::SRGB8_ALPHA8_ASTC_8x5;
        case KtxBundle::SRGB8_ALPHA8_ASTC_8x6: return T::SRGB8_ALPHA8_ASTC_8x6;
        case KtxBundle::SRGB8_ALPHA8_ASTC_8x8: return T::SRGB8_ALPHA8_ASTC_8x8;
        case KtxBundle::SRGB8_ALPHA8_ASTC_10x5: return T::SRGB8_ALPHA8_ASTC_10x5;
        case KtxBundle::SRGB8_ALPHA8_ASTC_10x6: return T::SRGB8_ALPHA8_ASTC_10x6;
        case KtxBundle::SRGB8_ALPHA8_ASTC_10x8: return T::SRGB8_ALPHA8_ASTC_10x8;
        case KtxBundle::SRGB8_ALPHA8_ASTC_10x10: return T::SRGB8_ALPHA8_ASTC_10x10;
        case KtxBundle::SRGB8_ALPHA8_ASTC_12x10: return T::SRGB8_ALPHA8_ASTC_12x10;
        case KtxBundle::SRGB8_ALPHA8_ASTC_12x12: return T::SRGB8_ALPHA8_ASTC_12x12;
        case KtxBundle::R11_EAC: return T::EAC_R11;
        case KtxBundle::SIGNED_R11_EAC: return T::EAC_R11_SIGNED;
        case KtxBundle::RG11_EAC: return T::EAC_RG11;
        case KtxBundle::SIGNED_RG11_EAC: return T::EAC_RG11_SIGNED;
        case KtxBundle::RGB8_ETC2: return T::ETC2_RGB8;
        case KtxBundle::SRGB8_ETC2: return T::ETC2_SRGB8;
        case KtxBundle::RGB8_ALPHA1_ETC2: return T::ETC2_RGB8_A1;
        case KtxBundle::SRGB8_ALPHA1_ETC: return T::ETC2_SRGB8_A1;
        case KtxBundle::RGBA8_ETC2_EAC: return T::ETC2_EAC_RGBA8;
        case KtxBundle::SRGB8_ALPHA8_ETC2_EAC: return T::ETC2_EAC_SRGBA8;
    }
    return (T) 0xffff;
}

filament::driver::CompressedPixelDataType toPixelDataType(uint32_t format) {
    return toFilamentEnum<filament::driver::CompressedPixelDataType>(format);
}

filament::driver::TextureFormat toTextureFormat(uint32_t format) {
    return toFilamentEnum<filament::driver::TextureFormat>(format);
}

}  // namespace filaweb
