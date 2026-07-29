/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <filamentapp/FilamentApp2.h>

#include <filamentapp/Config.h>

#include "PlatformHelper.h"


#if defined(FILAMENTAPP_HAS_WEB_UI)
#include "display_managers/HtmlDisplayManager.h"
#endif // defined(FILAMENTAPP_HAS_WEB_UI)

#include <filamentapp/DisplayManager.h>

#ifdef FILAMENTAPP_HAS_SDL
#include "display_managers/SDLDisplayManager.h"
#endif // defined(FILAMENTAPP_HAS_SDL)

#if defined(WIN32)
#include <utils/unwindows.h>
#endif

#include <iostream>

#include "FilamentAppGui.h"

#include <utils/EntityManager.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <filament/Camera.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
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

using namespace filament::math;
using namespace utils;
using namespace filament::app;

namespace {

using namespace filament::backend;
}

std::unique_ptr<FilamentApp2> FilamentApp2::Builder::build() {
    return std::unique_ptr<FilamentApp2>(new FilamentApp2(*this));
}

FilamentApp2::FilamentApp2(const Builder& builder)
        : mWindowWidth(builder.mWidth),
          mWindowHeight(builder.mHeight),
          mIblDirectory(builder.mIblDirectory),
          mDirtPath(builder.mDirt),
          mScale(builder.mScale),
          mBackend(builder.mBackend),
          mFeatureLevel(builder.mFeatureLevel),
          mCameraMode(builder.mCameraMode),
          mResizeable(builder.mResizeable),
          mHeadless(builder.mHeadless),
          mStereoscopicEyeCount(builder.mStereoscopicEyeCount),
          mSamples(builder.mSamples),
          mVulkanGPUHint(builder.mVulkanGPUHint),
          mForcedWebGPUBackend(builder.mForcedWebGPUBackend),
          mDisplayManagerConfig(builder.mDisplayManagerConfig),
          mAsynchronousMode(builder.mAsynchronousMode),
          mDisplayManager(builder.mDisplayManager),
          mSetupCallback(builder.mSetup),
          mCleanupCallback(builder.mCleanup),
          mPreRender(builder.mPreRender),
          mPostRender(builder.mPostRender),
          mImguiCallback(builder.mImgui),
          mAnimation(builder.mAnimation),
          mResize(builder.mResize),
          mDropHandler(builder.mDropHandler) {}

FilamentApp2::~FilamentApp2() {
    if (mDisplayManager) {
        mDisplayManager->terminate();
        delete mDisplayManager;
    }
}

void FilamentApp2::onSurfaceCreated(void* nativeWindow) {
    // To be implemented in later PRs
}

void FilamentApp2::onSurfaceChanged(int width, int height) {
    // To be implemented in later PRs
}

void FilamentApp2::onSurfaceDestroyed() {
    // To be implemented in later PRs
}

void FilamentApp2::onTouchEvent(int action, float x, float y) {
    // To be implemented in later PRs
}

View* FilamentApp2::getGuiView() const noexcept { return mAppGui ? mAppGui->getView() : nullptr; }

