/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <SDL.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>
#include <filamentapp/NativeWindowHelper.h>
#include <filameshio/MeshReader.h>
#include <math/mat4.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>

#include <functional>
#include <iostream>
#include <vector>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;

namespace {
    static constexpr Engine::Backend kBackend = Engine::Backend::OPENGL;
    static constexpr int kWidth = 640;
    static constexpr int kHeight = 480;
    static constexpr double kFieldOfViewDeg = 60.0;
    static constexpr double kNearPlane = 0.1;
    static constexpr double kFarPlane = 50.0;
    static constexpr const char* kIBLFolder = "assets/ibl/lightroom_14b";
    static constexpr double kRotationDegPerSec = 36.0;
    static constexpr math::float3 kCameraCenter = {0.0f, 0.0f, 0.0f};
    static constexpr math::float3 kCameraUp = {0.0f, 1.0f, 0.0f};
    static constexpr float kCameraDist = 3.0f;

    struct Window {
        std::function<void(Window&, double)> onNewFrame;

        SDL_Window* sdl_window = nullptr;
        Renderer* renderer = nullptr;
        SwapChain* swapChain = nullptr;
        utils::Entity cameraEntity;
        Camera* camera = nullptr;
        View* view = nullptr;
        Scene* scene = nullptr;
        IBL* ibl = nullptr;
        Material* material = nullptr;
        MaterialInstance* materialInstance = nullptr;
        filamesh::MeshReader::Mesh mesh;

        bool needsDraw = true;
        double time = 0.0;
        double lastDrawTime = 0.0;
    };
}

void setup_window(Window& w, Engine* engine);
void destroy_window(Window& w, Engine* engine);
void resize_window(Window& w, Engine* engine);

void setup_static_scene(Window& w, Engine* engine);
void setup_animating_scene(Window& w, Engine* engine);
void animation_new_frame(Window& w, double dt);
IBL* load_IBL(const utils::Path& iblDirectory, Engine* engine);

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char *argv[]) {
    // ---- initialize ----
    FILAMENT_CHECK_POSTCONDITION(SDL_Init(SDL_INIT_EVENTS) == 0) << "SDL_Init Failure";

    std::vector<Window> windows = { Window(), Window() };
    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
                           | SDL_WINDOW_RESIZABLE;
    int n = 1;
    int x = 50, y = 50;
    for (auto &w : windows) {
        auto title = std::string("Filament - Window ") + std::to_string(n);
        w.sdl_window = SDL_CreateWindow(title.c_str(), x, y, kWidth, kHeight,
                                        windowFlags);
        x += 50;
        y += 50;
        n += 1;
    }

    // Create SDL windows first, so that the Engine's context is current
    // if we are single-threaded. But we can't create the Filament objects
    // until after we have created the engine.
    auto engine = Engine::create(kBackend);

    for (auto &w : windows) {
        setup_window(w, engine);
    }
    setup_animating_scene(windows[0], engine);
    setup_static_scene(windows[1], engine);

    // ---- event loop ----
    size_t nClosed = 0;
    SDL_Event event;
    Uint64 lastTime = 0;
    const Uint64 kCounterFrequency = SDL_GetPerformanceFrequency();

    while (nClosed < windows.size()) {
        if (!UTILS_HAS_THREADING) {
            engine->execute();
        }

        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    nClosed = windows.size();
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            for (auto &w : windows) {
                                if (event.window.windowID == SDL_GetWindowID(w.sdl_window)) {
                                    resize_window(w, engine);
                                    break;
                                }
                            }
                            break;
                        case SDL_WINDOWEVENT_CLOSE:
                            for (auto &w : windows) {
                                if (event.window.windowID == SDL_GetWindowID(w.sdl_window)) {
                                    SDL_HideWindow(w.sdl_window);
                                    break;
                                }
                            }
                            nClosed++;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        Uint64 now = SDL_GetPerformanceCounter();
        const double dt = lastTime > 0 ? (double(now - lastTime) / kCounterFrequency) : (1.0 / 60.0);
        lastTime = now;
        for (auto &w : windows) {
            w.time += dt;
            if (w.onNewFrame) {
                w.onNewFrame(w, dt);
            }
        }

        for (auto &w : windows) {
            if (!w.needsDraw) { continue; }

            if (w.renderer->beginFrame(w.swapChain)) {
                w.renderer->render(w.view);
                w.renderer->endFrame();
            }
            w.needsDraw = false;
            w.lastDrawTime = w.time;
        }

        SDL_Delay(16);
    }

    // ---- cleanup ----
    for (auto &w : windows) {
        destroy_window(w, engine);
    }

    Engine::destroy(&engine);

    SDL_Quit();
    return 0;
}

