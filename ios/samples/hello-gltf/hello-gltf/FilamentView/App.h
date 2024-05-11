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

#ifndef App_h
#define App_h

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include "CameraManipulator.h"

#include <utils/EntityManager.h>
#include <utils/Path.h>

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;

namespace filament::gltfio {
    class AssetLoader;
    class MaterialProvider;
    class FilamentAsset;
}

class App {
public:

    App(void* nativeLayer, uint32_t width, uint32_t height, const utils::Path& resourcePath);
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void render();
    void pan(float deltaX, float deltaY);

private:

    void setupFilament();
    void setupIbl();
    void setupMaterial();
    void setupMesh();
    void setupView();

    void* nativeLayer = nullptr;
    uint32_t width, height;
    const utils::Path resourcePath;

    Engine* engine = nullptr;
    Renderer* renderer = nullptr;
    Scene* scene = nullptr;
    View* view = nullptr;
    Camera* camera = nullptr;
    SwapChain* swapChain = nullptr;

    // App-specific
    struct {
        IndirectLight* indirectLight = nullptr;
        Texture* iblTexture = nullptr;
        Texture* skyboxTexture = nullptr;
        Skybox* skybox = nullptr;
        Entity sun;

        filament::gltfio::AssetLoader* assetLoader;
        filament::gltfio::MaterialProvider* materialProvider;
        filament::gltfio::FilamentAsset* asset;
    } app;

    CameraManipulator cameraManipulator;

};

#endif /* App_h */