void FilamentApp2::run() {


    // Note that we need to determine the backend in order to build custom platforms.
    Engine::Backend backend = filament::app::resolveBackend(mBackend);

    backend::Platform* platform = nullptr;
    if (backend == Engine::Backend::VULKAN) {
        platform = mVulkanPlatform = filament::app::createVulkanPlatform(mVulkanGPUHint.c_str());
    } else if (backend == Engine::Backend::WEBGPU) {
        platform = mWebGPUPlatform = filament::app::createWebGPUPlatform(mForcedWebGPUBackend);
    }

    Engine::Config engineConfig = {
#if defined(FILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)
        .stereoscopicType = Engine::StereoscopicType::INSTANCED,
#elif defined(FILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
        .stereoscopicType = Engine::StereoscopicType::MULTIVIEW,
#else
        .stereoscopicType = Engine::StereoscopicType::NONE,
#endif
        .stereoscopicEyeCount = (uint8_t) mStereoscopicEyeCount,
        .asynchronousMode = mAsynchronousMode,
    };

    // "backend.enable_asynchronous_operation" is forcibly enabled here, inheriting the setting
    // from the Engine, purely to demonstrate the object's asynchronous behavior within
    // Filament. Users should manage this flag at their discretion.
    mEngine = Engine::Builder()
                      .backend(backend)
                      .featureLevel(mFeatureLevel)
                      .feature("backend.enable_asynchronous_operation",
                              engineConfig.asynchronousMode != AsynchronousMode::NONE)
                      .platform(platform)
                      .config(&engineConfig)
                      .build();

    assert_invariant(mEngine->getBackend() == backend);

    // By now we have resolved to a specific backend (instead of default).
    mBackend = backend;

    if (!mDisplayManager) {
        if (mDisplayManagerConfig == DisplayManager::WEB) {
#if defined(FILAMENTAPP_HAS_WEB_UI)
            mDisplayManager = new HtmlDisplayManager();
#endif // defined(FILAMENTAPP_HAS_WEB_UI)
        } else {
#ifdef FILAMENTAPP_HAS_SDL
            mDisplayManager = new SDLDisplayManager();
#else  // !defined(FILAMENTAPP_HAS_SDL)
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "SDLDisplayManager is not available on this platform";
#endif // defined(FILAMENTAPP_HAS_SDL)
        }
    }


    Config dummyConfig;
    dummyConfig.title = mWindowTitle;
    dummyConfig.width = mWindowWidth;
    dummyConfig.height = mWindowHeight;
    dummyConfig.iblDirectory = mIblDirectory;
    dummyConfig.dirt = mDirtPath;
    dummyConfig.scale = mScale;
    dummyConfig.splitView = mIsSplitView;
    dummyConfig.backend = mBackend;
    dummyConfig.featureLevel = mFeatureLevel;
    dummyConfig.cameraMode = mCameraMode;
    dummyConfig.resizeable = mResizeable;
    dummyConfig.headless = mHeadless;
    dummyConfig.stereoscopicEyeCount = mStereoscopicEyeCount;
    dummyConfig.samples = mSamples;
    dummyConfig.vulkanGPUHint = mVulkanGPUHint;
    dummyConfig.forcedWebGPUBackend = mForcedWebGPUBackend;
    dummyConfig.displayManager = static_cast<Config::DisplayManager>(mDisplayManagerConfig);
    dummyConfig.asynchronousMode = mAsynchronousMode;

    if (!mDisplayManager->init(dummyConfig)) {
        LOG(ERROR) << "Failed to initialize display manager" << utils::io::endl;
        return;
    }


    mWindow.reset(new FilamentApp2::Window(this, mWindowTitle, mCameraParams, mWindowWidth,
            mWindowHeight));
    Window* window = mWindow.get();

    mDepthMaterial =
            Material::Builder()
                    .package(FILAMENTAPP_DEPTHVISUALIZER_DATA, FILAMENTAPP_DEPTHVISUALIZER_SIZE)
                    .build(*mEngine);

    mDepthMI = mDepthMaterial->createInstance();

    mDefaultMaterial =
            Material::Builder()
                    .package(FILAMENTAPP_AIDEFAULTMAT_DATA, FILAMENTAPP_AIDEFAULTMAT_SIZE)
                    .build(*mEngine);

    mTransparentMaterial =
            Material::Builder()
                    .package(FILAMENTAPP_TRANSPARENTCOLOR_DATA, FILAMENTAPP_TRANSPARENTCOLOR_SIZE)
                    .build(*mEngine);

    mCameraCube.reset(new Cube(*mEngine, mTransparentMaterial, { 1, 0, 0 }));
    mCameraGrid.reset(new Grid(*mEngine, mTransparentMaterial, { 1, 1, 0 }));

    // we can't cull the light-frustum because it's not applied a rigid transform
    // and currently, filament assumes that for culling
    mLightmapCubes.reserve(4);
    mLightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 0, 1, 0 }, false);
    mLightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 0, 0, 1 }, false);
    mLightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 1, 1, 0 }, false);
    mLightmapCubes.emplace_back(*mEngine, mTransparentMaterial, float3{ 1, 0, 0 }, false);

    mScene = mEngine->createScene();

    window->mMainView->getView()->setVisibleLayers(0x4, 0x4);
    window->mMainView->getView()->setFroxelVizEnabled(true);

    if (mIsSplitView) {
        mScene->addEntity(mCameraCube->getSolidRenderable());
        mScene->addEntity(mCameraCube->getWireFrameRenderable());
        for (auto&& cube: mLightmapCubes) {
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
    mScene->addEntity(mCameraGrid->getWireFrameRenderable());

    loadDirt();
    loadIBL();

    for (auto& view: window->mViews) {
        if (view.get() != window->mUiView) {
            view->getView()->setScene(mScene);
        }
    }

    if (mSetupCallback) {
        mSetupCallback(mEngine, window->mMainView->getView(), mScene);
    }

    if (mImguiCallback) {
        mAppGui = std::make_unique<FilamentAppGui>(mEngine, window->mUiView->getView(),
                getRootAssetsPath() + "assets/fonts/Roboto-Medium.ttf");
    }

    mDisplayManager->startRendering([this] { return doFrame(); });
}


bool FilamentApp2::doFrame() {
#ifdef __EXCEPTIONS
    try {
#endif
        Window* window = mWindow.get();
        if (!window) {
            return true;
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

        if (mAppGui) {
            mAppGui->processAppEvents(events);
        }

        // Now, loop over the events a second time for app-side processing.
        for (const auto& event: events) {
            if (mClosed) {
                break;
            }
            bool wantCaptureMouse = mAppGui ? mAppGui->wantCaptureMouse() : false;
            switch (event.type) {
                case AppEvent::Type::QUIT:
                    mClosed = true;
                    break;
                case AppEvent::Type::KEYDOWN:
                    if (event.key.code == AppKey::ESCAPE) {
                        mClosed = true;
                    }
#if !defined(NDEBUG) && !defined(__ANDROID__)
                    if (event.key.code == AppKey::PRINT_SCREEN) {
                        DebugRegistry& debug = mEngine->getDebugRegistry();
                        if (bool* captureFrame = debug.getPropertyAddress<bool>(
                                    "d.renderer.doFrameCapture")) {
                            *captureFrame = true;
                        }
                    }
#endif
                    window->keyDown(event.key.code);
                    break;
                case AppEvent::Type::KEYUP:
                    window->keyUp(event.key.code);
                    break;
                case AppEvent::Type::MOUSE_WHEEL:
                    if (!wantCaptureMouse) window->mouseWheel(event.mouseWheel.delta);
                    break;
                case AppEvent::Type::MOUSE_BUTTON_DOWN:
                    if (!wantCaptureMouse)
                        window->mouseDown(event.mouseButton.button, event.mouseButton.x,
                                event.mouseButton.y);
                    break;
                case AppEvent::Type::MOUSE_BUTTON_UP:
                    if (!wantCaptureMouse)
                        window->mouseUp(event.mouseButton.x, event.mouseButton.y);
                    break;
                case AppEvent::Type::MOUSE_MOVE:
                    if (!wantCaptureMouse) window->mouseMoved(event.mouseMove.x, event.mouseMove.y);
                    break;
                case AppEvent::Type::DROP_FILE:
                    if (mDropHandler) {
                        mDropHandler(event.dropFile.path);
                    }
                    break;
                case AppEvent::Type::RESIZED:
                    window->resize(mCameraParams);
                    // Call the resize callback, if this FilamentApp2 has one. This must be done
                    // after configureCamerasForWindow, so the viewports are correct.
                    if (mResize) {
                        mResize(mEngine, window->mMainView->getView());
                    }
                    break;
                default:
                    break;
            }
        }

        // Note that shutdown() does not destroy the DisplayManager, but the destructor does. The
        // return here will go back to the caller, which is a DisplayManager.
        if (mClosed) {
            shutdown();
            return true;
        }

        // Calculate the time step.
        static double lastTime = 0;
        double now = mDisplayManager->getTime();
        const float timeStep = lastTime > 0 ? (float) (now - lastTime) : (float) (1.0f / 60.0f);
        lastTime = now;

        // Populate the UI scene, regardless of whether Filament wants to a skip frame. We should
        // always let ImGui generate a command list; if it skips a frame it'll destroy its widgets.
        if (mAppGui) {
            mAppGui->render(timeStep, mDisplayManager, window->mWindow, mImguiCallback,
                    mMousePressed);
        }

        // Update the camera manipulators for each view.
        for (auto const& view: window->mViews) {
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
        window->mDebugCamera->setExposure(window->mMainCamera->getAperture(),
                window->mMainCamera->getShutterSpeed(), window->mMainCamera->getSensitivity());

        window->mOrthoCamera->setExposure(window->mMainCamera->getAperture(),
                window->mMainCamera->getShutterSpeed(), window->mMainCamera->getSensitivity());

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
            mCameraGrid->update(
                    width, height, depth,
                    [=](int const i) {
                        float x = float(2 * i * froxelDimension.x) / float(viewportWidth) - 1.0f;
                        x = (x - ct.z) / ct.x;
                        return x;
                    },
                    [=](int const j) {
                        float y = float(2 * j * froxelDimension.y) / float(viewportHeight) - 1.0f;
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
        if (mIsSplitView) {
            rcm.setLayerMask(rcm.getInstance(mCameraCube->getSolidRenderable()), 0x3,
                    mCameraFrustumEnabled);
            rcm.setLayerMask(rcm.getInstance(mCameraCube->getWireFrameRenderable()), 0x3,
                    mCameraFrustumEnabled);
        }
        rcm.setLayerMask(rcm.getInstance(mCameraGrid->getWireFrameRenderable()), 0x3,
                mFroxelGridEnabled);

        // Update the cube distortion matrix used for frustum visualization.
        auto const csm = window->mMainView->getView()->getDirectionalShadowCameras();
        // show/hide the cascades
        for (size_t i = 0; i < 4; i++) {
            rcm.setLayerMask(rcm.getInstance(mLightmapCubes[i].getSolidRenderable()), 0x3, 0x0);
            rcm.setLayerMask(rcm.getInstance(mLightmapCubes[i].getWireFrameRenderable()), 0x3, 0x0);
        }
        if (!csm.empty()) {
            for (size_t i = 0, c = csm.size(); i < c; i++) {
                if (csm[i]) {
                    mLightmapCubes[i].mapFrustum(*mEngine, csm[i]);
                }
                uint8_t const layer = csm[i] ? mDirectionalShadowFrustumEnabled : 0x0;
                rcm.setLayerMask(rcm.getInstance(mLightmapCubes[i].getSolidRenderable()), 0x3,
                        layer);
                rcm.setLayerMask(rcm.getInstance(mLightmapCubes[i].getWireFrameRenderable()), 0x3,
                        layer);
            }
        }

        mCameraCube->mapFrustum(*mEngine, window->mMainCamera);
        mCameraGrid->mapFrustum(*mEngine, window->mMainCamera);
        Renderer* renderer = window->getRenderer();

        if (mPreRender) {
            mPreRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
        }

        if (mReconfigureCameras) {
            window->configureCamerasForWindow(mCameraParams);
            mReconfigureCameras = false;
        }

        if (mIsSplitView) {
            if (!window->mOrthoView->getView()->hasCamera()) {
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

            if (mPostRender) {
                mPostRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
            }
            renderer->endFrame();
        } else {
            ++mSkippedFrames;
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
    return false;
}

void FilamentApp2::shutdown() {
    if (mAppGui) {
        mAppGui.reset();
    }

    mCleanupCallback(mEngine, mWindow->mMainView->getView(), mScene);

    mCameraCube.reset();
    mCameraGrid.reset();
    mLightmapCubes.clear();
    mWindow.reset();
    mPrimarySwapChain = nullptr;

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
        mVulkanPlatform = nullptr;
    }
    if (mWebGPUPlatform) {
        filament::app::destroyWebGPUPlatform(mWebGPUPlatform);
        mWebGPUPlatform = nullptr;
    }
}

// RELATIVE_ASSET_PATH is set inside samples/CMakeLists.txt and used to support multi-configuration
// generators, like Visual Studio or Xcode.
#ifndef RELATIVE_ASSET_PATH
#define RELATIVE_ASSET_PATH "."
#endif

const utils::Path& FilamentApp2::getRootAssetsPath() {
    static const utils::Path root =
            utils::Path::getCurrentExecutable().getParent() + RELATIVE_ASSET_PATH;
    return root;
}

void FilamentApp2::loadIBL(std::string_view path) {
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

void FilamentApp2::loadIBL() {
    if (mIblDirectory.empty()) {
        return;
    }
    loadIBL(mIblDirectory);
}

void FilamentApp2::loadDirt() {
    if (!mDirtPath.empty()) {
        Path dirtPath(mDirtPath);

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

        mDirt->setImage(*mEngine, 0,
                { data, size_t(w * h * 3), Texture::Format::RGB, Texture::Type::UBYTE,
                    (Texture::PixelBufferDescriptor::Callback) &stbi_image_free });
    }
}

void FilamentApp2::setCameraFrustumEnabled(bool enabled) noexcept {
    mCameraFrustumEnabled = enabled ? 0x2 : 0x0;
}

void FilamentApp2::setDirectionalShadowFrustumEnabled(bool enabled) noexcept {
    mDirectionalShadowFrustumEnabled = enabled ? 0x2 : 0x0;
}

void FilamentApp2::setFroxelGridEnabled(bool enabled) noexcept {
    mFroxelGridEnabled = enabled ? 0x3 : 0x0;
}

bool FilamentApp2::isCameraFrustumEnabled() const noexcept { return !!mCameraFrustumEnabled; }

bool FilamentApp2::isDirectionalShadowFrustumEnabled() const noexcept {
    return !!mDirectionalShadowFrustumEnabled;
}

bool FilamentApp2::isFroxelGridEnabled() const noexcept { return !!mFroxelGridEnabled; }

// ------------------------------------------------------------------------------------------------

FilamentApp2::Window::Window(FilamentApp2* filamentApp, std::string title,
        WindowCameraParams const& cameraParams, size_t w, size_t h)
        : mDisplayManager(filamentApp->mDisplayManager),
          mEngine(filamentApp->mEngine),
          mApp(filamentApp) {
    mWindow = mDisplayManager->createWindow(title.c_str(), w, h, filamentApp->mResizeable,
            filamentApp->mHeadless);

    void* nativeWindow = mDisplayManager->getNativeWindow(mWindow);
    auto engine = mEngine;

    mWidth = w;
    mHeight = h;

    // Write back the active feature level.
    filamentApp->mFeatureLevel = engine->getActiveFeatureLevel();


    if (filamentApp->mHeadless) {
        mSwapChain = engine->createSwapChain((uint32_t) w, (uint32_t) h);
    } else {
        mSwapChain = engine->createSwapChain(nativeWindow,
                filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
    }
    filamentApp->mPrimarySwapChain = mSwapChain;

    mRenderer = engine->createRenderer();

    // create cameras
    utils::EntityManager& em = utils::EntityManager::get();
    em.create(3, mCameraEntities);
    mCameras[0] = mMainCamera = engine->createCamera(mCameraEntities[0]);
    mCameras[1] = mDebugCamera = engine->createCamera(mCameraEntities[1]);
    mCameras[2] = mOrthoCamera = engine->createCamera(mCameraEntities[2]);

    // set exposure
    for (auto camera: mCameras) {
        camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
    }

    // create views
    mViews.emplace_back(mMainView = new CView(*mRenderer, "Main View"));
    if (filamentApp->mIsSplitView) {
        mViews.emplace_back(mDepthView = new CView(*mRenderer, "Depth View"));
        mViews.emplace_back(mGodView = new GodView(*mRenderer, "God View"));
        mViews.emplace_back(mOrthoView = new CView(*mRenderer, "Shadow View"));
    }
    mViews.emplace_back(mUiView = new CView(*mRenderer, "UI View"));

    // set-up the camera manipulators
    mMainCameraMan =
            CameraManipulator::Builder().targetPosition(0, 0, -4).flightMoveDamping(15.0).build(
                    filamentApp->mCameraMode);
    mDebugCameraMan =
            CameraManipulator::Builder().targetPosition(0, 0, -4).flightMoveDamping(15.0).build(
                    filamentApp->mCameraMode);

    mMainView->setCamera(mMainCamera);
    mMainView->setCameraManipulator(mMainCameraMan);
    if (filamentApp->mIsSplitView) {
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

    mMainCamera->lookAt({ 4, 0, -4 }, { 0, 0, -4 }, { 0, 1, 0 });
}

FilamentApp2::Window::~Window() {
    mViews.clear();
    utils::EntityManager& em = utils::EntityManager::get();
    for (auto e: mCameraEntities) {
        mEngine->destroyCameraComponent(e);
        em.destroy(e);
    }
    mEngine->destroy(mRenderer);
    mEngine->destroy(mSwapChain);
    mDisplayManager->destroyWindow(mWindow);
    delete mMainCameraMan;
    delete mDebugCameraMan;
}

void FilamentApp2::Window::mouseDown(int button, ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    y = mHeight - y;
    for (auto const& view: mViews) {
        if (view->intersects(x, y)) {
            mMouseEventTarget = view.get();
            view->mouseDown(button, x, y);
            break;
        }
    }
}

void FilamentApp2::Window::mouseWheel(ssize_t x) {
    if (mMouseEventTarget) {
        mMouseEventTarget->mouseWheel(x);
    } else {
        for (auto const& view: mViews) {
            if (view->intersects(mLastX, mLastY)) {
                view->mouseWheel(x);
                break;
            }
        }
    }
}

void FilamentApp2::Window::mouseUp(ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    if (mMouseEventTarget) {
        y = mHeight - y;
        mMouseEventTarget->mouseUp(x, y);
        mMouseEventTarget = nullptr;
    }
}

void FilamentApp2::Window::mouseMoved(ssize_t x, ssize_t y) {
    fixupMouseCoordinatesForHdpi(x, y);
    y = mHeight - y;
    if (mMouseEventTarget) {
        mMouseEventTarget->mouseMoved(x, y);
    }
    mLastX = x;
    mLastY = y;
}

void FilamentApp2::Window::keyDown(AppKey key) {
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
        for (auto const& view: mViews) {
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

void FilamentApp2::Window::keyUp(AppKey key) {
    auto& eventTarget = mKeyEventTarget[key];
    if (!eventTarget) {
        return;
    }
    eventTarget->keyUp(key);
    eventTarget = nullptr;
}

void FilamentApp2::Window::fixupMouseCoordinatesForHdpi(ssize_t& x, ssize_t& y) const {
    uint32_t dw, dh, ww, wh;
    mDisplayManager->getDrawableSize(mWindow, &dw, &dh);
    mDisplayManager->getWindowSize(mWindow, &ww, &wh);
    x = x * (ssize_t) dw / (ssize_t) ww;
    y = y * (ssize_t) dh / (ssize_t) wh;
}

void FilamentApp2::Window::resize(WindowCameraParams const& cameraParams) {
    mDisplayManager->onWindowResized(mWindow);
    configureCamerasForWindow(cameraParams);
}

void FilamentApp2::Window::configureCamerasForWindow(WindowCameraParams const& cameraParams) {
    float dpiScaleX = 1.0f;
    float dpiScaleY = 1.0f;

    // If the app is not headless, query the window for its physical & virtual sizes.
    if (!mApp->mHeadless) {
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
        const int ec = mApp->mStereoscopicEyeCount;
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
        mMainView->setViewport({ sidebar + 0, 0, vpw, vph });
        mDepthView->setViewport({ sidebar + int32_t(vpw), 0, vpw, vph });
        mGodView->setViewport({ sidebar + int32_t(vpw), int32_t(vph), vpw, vph });
        mOrthoView->setViewport({ sidebar + 0, int32_t(vph), vpw, vph });
    } else {
        mMainView->setViewport({ sidebar, 0, mainWidth, height });
    }
    mUiView->setViewport({ 0, 0, width, height });
}

// ------------------------------------------------------------------------------------------------

FilamentApp2::CView::CView(Renderer& renderer, std::string name)
        : engine(*renderer.getEngine()),
          mName(name) {
    view = engine.createView();
    view->setName(name.c_str());
}

FilamentApp2::CView::~CView() { engine.destroy(view); }

void FilamentApp2::CView::setViewport(filament::Viewport const& viewport) {
    mViewport = viewport;
    view->setViewport(viewport);
    if (mCameraManipulator) {
        mCameraManipulator->setViewport(viewport.width, viewport.height);
    }
}

void FilamentApp2::CView::mouseDown(int button, ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabBegin(x, y, button == 3);
    }
}

void FilamentApp2::CView::mouseUp(ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabEnd();
    }
}

void FilamentApp2::CView::mouseMoved(ssize_t x, ssize_t y) {
    if (mCameraManipulator) {
        mCameraManipulator->grabUpdate(x, y);
    }
}

void FilamentApp2::CView::mouseWheel(ssize_t x) {
    if (mCameraManipulator) {
        mCameraManipulator->scroll(0, 0, x);
    }
}

bool FilamentApp2::manipulatorKeyFromKeycode(AppKey scancode, CameraManipulator::Key& key) {
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

void FilamentApp2::CView::keyUp(AppKey scancode) {
    if (mCameraManipulator) {
        CameraManipulator::Key key;
        if (manipulatorKeyFromKeycode(scancode, key)) {
            mCameraManipulator->keyUp(key);
        }
    }
}

void FilamentApp2::CView::keyDown(AppKey scancode) {
    if (mCameraManipulator) {
        CameraManipulator::Key key;
        if (manipulatorKeyFromKeycode(scancode, key)) {
            mCameraManipulator->keyDown(key);
        }
    }
}

bool FilamentApp2::CView::intersects(ssize_t x, ssize_t y) {
    if (x >= mViewport.left && x < mViewport.left + mViewport.width)
        if (y >= mViewport.bottom && y < mViewport.bottom + mViewport.height) return true;

    return false;
}

void FilamentApp2::CView::setCameraManipulator(CameraManipulator* cm) { mCameraManipulator = cm; }

void FilamentApp2::CView::setCamera(Camera* camera) { view->setCamera(camera); }

void FilamentApp2::GodView::setGodCamera(Camera* camera) { getView()->setDebugCamera(camera); }