void setup_window(Window& w, Engine* engine) {
    w.renderer = engine->createRenderer();

    void* nativeWindow = ::getNativeWindow(w.sdl_window);
    void* nativeSwapChain = nativeWindow;
#if defined(__APPLE__)
    void* metalLayer = nullptr;

#if defined(FILAMENT_SUPPORTS_WEBGPU)
    if (kBackend == filament::Engine::Backend::METAL || kBackend == filament::Engine::Backend::VULKAN
        || kBackend == filament::Engine::Backend::WEBGPU) {
#else
    if (kBackend == filament::Engine::Backend::METAL || kBackend == filament::Engine::Backend::VULKAN) {
#endif
        metalLayer = setUpMetalLayer(nativeWindow);
        // The swap chain on both native Metal and MoltenVK is a CAMetalLayer.
        nativeSwapChain = metalLayer;
    }
#endif
    w.swapChain = engine->createSwapChain(nativeSwapChain);

    utils::EntityManager& em = utils::EntityManager::get();
    em.create(1, &w.cameraEntity);
    w.camera = engine->createCamera(w.cameraEntity);

    w.view = engine->createView();
    w.view->setCamera(w.camera);

    w.scene = engine->createScene();
    w.view->setScene(w.scene);

    resize_window(w, engine);
}

void destroy_window(Window& w, Engine* engine) {
    delete w.ibl;

    engine->destroy(w.mesh.renderable);
    engine->destroy(w.materialInstance);
    engine->destroy(w.material);
    w.view->setScene(nullptr);
    engine->destroy(w.scene);
    engine->destroy(w.view);
    engine->destroyCameraComponent(w.cameraEntity);
    engine->destroy(w.cameraEntity);
    engine->destroy(w.swapChain);
    engine->destroy(w.renderer);

    SDL_DestroyWindow(w.sdl_window);
}

void resize_window(Window& w, Engine* engine) {
#if defined(__APPLE__)
    void* nativeWindow = ::getNativeWindow(w.sdl_window);
    if (kBackend == filament::Engine::Backend::METAL) {
        resizeMetalLayer(nativeWindow);
    }
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
    if (kBackend == filament::Engine::Backend::VULKAN) {
        resizeMetalLayer(nativeWindow);
    }
#endif
#endif

    int width, height;
    SDL_GL_GetDrawableSize(w.sdl_window, &width, &height);
    w.view->setViewport({0, 0, uint32_t(width), uint32_t(height)});

    w.camera->setProjection(kFieldOfViewDeg, double(width) / double(height),
                            kNearPlane, kFarPlane);

    w.needsDraw = true;
}

void setup_static_scene(Window& w, Engine* engine) {
    auto iblDir = FilamentApp::getRootAssetsPath() + kIBLFolder;
    w.ibl = load_IBL(iblDir, engine);
    if (w.ibl) {
        w.ibl->getIndirectLight()->setIntensity(10000);
        w.scene->setIndirectLight(w.ibl->getIndirectLight());
        w.scene->setSkybox(w.ibl->getSkybox());
    }

    w.material = Material::Builder().package(RESOURCES_SANDBOXLIT_DATA,
                                             RESOURCES_SANDBOXLIT_SIZE)
                                    .build(*engine);
    w.materialInstance = w.material->createInstance();
    w.materialInstance->setParameter("baseColor", RgbType::sRGB,
                                     {0.50f, 0.90f, 0.80f});
    w.materialInstance->setParameter("roughness", 0.90f);
    w.materialInstance->setParameter("metallic", 0.01f);
    w.materialInstance->setParameter("reflectance", 0.00f);
    w.materialInstance->setParameter("sheenColor", 0.00f);
    w.materialInstance->setParameter("clearCoat", 1.00f);
    w.materialInstance->setParameter("clearCoatRoughness", 0.00f);
    w.mesh = filamesh::MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, w.materialInstance);
    w.scene->addEntity(w.mesh.renderable);

    int width, height;
    SDL_GL_GetDrawableSize(w.sdl_window, &width, &height);
    w.camera->setProjection(kFieldOfViewDeg, double(width) / double(height),
                            kNearPlane, kFarPlane);
    w.camera->lookAt({0.0f, 0.0f, kCameraDist}, kCameraCenter, kCameraUp);

    w.needsDraw = true;
}

