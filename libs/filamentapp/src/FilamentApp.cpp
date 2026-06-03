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

#include <filamentapp/FilamentApp.h>

#include "PlatformHelper.h"

#include "KeyInputConversion.h"
#include "display_managers/HtmlDisplayManager.h"
#include "display_managers/SDLDisplayManager.h"

#if defined(WIN32)
#    include <utils/unwindows.h>
#endif

#include <iostream>

#include <imgui.h>

#include <utils/EntityManager.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <filament/Camera.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/SwapChain.h>
#include <filament/View.h>

#include <backend/Platform.h>

#ifndef NDEBUG
#include <filament/DebugRegistry.h>
#endif

#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
#include <backend/platforms/VulkanPlatform.h>
#endif

#include <filagui/ImGuiHelper.h>

#include <filamentapp/Cube.h>
#include <filamentapp/Grid.h>

#include <stb_image.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <thread>
#include <vector>

#ifdef __EXCEPTIONS
#include <exception>
#endif

#include <stdint.h>

#include "generated/resources/filamentapp.h"

using namespace filament;
using namespace filagui;
using namespace filament::math;
using namespace utils;
using namespace filament::app;

namespace {

using namespace filament::backend;
}

FilamentApp& FilamentApp::get() {
    static FilamentApp filamentApp;
    return filamentApp;
}

FilamentApp::FilamentApp() {}

FilamentApp::~FilamentApp() {
    if (mDisplayManager) {
        mDisplayManager->terminate();
        delete mDisplayManager;
    }
}

View* FilamentApp::getGuiView() const noexcept {
    return mImGuiHelper->getView();
}

void FilamentApp::run(const Config& config, SetupCallback setupCallback,
        CleanupCallback cleanupCallback, ImGuiCallback imguiCallback,
        PreRenderCallback preRender, PostRenderCallback postRender,
        size_t width, size_t height) {

    // Note that we need to determine the backend in order to build custom platforms.
    Engine::Backend backend = filament::app::resolveBackend(config.backend);

    backend::Platform* platform = nullptr;
    if (backend == Engine::Backend::VULKAN) {
        platform = mVulkanPlatform =
                filament::app::createVulkanPlatform(config.vulkanGPUHint.c_str());
    } else if (backend == Engine::Backend::WEBGPU) {
        platform = mWebGPUPlatform =
                filament::app::createWebGPUPlatform(config.forcedWebGPUBackend);
    }

    Engine::Config engineConfig = {
#if defined(FILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)
        .stereoscopicType = Engine::StereoscopicType::INSTANCED,
#elif defined(FILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
        .stereoscopicType = Engine::StereoscopicType::MULTIVIEW,
#else
        .stereoscopicType = Engine::StereoscopicType::NONE,
#endif
        .stereoscopicEyeCount = (uint8_t) config.stereoscopicEyeCount,
        .asynchronousMode = config.asynchronousMode,
    };

    // "backend.enable_asynchronous_operation" is forcibly enabled here, inheriting the setting
    // from the Engine, purely to demonstrate the object's asynchronous behavior within
    // Filament. Users should manage this flag at their discretion.
    mEngine = Engine::Builder()
                      .backend(backend)
                      .featureLevel(config.featureLevel)
                      .feature("backend.enable_asynchronous_operation",
                              engineConfig.asynchronousMode != AsynchronousMode::NONE)
                      .platform(platform)
                      .config(&engineConfig)
                      .build();

    assert_invariant(mEngine->getBackend() == backend);

    // By now we have resolved to a specific backend (instead of default).
    config.backend = backend;

    if (config.displayManager == Config::DisplayManager::WEB) {
        mDisplayManager = new HtmlDisplayManager();
    } else {
        mDisplayManager = new SDLDisplayManager();
    }

    if (!mDisplayManager->init(config)) {
        LOG(ERROR) << "Failed to initialize display manager" << utils::io::endl;
        return;
    }

    mWindowTitle = config.title;
    std::unique_ptr<FilamentApp::Window> window(
            new FilamentApp::Window(this, config, config.title, mCameraParams, width, height));

    mDepthMaterial = Material::Builder()
            .package(FILAMENTAPP_DEPTHVISUALIZER_DATA, FILAMENTAPP_DEPTHVISUALIZER_SIZE)
            .build(*mEngine);

    mDepthMI = mDepthMaterial->createInstance();

    mDefaultMaterial = Material::Builder()
            .package(FILAMENTAPP_AIDEFAULTMAT_DATA, FILAMENTAPP_AIDEFAULTMAT_SIZE)
            .build(*mEngine);

    mTransparentMaterial = Material::Builder()
            .package(FILAMENTAPP_TRANSPARENTCOLOR_DATA, FILAMENTAPP_TRANSPARENTCOLOR_SIZE)
            .build(*mEngine);

    std::unique_ptr<Cube> cameraCube{ new Cube(*mEngine, mTransparentMaterial, { 1, 0, 0 }) };
    std::unique_ptr<Grid> cameraGrid{ new Grid(*mEngine, mTransparentMaterial, { 1, 1, 0 }) };

    // we can't cull the light-frustum because it's not applied a rigid transform
    // and currently, filament assumes that for culling
    std::vector<Cube> lightmapCubes;
    lightmapCubes.reserve(4);
    lightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 0, 1, 0 }, false);
    lightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 0, 0, 1 }, false);
    lightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 1, 1, 0 }, false);
    lightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 1, 0, 0 }, false);

    mScene = mEngine->createScene();

    window->mMainView->getView()->setVisibleLayers(0x4, 0x4);
    window->mMainView->getView()->setFroxelVizEnabled(true);

    if (config.splitView) {
        mScene->addEntity(cameraCube->getSolidRenderable());
        mScene->addEntity(cameraCube->getWireFrameRenderable());
        for (auto&& cube : lightmapCubes) {
            mScene->addEntity(cube.getSolidRenderable());
            mScene->addEntity(cube.getWireFrameRenderable());
        }

        window->mDepthView->getView()->setVisibleLayers(0x4, 0x4);
        window->mGodView->getView()->setVisibleLayers(0x6, 0x6);
        window->mOrthoView->getView()->setVisibleLayers(0x6, 0x6);

        // only preserve the color buffer for additional views; depth and stencil can be discarded.
        window->mDepthView->getView()->setShadowingEnabled(false);
        window->mGodView->getView()->setShadowingEnabled(false);
        window->mOrthoView->getView()->setShadowingEnabled(false);
    }

    // froxel debug grid always added (but hidden)
    mScene->addEntity(cameraGrid->getWireFrameRenderable());

    loadDirt(config);
    loadIBL(config);

    for (auto& view : window->mViews) {
        if (view.get() != window->mUiView) {
            view->getView()->setScene(mScene);
        }
    }

    setupCallback(mEngine, window->mMainView->getView(), mScene);

    if (imguiCallback) {
        mImGuiHelper = std::make_unique<ImGuiHelper>(mEngine, window->mUiView->getView(),
                getRootAssetsPath() + "assets/fonts/Roboto-Medium.ttf");
    }

    bool mousePressed[3] = { false };

    int sidebarWidth = mCameraParams.sidebarWidth;
    float cameraFocalLength = mCameraParams.focalLength;
    float cameraNear = mCameraParams.near;
    float cameraFar = mCameraParams.far;
    WindowCameraParams oldCamera = mCameraParams;

