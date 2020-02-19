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

#ifndef FilamentApp_h
#define FilamentApp_h

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include "CameraManipulator.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;

class FilamentApp {
public:

    FilamentApp(void* nativeLayer, uint32_t width, uint32_t height)
        : nativeLayer(nativeLayer), width(width), height(height) {}
    ~FilamentApp();

    void initialize();
    void render();
    void pan(float deltaX, float deltaY);

private:

    void updateRotation();

    void* nativeLayer = nullptr;
    uint32_t width, height;

    Engine* engine = nullptr;
    Renderer* renderer = nullptr;
    Scene* scene = nullptr;
    View* filaView = nullptr;
    Camera* camera = nullptr;
    SwapChain* swapChain = nullptr;

    // The amount of rotation to apply to the camera to offset the device's
    // rotation (in radians)
    float deviceRotation = 0.0f;
    float desiredRotation = 0.0f;

    // App-specific
    struct {
        Material* mat;
        MaterialInstance* materialInstance;
        IndirectLight* indirectLight = nullptr;
        Texture* iblTexture = nullptr;
        Texture* skyboxTexture = nullptr;
        Skybox* skybox = nullptr;
        Entity renderable;
        Entity sun;
    } app;

    CameraManipulator cameraManipulator;

};

#endif /* FilamentApp_h */