void setup_animating_scene(Window& w, Engine* engine) {
    auto iblDir = FilamentApp::getRootAssetsPath() + kIBLFolder;
    w.ibl = load_IBL(iblDir, engine);
    if (w.ibl) {
        w.ibl->getIndirectLight()->setIntensity(10000);
        w.scene->setIndirectLight(w.ibl->getIndirectLight());
        w.scene->setSkybox(w.ibl->getSkybox());
    }

    w.material = Material::Builder().package(RESOURCES_SANDBOXLIT_DATA,
                                             RESOURCES_SANDBOXLIT_SIZE)
                                    .build(*engine);
    w.materialInstance = w.material->createInstance();
    w.materialInstance->setParameter("baseColor", RgbType::sRGB,
                                     {1.00f, 0.85f, 0.57f});
    w.materialInstance->setParameter("roughness", 0.40f);
    w.materialInstance->setParameter("metallic", 0.99f);
    w.materialInstance->setParameter("reflectance", 0.75f);
    w.materialInstance->setParameter("sheenColor", 0.00f);
    w.materialInstance->setParameter("clearCoat", 0.00f);
    w.materialInstance->setParameter("clearCoatRoughness", 0.00f);
    w.mesh = filamesh::MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, w.materialInstance);
    w.scene->addEntity(w.mesh.renderable);

    int width, height;
    SDL_GL_GetDrawableSize(w.sdl_window, &width, &height);
    w.camera->setProjection(kFieldOfViewDeg, double(width) / double(height),
                            kNearPlane, kFarPlane);
    w.camera->lookAt({0.0f, 0.0f, kCameraDist}, kCameraCenter, kCameraUp);

    w.needsDraw = true;

    w.onNewFrame = animation_new_frame;
}

void animation_new_frame(Window& w, double dt) {
    // Don't animate every frame or the frames queue up and get very laggy.
    if ((w.time - w.lastDrawTime) < 0.040) {
        return;
    }

    double theta = w.time * kRotationDegPerSec * 3.141592653589793 / 180.0;
    math::float3 eye = {kCameraDist * std::sin(theta),
                        0.0f,
                        kCameraDist * std::cos(theta)};
    w.camera->lookAt(eye, kCameraCenter, kCameraUp);
    
    w.needsDraw = true;
}

IBL* load_IBL(const utils::Path& iblDirectory, Engine* engine) {
    utils::Path iblPath(iblDirectory);

    if (!iblPath.exists()) {
        std::cerr << "The specified IBL path does not exist: " << iblPath << std::endl;
        return nullptr;
    }

    if (!iblPath.isDirectory()) {
        std::cerr << "The specified IBL path is not a directory: " << iblPath << std::endl;
        return nullptr;
    }

    IBL* ibl= new IBL(*engine);
    if (!ibl->loadFromDirectory(iblPath)) {
        std::cerr << "Could not load the specified IBL: " << iblPath << std::endl;
        delete ibl;
        return nullptr;
    }

    return ibl;
}