#ifdef __EXCEPTIONS
try {
#endif
    while (!mClosed) {
        uint32_t width, height;
        mDisplayManager->getWindowSize(window->mWindow, &width, &height);

        if (oldCamera != mCameraParams) {
            window->configureCamerasForWindow(mCameraParams);
            oldCamera = mCameraParams;
        }

        if (!UTILS_HAS_THREADING) {
            mEngine->execute();
        }

        // Allow the app to animate the scene if desired.
        if (mAnimation) {
            mAnimation(mEngine, window->mMainView->getView(), mDisplayManager->getTime());
        }

        // Loop over fresh events twice: first stash them and let ImGui process them, then allow
        // the app to process the stashed events. This is done because ImGui might wish to block
        // certain events from the app (e.g., when dragging the mouse over an obscuring window).
        std::vector<filament::app::AppEvent> events;
        mDisplayManager->pollEvents(events);

        for (const auto& event: events) {
            if (mImGuiHelper) {
                ImGuiIO& io = ImGui::GetIO();
                switch (event.type) {
                    case AppEvent::Type::MOUSE_WHEEL: {
                        io.MouseWheel += event.mouseWheel.delta;
                        break;
                    }
                    case AppEvent::Type::MOUSE_BUTTON_DOWN: {
                        if (event.mouseButton.button == 1) mousePressed[0] = true;
                        if (event.mouseButton.button == 3) mousePressed[1] = true;
                        if (event.mouseButton.button == 2) mousePressed[2] = true;
                        break;
                    }
                    case AppEvent::Type::KEYDOWN:
                    case AppEvent::Type::KEYUP: {
                        io.AddKeyEvent(ImGuiMod_Ctrl,
                                (event.key.modifiers & AppKeyModifier::CTRL) != 0);
                        io.AddKeyEvent(ImGuiMod_Shift,
                                (event.key.modifiers & AppKeyModifier::SHIFT) != 0);
                        io.AddKeyEvent(ImGuiMod_Alt,
                                (event.key.modifiers & AppKeyModifier::ALT) != 0);
                        io.AddKeyEvent(ImGuiMod_Super,
                                (event.key.modifiers & AppKeyModifier::SUPER) != 0);
                        io.AddKeyEvent(filamentapp_utils::AppKeyToImGuiKey(event.key.code),
                                event.type == AppEvent::Type::KEYDOWN);
                        break;
                    }
                    case AppEvent::Type::TEXTINPUT: {
                        io.AddInputCharactersUTF8(event.text.text);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        // Now, loop over the events a second time for app-side processing.
        for (const auto& event: events) {
            ImGuiIO* io = mImGuiHelper ? &ImGui::GetIO() : nullptr;
            switch (event.type) {
                case AppEvent::Type::QUIT:
                    mClosed = true;
                    break;
                case AppEvent::Type::KEYDOWN:
                    if (event.key.code == AppKey::ESCAPE) {
                        mClosed = true;
                    }
#ifndef NDEBUG
                    if (event.key.code == AppKey::PRINT_SCREEN) {
                        DebugRegistry& debug = mEngine->getDebugRegistry();
                        bool* captureFrame =
                                debug.getPropertyAddress<bool>("d.renderer.doFrameCapture");
                        *captureFrame = true;
                    }
#endif
                    window->keyDown(event.key.code);
                    break;
                case AppEvent::Type::KEYUP:
                    window->keyUp(event.key.code);
                    break;
                case AppEvent::Type::MOUSE_WHEEL:
                    if (!io || !io->WantCaptureMouse) window->mouseWheel(event.mouseWheel.delta);
                    break;
                case AppEvent::Type::MOUSE_BUTTON_DOWN:
                    if (!io || !io->WantCaptureMouse)
                        window->mouseDown(event.mouseButton.button, event.mouseButton.x,
                                event.mouseButton.y);
                    break;
                case AppEvent::Type::MOUSE_BUTTON_UP:
                    if (!io || !io->WantCaptureMouse)
                        window->mouseUp(event.mouseButton.x, event.mouseButton.y);
                    break;
                case AppEvent::Type::MOUSE_MOVE:
                    if (!io || !io->WantCaptureMouse)
                        window->mouseMoved(event.mouseMove.x, event.mouseMove.y);
                    break;
                case AppEvent::Type::DROP_FILE:
                    if (mDropHandler) {
                        mDropHandler(event.dropFile.path);
                    }
                    break;
                case AppEvent::Type::RESIZED:
                    window->resize(mCameraParams);
                    // Call the resize callback, if this FilamentApp has one. This must be done
                    // after configureCamerasForWindow, so the viewports are correct.
                    if (mResize) {
                        mResize(mEngine, window->mMainView->getView());
                    }
                    break;
                default:
                    break;
            }
        }

        // Calculate the time step.
        static double lastTime = 0;
        double now = mDisplayManager->getTime();
        const float timeStep = lastTime > 0 ? (float) (now - lastTime) : (float) (1.0f / 60.0f);
        lastTime = now;

        // Populate the UI scene, regardless of whether Filament wants to a skip frame. We should
        // always let ImGui generate a command list; if it skips a frame it'll destroy its widgets.
        if (mImGuiHelper) {

            // Inform ImGui of the current window size in case it was resized.
            uint32_t windowWidth, windowHeight;
            uint32_t displayWidth, displayHeight;
            mDisplayManager->getWindowSize(window->mWindow, &windowWidth, &windowHeight);
            mDisplayManager->getDrawableSize(window->mWindow, &displayWidth, &displayHeight);
            mImGuiHelper->setDisplaySize(windowWidth, windowHeight,
                    windowWidth > 0 ? ((float) displayWidth / windowWidth) : 0,
                    displayHeight > 0 ? ((float) displayHeight / windowHeight) : 0);

            // Setup mouse inputs (we already got mouse wheel, keyboard keys & characters
            // from our event handler)
            ImGuiIO& io = ImGui::GetIO();
            int mx, my;
            uint32_t buttons = mDisplayManager->getMouseState(&mx, &my);
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
            io.MouseDown[0] = mousePressed[0] || (buttons & (1 << 0)) != 0;
            io.MouseDown[1] = mousePressed[1] || (buttons & (1 << 2)) != 0;
            io.MouseDown[2] = mousePressed[2] || (buttons & (1 << 1)) != 0;
            mousePressed[0] = mousePressed[1] = mousePressed[2] = false;

            if (mDisplayManager->isWindowFocused(window->mWindow)) {
                io.MousePos = ImVec2((float)mx, (float)my);
            }

            // Populate the UI Scene.
            mImGuiHelper->render(timeStep, imguiCallback);
        }

        // Update the camera manipulators for each view.
        for (auto const& view : window->mViews) {
            auto* cm = view->getCameraManipulator();
            if (cm) {
                cm->update(timeStep);
            }
        }

        // Update the position and orientation of the two cameras.
        filament::math::float3 eye, center, up;
        window->mMainCameraMan->getLookAt(&eye, &center, &up);
        window->mMainCamera->lookAt(eye, center, up);

        window->mDebugCameraMan->getLookAt(&eye, &center, &up);
        window->mDebugCamera->lookAt(eye, center, up);
        window->mDebugCamera->setExposure(
            window->mMainCamera->getAperture(),
            window->mMainCamera->getShutterSpeed(),
            window->mMainCamera->getSensitivity());

        window->mOrthoCamera->setExposure(
            window->mMainCamera->getAperture(),
            window->mMainCamera->getShutterSpeed(),
            window->mMainCamera->getSensitivity());

        auto const fci = window->mMainView->getView()->getFroxelConfigurationInfo();
        if (UTILS_UNLIKELY(fci.age != mFroxelInfoAge)) {
            mFroxelInfoAge = fci.age;
            auto width = fci.info.width;
            auto height = fci.info.height;
            auto depth = fci.info.depth;
            auto froxelDimension = fci.info.froxelDimension;
            auto viewportWidth = fci.info.viewportWidth;
            auto viewportHeight = fci.info.viewportHeight;
            auto zLightFar = fci.info.zLightFar;
            auto linearizer = fci.info.linearizer;
            auto p = fci.info.p;
            auto ct = fci.info.clipTransform;
            cameraGrid->update(width, height, depth,
                [=](int const i) {
                    float x = float(2 * i * froxelDimension.x) / float(viewportWidth ) - 1.0f;
                    x = (x - ct.z) / ct.x;
                    return x;
                },
                [=](int const j) {
                    float y =  float(2 * j * froxelDimension.y) / float(viewportHeight) - 1.0f;
                    y = (y - ct.w) / ct.y;
                    return y;
                },
                [=](int const k) {
                    float const z_view = -zLightFar * std::exp2(float(k - depth) * linearizer);
                    auto c = p * float4{ 0, 0, z_view, 1 };
                    float const z_clip_dx = k == 0 ? 1.0f : c.z / c.w;
                    float const z_clip_gl = (1 - z_clip_dx) * 2.0f - 1.0f;
                    return z_clip_gl;
            });
        }

        auto& rcm = mEngine->getRenderableManager();
        if (config.splitView) {
            rcm.setLayerMask(rcm.getInstance(cameraCube->getSolidRenderable()),     0x3, mCameraFrustumEnabled);
            rcm.setLayerMask(rcm.getInstance(cameraCube->getWireFrameRenderable()), 0x3, mCameraFrustumEnabled);
        }
        rcm.setLayerMask(rcm.getInstance(cameraGrid->getWireFrameRenderable()), 0x3, mFroxelGridEnabled);

        // Update the cube distortion matrix used for frustum visualization.
        auto const csm = window->mMainView->getView()->getDirectionalShadowCameras();
        // show/hide the cascades
        for (size_t i = 0 ; i < 4; i++) {
            rcm.setLayerMask(rcm.getInstance(lightmapCubes[i].getSolidRenderable()), 0x3, 0x0);
            rcm.setLayerMask(rcm.getInstance(lightmapCubes[i].getWireFrameRenderable()), 0x3, 0x0);
        }
        if (!csm.empty()) {
            for (size_t i = 0, c = csm.size(); i < c; i++) {
                if (csm[i]) {
                    lightmapCubes[i].mapFrustum(*mEngine, csm[i]);
                }
                uint8_t const layer = csm[i] ? mDirectionalShadowFrustumEnabled : 0x0;
                rcm.setLayerMask(rcm.getInstance(lightmapCubes[i].getSolidRenderable()), 0x3, layer);
                rcm.setLayerMask(rcm.getInstance(lightmapCubes[i].getWireFrameRenderable()), 0x3, layer);
            }
        }

        cameraCube->mapFrustum(*mEngine, window->mMainCamera);
        cameraGrid->mapFrustum(*mEngine, window->mMainCamera);

        // Delay rendering for roughly one monitor refresh interval
        // TODO: Use SDL_GL_SetSwapInterval for proper vsync
        // For now, we'll use a fixed 16ms sleep for SDL-like platforms to match original behavior.
        // HtmlDisplayManager might not need this if it has its own throttling.
        if (mDisplayManager->isVsyncSupported()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        Renderer* renderer = window->getRenderer();

        if (preRender) {
            preRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
        }

        if (mReconfigureCameras) {
            window->configureCamerasForWindow(mCameraParams);
            mReconfigureCameras = false;
        }

        if (config.splitView) {
            if(!window->mOrthoView->getView()->hasCamera()) {
                auto const csm = window->mMainView->getView()->getDirectionalShadowCameras();
                if (!csm.empty()) {
                    // here we could choose the cascade
                    Camera const* debugDirectionalShadowCamera = csm[0];
                    if (debugDirectionalShadowCamera) {
                        window->mOrthoView->setCamera(
                                const_cast<Camera*>(debugDirectionalShadowCamera));
                    }
                }
            }
        }

        if (renderer->beginFrame(window->getSwapChain())) {
            for (filament::View* offscreenView: mOffscreenViews) {
                renderer->render(offscreenView);
            }
            for (auto const& view: window->mViews) {
                renderer->render(view->getView());
            }

            mDisplayManager->onFrameFinished(window->mWindow, mEngine, renderer);

            if (postRender) {
                postRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
            }
            renderer->endFrame();
        } else {
            ++mSkippedFrames;
        }
    }
#ifdef __EXCEPTIONS
} catch (Panic const& e) {
    LOG(ERROR) << "Filament exception (terminate cleanly): " << e.what();
    LOG(ERROR) << e.getCallStack();
} catch (std::exception const& e) {
    LOG(ERROR) << "System exception (terminate cleanly): " << e.what();
} catch (...) {
    LOG(ERROR) << "Unknown exception! (terminate cleanly)";
}
#endif

    if (mImGuiHelper) {
        mImGuiHelper.reset();
    }

    cleanupCallback(mEngine, window->mMainView->getView(), mScene);

    cameraCube.reset();
    cameraGrid.reset();
    lightmapCubes.clear();
    window.reset();

    mIBL.reset();
    mEngine->destroy(mDepthMI);
    mEngine->destroy(mDepthMaterial);
    mEngine->destroy(mDefaultMaterial);
    mEngine->destroy(mTransparentMaterial);
    mEngine->destroy(mScene);
    Engine::destroy(&mEngine);
    mEngine = nullptr;

    if (mVulkanPlatform) {
        filament::app::destroyVulkanPlatform(mVulkanPlatform);
    }
    if (mWebGPUPlatform) {
        filament::app::destroyWebGPUPlatform(mWebGPUPlatform);
    }
}

// RELATIVE_ASSET_PATH is set inside samples/CMakeLists.txt and used to support multi-configuration
// generators, like Visual Studio or Xcode.
#ifndef RELATIVE_ASSET_PATH
#define RELATIVE_ASSET_PATH "."
#endif

const utils::Path& FilamentApp::getRootAssetsPath() {
    static const utils::Path root = utils::Path::getCurrentExecutable().getParent() + RELATIVE_ASSET_PATH;
    return root;
}

void FilamentApp::loadIBL(std::string_view path) {
    Path iblPath(path);
    if (!iblPath.exists()) {
        std::cerr << "The specified IBL path does not exist: " << iblPath << std::endl;
        return;
    }

    // Note that IBL holds a skybox, and Scene also holds a reference.  We cannot release IBL's
    // skybox until after new skybox has been set in the scene.
    std::unique_ptr<IBL> oldIBL = std::move(mIBL);
    mIBL = std::make_unique<IBL>(*mEngine);

    if (!iblPath.isDirectory()) {
        if (!mIBL->loadFromEquirect(iblPath)) {
            std::cerr << "Could not load the specified IBL: " << iblPath << std::endl;
            mIBL.reset(nullptr);
            return;
        }
    } else {
        if (!mIBL->loadFromDirectory(iblPath)) {
            std::cerr << "Could not load the specified IBL: " << iblPath << std::endl;
            mIBL.reset(nullptr);
            return;
        }
    }

    if (mIBL != nullptr) {
        mIBL->getSkybox()->setLayerMask(0x7, 0x4);
        mScene->setSkybox(mIBL->getSkybox());
        mScene->setIndirectLight(mIBL->getIndirectLight());
    }
}

void FilamentApp::loadIBL(const Config& config) {
    if (config.iblDirectory.empty()) {
        return;
    }
    loadIBL(config.iblDirectory);
}

void FilamentApp::loadDirt(const Config& config) {
    if (!config.dirt.empty()) {
        Path dirtPath(config.dirt);

        if (!dirtPath.exists()) {
            std::cerr << "The specified dirt file does not exist: " << dirtPath << std::endl;
            return;
        }

        if (!dirtPath.isFile()) {
            std::cerr << "The specified dirt path is not a file: " << dirtPath << std::endl;
            return;
        }

        int w, h, n;

        unsigned char* data = stbi_load(dirtPath.getAbsolutePath().c_str(), &w, &h, &n, 3);
        assert(n == 3);

        mDirt = Texture::Builder()
                .width(w)
                .height(h)
                .format(Texture::InternalFormat::RGB8)
                .build(*mEngine);

        mDirt->setImage(*mEngine, 0, { data, size_t(w * h * 3),
                Texture::Format::RGB, Texture::Type::UBYTE,
                (Texture::PixelBufferDescriptor::Callback)&stbi_image_free });
    }
}

void FilamentApp::setCameraFrustumEnabled(bool enabled) noexcept {
    mCameraFrustumEnabled = enabled ? 0x2 : 0x0;
}

void FilamentApp::setDirectionalShadowFrustumEnabled(bool enabled) noexcept {
    mDirectionalShadowFrustumEnabled = enabled ? 0x2 : 0x0;
}

void FilamentApp::setFroxelGridEnabled(bool enabled) noexcept {
    mFroxelGridEnabled = enabled ? 0x3 : 0x0;
}

bool FilamentApp::isCameraFrustumEnabled() const noexcept {
    return !!mCameraFrustumEnabled;
}

bool FilamentApp::isDirectionalShadowFrustumEnabled() const noexcept {
    return !!mDirectionalShadowFrustumEnabled;
}

bool FilamentApp::isFroxelGridEnabled() const noexcept {
    return !!mFroxelGridEnabled;
}

// ------------------------------------------------------------------------------------------------

FilamentApp::Window::Window(FilamentApp* filamentApp, const Config& config, std::string title,
        WindowCameraParams const& cameraParams, size_t w, size_t h)
        : mDisplayManager(filamentApp->mDisplayManager),
          mEngine(filamentApp->mEngine),
          mConfig(config),
          mIsHeadless(config.headless) {

    mWindow =
            mDisplayManager->createWindow(title.c_str(), w, h, config.resizeable, config.headless);

    void* nativeWindow = mDisplayManager->getNativeWindow(mWindow);
    auto engine = mEngine;

    mWidth = w;
    mHeight = h;

    // Write back the active feature level.
    config.featureLevel = engine->getActiveFeatureLevel();


    if (config.headless) {
        mSwapChain = engine->createSwapChain((uint32_t) w, (uint32_t) h);
    } else {
        mSwapChain = engine->createSwapChain(nativeWindow,
                filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
    }

    mRenderer = engine->createRenderer();

    // create cameras
    utils::EntityManager& em = utils::EntityManager::get();
    em.create(3, mCameraEntities);
    mCameras[0] = mMainCamera = engine->createCamera(mCameraEntities[0]);
    mCameras[1] = mDebugCamera = engine->createCamera(mCameraEntities[1]);
    mCameras[2] = mOrthoCamera = engine->createCamera(mCameraEntities[2]);

    // set exposure
    for (auto camera : mCameras) {
        camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
    }

    // create views
    mViews.emplace_back(mMainView = new CView(*mRenderer, "Main View"));
    if (config.splitView) {
        mViews.emplace_back(mDepthView = new CView(*mRenderer, "Depth View"));
        mViews.emplace_back(mGodView = new GodView(*mRenderer, "God View"));
        mViews.emplace_back(mOrthoView = new CView(*mRenderer, "Shadow View"));
    }
    mViews.emplace_back(mUiView = new CView(*mRenderer, "UI View"));

    // set-up the camera manipulators
    mMainCameraMan = CameraManipulator::Builder()
            .targetPosition(0, 0, -4)
            .flightMoveDamping(15.0)
            .build(config.cameraMode);
    mDebugCameraMan = CameraManipulator::Builder()
            .targetPosition(0, 0, -4)
            .flightMoveDamping(15.0)
            .build(config.cameraMode);

    mMainView->setCamera(mMainCamera);
    mMainView->setCameraManipulator(mMainCameraMan);
    if (config.splitView) {
        // Depth view always uses the main camera
        mDepthView->setCamera(mMainCamera);
        mDepthView->setCameraManipulator(mMainCameraMan);

        // The god view uses the main camera for culling, but the debug camera for viewing
        mGodView->setCamera(mMainCamera);
        mGodView->setGodCamera(mDebugCamera);
        mGodView->setCameraManipulator(mDebugCameraMan);
    }

    // configure the cameras
    configureCamerasForWindow(cameraParams);

    mMainCamera->lookAt({4, 0, -4}, {0, 0, -4}, {0, 1, 0});
}

FilamentApp::Window::~Window() {
    mViews.clear();
    utils::EntityManager& em = utils::EntityManager::get();
    for (auto e : mCameraEntities) {
        mEngine->destroyCameraComponent(e);
        em.destroy(e);
    }
    mEngine->destroy(mRenderer);
    mEngine->destroy(mSwapChain);
    mDisplayManager->destroyWindow(mWindow);
    delete mMainCameraMan;
    delete mDebugCameraMan;
}

void FilamentApp::Window::mouseDown(int button, ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    y = mHeight - y;
    for (auto const& view : mViews) {
        if (view->intersects(x, y)) {
            mMouseEventTarget = view.get();
            view->mouseDown(button, x, y);
            break;
        }
    }
}

void FilamentApp::Window::mouseWheel(ssize_t x) {
    if (mMouseEventTarget) {
        mMouseEventTarget->mouseWheel(x);
    } else {
        for (auto const& view : mViews) {
            if (view->intersects(mLastX, mLastY)) {
                view->mouseWheel(x);
                break;
            }
        }
    }
}

void FilamentApp::Window::mouseUp(ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    if (mMouseEventTarget) {
        y = mHeight - y;
        mMouseEventTarget->mouseUp(x, y);
        mMouseEventTarget = nullptr;
    }
}

void FilamentApp::Window::mouseMoved(ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    y = mHeight - y;
    if (mMouseEventTarget) {
        mMouseEventTarget->mouseMoved(x, y);
    }
    mLastX = x;
    mLastY = y;
}

void FilamentApp::Window::keyDown(AppKey key) {
    auto& eventTarget = mKeyEventTarget[key];

    // keyDown events can be sent multiple times per key (for key repeat)
    // If this key is already down, do nothing.
    if (eventTarget) {
        return;
    }

    // Decide which view will get this key's corresponding keyUp event.
    // If we're currently in a mouse grap session, it should be the mouse grab's target view.
    // Otherwise, it should be whichever view we're currently hovering over.
    CView* targetView = nullptr;
    if (mMouseEventTarget) {
        targetView = mMouseEventTarget;
    } else {
        for (auto const& view : mViews) {
            if (view->intersects(mLastX, mLastY)) {
                targetView = view.get();
                break;
            }
        }
    }

    if (targetView) {
        targetView->keyDown(key);
        eventTarget = targetView;
    }
}

void FilamentApp::Window::keyUp(AppKey key) {
    auto& eventTarget = mKeyEventTarget[key];
    if (!eventTarget) {
        return;
    }
    eventTarget->keyUp(key);
    eventTarget = nullptr;
}

void FilamentApp::Window::fixupMouseCoordinatesForHdpi(ssize_t& x, ssize_t& y) const {
    uint32_t dw, dh, ww, wh;
    mDisplayManager->getDrawableSize(mWindow, &dw, &dh);
    mDisplayManager->getWindowSize(mWindow, &ww, &wh);
    x = x * (ssize_t) dw / (ssize_t) ww;
    y = y * (ssize_t) dh / (ssize_t) wh;
}

void FilamentApp::Window::resize(WindowCameraParams const& cameraParams) {
    mDisplayManager->onWindowResized(mWindow);
    configureCamerasForWindow(cameraParams);
}

void FilamentApp::Window::configureCamerasForWindow(WindowCameraParams const& cameraParams) {
    float dpiScaleX = 1.0f;
    float dpiScaleY = 1.0f;

    // If the app is not headless, query the window for its physical & virtual sizes.
    if (!mIsHeadless) {
        uint32_t width, height;
        mDisplayManager->getDrawableSize(mWindow, &width, &height);
        mWidth = (size_t) width;
        mHeight = (size_t) height;

        uint32_t virtualWidth, virtualHeight;
        mDisplayManager->getWindowSize(mWindow, &virtualWidth, &virtualHeight);
        dpiScaleX = (float) width / virtualWidth;
        dpiScaleY = (float) height / virtualHeight;
    } else {
        uint32_t width, height;
        mDisplayManager->getWindowSize(mWindow, &width, &height);
        if (width != mWidth || height != mHeight) {
            mWidth = width;
            mHeight = height;
            if (mSwapChain) {
                mEngine->destroy(mSwapChain);
            }
            mSwapChain = mEngine->createSwapChain((uint32_t) width, (uint32_t) height);
        }
    }

    const uint32_t width = mWidth;
    const uint32_t height = mHeight;

    const float3 at(0, 0, -4);
    const double ratio = double(height) / double(width);
    const int sidebar = cameraParams.sidebarWidth * dpiScaleX;

    const bool splitview = mViews.size() > 2;

    const uint32_t mainWidth = std::max(2, (int) width - sidebar);

    double near = cameraParams.near;
    double far = cameraParams.far;
    auto aspectRatio = double(mainWidth) / height;
    if (mMainView->getView()->getStereoscopicOptions().enabled) {
        const int ec = mConfig.stereoscopicEyeCount;
        aspectRatio = double(mainWidth) / ec / height;

        mat4 projections[4];
        projections[0] = Camera::projection(cameraParams.focalLength, aspectRatio, near, far);
        projections[1] = projections[0];
        // simulate foveated rendering
        projections[2] = Camera::projection(cameraParams.focalLength * 2.0, aspectRatio, near, far);
        projections[3] = projections[2];
        mMainCamera->setCustomEyeProjection(projections, 4, projections[0], near, far);
    } else {
        mMainCamera->setLensProjection(cameraParams.focalLength, aspectRatio, near, far);
    }
    mDebugCamera->setProjection(45.0, aspectRatio, 0.0625, 4096, Camera::Fov::VERTICAL);

    // We're in split view when there are more views than just the Main and UI views.
    if (splitview) {
        uint32_t const vpw = mainWidth / 2;
        uint32_t const vph = height / 2;
        mMainView->setViewport ({ sidebar +            0,            0, vpw, vph });
        mDepthView->setViewport({ sidebar + int32_t(vpw),            0, vpw, vph });
        mGodView->setViewport  ({ sidebar + int32_t(vpw), int32_t(vph), vpw, vph });
        mOrthoView->setViewport({ sidebar +            0, int32_t(vph), vpw, vph });
    } else {
        mMainView->setViewport({ sidebar, 0, mainWidth, height });
    }
    mUiView->setViewport({ 0, 0, width, height });
}

// ------------------------------------------------------------------------------------------------

FilamentApp::CView::CView(Renderer& renderer, std::string name)
        : engine(*renderer.getEngine()), mName(name) {
    view = engine.createView();
    view->setName(name.c_str());
}

FilamentApp::CView::~CView() {
    engine.destroy(view);
}

void FilamentApp::CView::setViewport(filament::Viewport const& viewport) {
    mViewport = viewport;
    view->setViewport(viewport);
    if (mCameraManipulator) {
        mCameraManipulator->setViewport(viewport.width, viewport.height);
    }
}

void FilamentApp::CView::mouseDown(int button, ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabBegin(x, y, button == 3);
    }
}

void FilamentApp::CView::mouseUp(ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabEnd();
    }
}

void FilamentApp::CView::mouseMoved(ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabUpdate(x, y);
    }
}

void FilamentApp::CView::mouseWheel(ssize_t x) {
    if (mCameraManipulator) {
        mCameraManipulator->scroll(0, 0, x);
    }
}

bool FilamentApp::manipulatorKeyFromKeycode(AppKey scancode, CameraManipulator::Key& key) {
    switch (scancode) {
        case AppKey::W:
            key = CameraManipulator::Key::FORWARD;
            return true;
        case AppKey::A:
            key = CameraManipulator::Key::LEFT;
            return true;
        case AppKey::S:
            key = CameraManipulator::Key::BACKWARD;
            return true;
        case AppKey::D:
            key = CameraManipulator::Key::RIGHT;
            return true;
        case AppKey::E:
            key = CameraManipulator::Key::UP;
            return true;
        case AppKey::Q:
            key = CameraManipulator::Key::DOWN;
            return true;
        default:
            return false;
    }
}

void FilamentApp::CView::keyUp(AppKey scancode) {
    if (mCameraManipulator) {
        CameraManipulator::Key key;
        if (manipulatorKeyFromKeycode(scancode, key)) {
            mCameraManipulator->keyUp(key);
        }
    }
}

void FilamentApp::CView::keyDown(AppKey scancode) {
    if (mCameraManipulator) {
        CameraManipulator::Key key;
        if (manipulatorKeyFromKeycode(scancode, key)) {
            mCameraManipulator->keyDown(key);
        }
    }
}

bool FilamentApp::CView::intersects(ssize_t x, ssize_t y) {
    if (x >= mViewport.left && x < mViewport.left + mViewport.width)
        if (y >= mViewport.bottom && y < mViewport.bottom + mViewport.height)
            return true;

    return false;
}

void FilamentApp::CView::setCameraManipulator(CameraManipulator* cm) {
    mCameraManipulator = cm;
}

void FilamentApp::CView::setCamera(Camera* camera) {
    view->setCamera(camera);
}

void FilamentApp::GodView::setGodCamera(Camera* camera) {
    getView()->setDebugCamera(camera);
}
