/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_GLTFVIEWER_H
#define TNT_GLTFVIEWER_H

#include <string>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <viewer/ViewerGui.h>

#include <utils/EntityManager.h>

class GLTFViewer {
public:
    void setFilename(std::string fileName);

    void runApp(std::function<void(filament::Engine*, filament::View*, filament::Scene*,
                    filament::Renderer*)>
                    postRender);

private:
    void setup(filament::Engine* engine, filament::View* view, filament::Scene* scene);
    void cleanup(filament::Engine* engine, filament::View* view, filament::Scene* scene);

    void animate(filament::Engine* engine, filament::View* view, double now);

    void loadAsset(utils::Path filename);
    void loadResources(utils::Path filename);

    enum MaterialSource {
        JITSHADER,
        UBERSHADER,
    };
    struct App {
        filament::Engine* engine;
        filament::viewer::ViewerGui* viewer;
        Config config;
        filament::gltfio::AssetLoader* loader;
        filament::gltfio::FilamentAsset* asset = nullptr;
        utils::NameComponentManager* names;
        filament::gltfio::MaterialProvider* materials;
        MaterialSource materialSource = UBERSHADER;
        filament::gltfio::ResourceLoader* resourceLoader = nullptr;
        filament::gltfio::TextureProvider* stbDecoder = nullptr;
        filament::gltfio::TextureProvider* ktxDecoder = nullptr;
        filament::gltfio::FilamentInstance* instance;
        bool enableMSAA = false;
    };
    App mApp;

    std::string mFilename;
};

#endif // TNT_GLTFVIEWER_H
