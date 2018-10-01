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

#include "../app/CameraManipulator.h"

#include <image/KtxBundle.h>

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

#include <functional>

namespace filaweb {

// Filaweb defines three kinds of assets: textures, environments, and raw files.
// For simplicity all three kinds are represented with a single struct.
struct Asset {
    std::unique_ptr<image::KtxBundle> texture;
    std::unique_ptr<Asset> envIBL;
    std::unique_ptr<Asset> envSky;
    std::unique_ptr<uint8_t[]> rawData;
    uint32_t rawSize;
    char rawUrl[256];
};

Asset getRawFile(const char* name);
Asset getTexture(const char* name);
Asset getCubemap(const char* name);

struct SkyLight {
    math::float3 bands[9];
    filament::IndirectLight* indirectLight;
    filament::Skybox* skybox;
};

SkyLight getSkyLight(filament::Engine& engine, const char* name);

filament::driver::CompressedPixelDataType toPixelDataType(uint32_t format);
filament::driver::TextureFormat toTextureFormat(uint32_t format);

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

    void run(SetupCallback setup, AnimCallback animation, ImGuiCallback imgui = NoopCallback);
    void resize(uint32_t width, uint32_t height, double pixelRatio);
    void mouse(uint32_t x, uint32_t y, int32_t wx, int32_t wy, uint16_t buttons);
    void render();

    CameraManipulator& getManipulator() { return mManipulator; }

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
    CameraManipulator mManipulator;
    AnimCallback mAnimation;
    ImGuiCallback mGuiCallback;
    double mPixelRatio = 1.0;
};

}  // namespace filaweb

#endif // TNT_FILAMENT_SAMPLES_FILAWEB_H
