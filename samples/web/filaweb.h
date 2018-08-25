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

#ifndef TNT_FILAMENT_SAMPLES_FILAWEB_H
#define TNT_FILAMENT_SAMPLES_FILAWEB_H

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Viewport.h>

#include <chrono>
#include <functional>
#include <memory>

#include <emscripten.h>

namespace filaweb {

struct Asset {
    std::unique_ptr<uint8_t[]> data;
    uint32_t nbytes;
    uint32_t width;
    uint32_t height;
};

static Asset getTexture(const char* name);
static Asset getRawFile(const char* name);

class Application {
public:
    using Engine = filament::Engine;
    using View = filament::View;
    using Scene = filament::Scene;
    using SetupCallback = std::function<void(Engine*, View*, Scene*)>;
    using ImGuiCallback = std::function<void(Engine*, View*)>;
    using AnimCallback = std::function<void(Engine*, View*, double now)>;

    static Application* get() {
        static Application app;
        return &app;
    }

    void run(SetupCallback setup, ImGuiCallback imgui, AnimCallback animation) {
        mAnimation = animation;
        mEngine = Engine::create(Engine::Backend::OPENGL);
        mSwapChain = mEngine->createSwapChain(nullptr);
        mScene = mEngine->createScene();
        mRenderer = mEngine->createRenderer();
        mCamera = mEngine->createCamera();
        mView = mEngine->createView();
        mView->setScene(mScene);
        mView->setCamera(mCamera);
        setup(mEngine, mView, mScene);
    }

    void resize(uint32_t width, uint32_t height) {
        mView->setViewport({0, 0, width, height});
    }

    void render() {
        auto milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
        mAnimation(mEngine, mView, milliseconds_since_epoch / 1000.0);
        if (mRenderer->beginFrame(mSwapChain)) {
            mRenderer->render(mView);
            mRenderer->endFrame();
        }
        mEngine->execute();
    }

private:
    Application() { }
    Engine* mEngine = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
    filament::Renderer* mRenderer = nullptr;
    filament::Camera* mCamera = nullptr;
    filament::SwapChain* mSwapChain = nullptr;
    AnimCallback mAnimation;
};

extern "C" void render() {
    Application::get()->render();
}

extern "C" void resize(uint32_t width, uint32_t height) {
    Application::get()->resize(width, height);
}

static Asset getRawFile(const char* name) {
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
        var data = $0 >> 2;
        var name = UTF8ToString($1);
        HEAP32.set(assets[name].data, data);
        assets[name].data = null;
    }, data, name);
    printf("%s: %d bytes\n", name, nbytes);
    return {
        .data = decltype(Asset::data)(data),
        .nbytes = nbytes
    };
}

static Asset getTexture(const char* name) {
    // Obtain image dimensions from JavaScript.
    uint32_t dims[2];
    EM_ASM({
        var dims = $0 >> 2;
        var name = UTF8ToString($1);
        HEAP32[dims] = assets[name].width;
        HEAP32[dims+1] = assets[name].height;
    }, dims, name);
    const uint8_t nbytes = dims[0] * dims[1] * 4;

    // Move the data from JavaScript.
    uint8_t* texels = new uint8_t[nbytes];
    EM_ASM({
        var texels = $0 >> 2;
        var name = UTF8ToString($1);
        HEAP32.set(assets[name].data, texels);
        assets[name].data = null;
    }, texels, name);
    printf("%s: %d x %d\n", name, dims[0], dims[1]);
    return {
        .data = decltype(Asset::data)(texels),
        .nbytes = nbytes,
        .width = dims[0],
        .height = dims[1]
    };
}

}  // namespace filaweb

#endif // TNT_FILAMENT_SAMPLES_FILAWEB_H
