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

#include <filagui/ImGuiHelper.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <chrono>
#include <functional>
#include <memory>

#include <imgui.h>

#include <emscripten.h>

namespace filaweb {

// Filaweb defines three kinds of assets: raw files, single-mip textures, and cubemaps.
// For simplicity all three kinds are represented with a single struct.
struct Asset {
    std::unique_ptr<uint8_t[]> data;
    uint32_t nbytes;
    uint32_t width;
    uint32_t height;
    uint32_t envMipCount;
    std::unique_ptr<Asset> envShCoeffs;
    std::unique_ptr<Asset[]> envFaces;
    std::unique_ptr<Asset[]> skyFaces;
};

Asset getRawFile(const char* name);
Asset getTexture(const char* name);
Asset getCubemap(const char* name);

struct SkyLight {
    math::float3 bands[9];
    filament::IndirectLight const* indirectLight;
    filament::Skybox* skybox;
};

SkyLight getSkyLight(filament::Engine& engine, const char* name);

static const auto NoopCallback = [](filament::Engine*, filament::View*) {};

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

    void run(SetupCallback setup, AnimCallback animation, ImGuiCallback imgui = NoopCallback) {
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

    void resize(uint32_t width, uint32_t height, double pixelRatio) {
        mPixelRatio = pixelRatio;
        mView->setViewport({0, 0, width, height});
        mGuiView->setViewport({0, 0, width, height});
        mGuiCam->setProjection(filament::Camera::Projection::ORTHO,
            0.0, width / pixelRatio,
            height / pixelRatio, 0.0,
            0.0, 1.0);
        mGuiHelper->setDisplaySize(width / pixelRatio, height / pixelRatio, pixelRatio, pixelRatio);
    }

    void mouse(uint32_t x, uint32_t y, int32_t wx, int32_t wy, uint16_t buttons) {
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
    }

    void render() {
        auto milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
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

private:
    Application() { }
    filagui::ImGuiHelper* mGuiHelper = nullptr;
    Engine* mEngine = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
    View* mGuiView = nullptr;
    filament::Camera* mGuiCam = nullptr;
    filament::Renderer* mRenderer = nullptr;
    filament::SwapChain* mSwapChain = nullptr;
    AnimCallback mAnimation;
    ImGuiCallback mGuiCallback;
    double mPixelRatio = 1.0;
};

}  // namespace filaweb

#endif // TNT_FILAMENT_SAMPLES_FILAWEB_H
