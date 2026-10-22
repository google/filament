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
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include "FullScreenTriangle.h"

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;

class FilamentApp {
public:

    FilamentApp(void* nativeLayer, uint32_t width, uint32_t height);
    ~FilamentApp();
    FilamentApp(const FilamentApp&) = delete;
    FilamentApp& operator=(const FilamentApp&) = delete;

    struct FilamentArFrame {
        void* cameraImage;              // A CVPixelBufferRef given to us by ARKit
        mat3f cameraTextureTransform;   // Transform to be applied to the camera Image
        mat4f view;                     // Camera's current view matrix
        mat4 projection;                // Camera's current projection matrix
    };

    void render(const FilamentArFrame& frame);
    void setObjectTransform(const mat4f& transform);

    struct FilamentArPlaneGeometry {
        mat4f transform;
        float4* vertices;
        uint16_t* indices;
        size_t vertexCount;
        size_t indexCount;
    };

    void updatePlaneGeometry(const FilamentArPlaneGeometry& geometry);

private:

    void setupFilament();
    void setupIbl();
    void setupMaterial();
    void setupMesh();
    void setupView();
    void setupCameraFeedTriangle();

    void* nativeLayer = nullptr;
    uint32_t width, height;

    Engine* engine = nullptr;
    Renderer* renderer = nullptr;
    Scene* scene = nullptr;
    View* view = nullptr;
    Camera* camera = nullptr;
    SwapChain* swapChain = nullptr;

    quatf meshRotation = quatf(1.0f, 0.0f, 0.0f, 0.0f);
    mat4f meshTransform;

    // App-specific
    struct {
        Material* mat;
        MaterialInstance* materialInstance;
        IndirectLight* indirectLight = nullptr;
        Texture* iblTexture = nullptr;
        Entity renderable;
        Entity sun;
        FullScreenTriangle* cameraFeedTriangle = nullptr;

        VertexBuffer* planeVertices = nullptr;
        IndexBuffer* planeIndices = nullptr;
        Entity planeGeometry;
        Material* shadowPlane = nullptr;
    } app;

};

#endif /* FilamentApp_h */
