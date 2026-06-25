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

#ifndef TNT_FILAMENT_SAMPLE_FILAMENTAPP_H
#define TNT_FILAMENT_SAMPLE_FILAMENTAPP_H

#include "Config.h"
#include "Cube.h"
#include "Grid.h"
#include "IBL.h"

#include <filament/Engine.h>
#include <filament/Viewport.h>

#include <camutils/Manipulator.h>

#include <utils/Entity.h>
#include <utils/Path.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace filament {
class Renderer;
class Scene;
class SwapChain;
class View;
} // namespace filament

namespace filagui {
class ImGuiHelper;
} // namespace filagui

class IBL;
class MeshAssimp;

// For customizing the vulkan backend
namespace filament::backend {
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
class VulkanPlatform;
#endif

#if defined(FILAMENT_SUPPORTS_WEBGPU)
class WebGPUPlatform;
#endif

}

namespace filament::app {
class DisplayManager;
enum class AppKey : uint32_t;
} // namespace filament::app

class FilamentApp {
public:
    using SetupCallback = std::function<void(filament::Engine*, filament::View*, filament::Scene*)>;
    using CleanupCallback =
            std::function<void(filament::Engine*, filament::View*, filament::Scene*)>;
    using PreRenderCallback = std::function<void(filament::Engine*, filament::View*,
            filament::Scene*, filament::Renderer*)>;
    using PostRenderCallback = std::function<void(filament::Engine*, filament::View*,
            filament::Scene*, filament::Renderer*)>;
    using ImGuiCallback = std::function<void(filament::Engine*, filament::View*)>;
    using AnimCallback = std::function<void(filament::Engine*, filament::View*, double now)>;
    using ResizeCallback = std::function<void(filament::Engine*, filament::View*)>;
    using DropCallback = std::function<void(std::string_view)>;

    static FilamentApp& get();

    ~FilamentApp();

    void animate(AnimCallback animation) { mAnimation = animation; }

    void resize(ResizeCallback resize) { mResize = resize; }

    void setDropHandler(DropCallback handler) { mDropHandler = handler; }

    void run(const Config& config, SetupCallback setup, CleanupCallback cleanup,
            ImGuiCallback imgui = ImGuiCallback(),
            PreRenderCallback preRender = PreRenderCallback(),
            PostRenderCallback postRender = PostRenderCallback(), size_t width = 1024,
            size_t height = 640);

    void reconfigureCameras() { mReconfigureCameras = true; }

    filament::Material const* getDefaultMaterial() const noexcept { return mDefaultMaterial; }
    filament::Material const* getTransparentMaterial() const noexcept {
        return mTransparentMaterial;
    }
    IBL* getIBL() const noexcept { return mIBL.get(); }
    filament::Texture* getDirtTexture() const noexcept { return mDirt; }
    filament::View* getGuiView() const noexcept;
    filament::SwapChain* getPrimarySwapChain() const noexcept { return mPrimarySwapChain; }

    void close() { mClosed = true; }

    void setSidebarWidth(int width) {
        mCameraParams.sidebarWidth = width;
        mReconfigureCameras = true;
    }

    void setWindowTitle(const char* title) { mWindowTitle = title; }

    void setCameraFocalLength(float focalLength) {
        mCameraParams.focalLength = focalLength;
        mReconfigureCameras = true;
    }

    void setCameraNearFar(float near, float far) {
        mCameraParams.near = near;
        mCameraParams.far = far;
        mReconfigureCameras = true;
    }

    void addOffscreenView(filament::View* view) { mOffscreenViews.push_back(view); }

    size_t getSkippedFrameCount() const { return mSkippedFrames; }

    void loadIBL(std::string_view path);

    // debugging: enable/disable the froxel grid
    void setCameraFrustumEnabled(bool enabled) noexcept;
    void setDirectionalShadowFrustumEnabled(bool enabled) noexcept;
    void setFroxelGridEnabled(bool enabled) noexcept;
    bool isCameraFrustumEnabled() const noexcept;
    bool isDirectionalShadowFrustumEnabled() const noexcept;
    bool isFroxelGridEnabled() const noexcept;

    FilamentApp(const FilamentApp& rhs) = delete;
    FilamentApp(FilamentApp&& rhs) = delete;
    FilamentApp& operator=(const FilamentApp& rhs) = delete;
    FilamentApp& operator=(FilamentApp&& rhs) = delete;

    /**
     * Returns the path to the Filament root for loading assets. This is determined from the
     * executable folder, which allows users to launch samples from any folder.
     *
     * This takes into account multi-configuration CMake generators, like Visual Studio or Xcode,
     * that have different executable paths compared to single-configuration generators, like Ninja.
     */
    static const utils::Path& getRootAssetsPath();

private:
    FilamentApp();

    using CameraManipulator = filament::camutils::Manipulator<float>;

    static bool manipulatorKeyFromKeycode(filament::app::AppKey scancode,
            CameraManipulator::Key& key);

    class CView {
    public:
        CView(filament::Renderer& renderer, std::string name);
        virtual ~CView();

        void setCameraManipulator(CameraManipulator* cm);
        void setViewport(filament::Viewport const& viewport);
        void setCamera(filament::Camera* camera);
        bool intersects(ssize_t x, ssize_t y);

        virtual void mouseDown(int button, ssize_t x, ssize_t y);
        virtual void mouseUp(ssize_t x, ssize_t y);
        virtual void mouseMoved(ssize_t x, ssize_t y);
        virtual void mouseWheel(ssize_t x);
        virtual void keyDown(filament::app::AppKey scancode);
        virtual void keyUp(filament::app::AppKey scancode);

        filament::View const* getView() const { return view; }
        filament::View* getView() { return view; }
        CameraManipulator* getCameraManipulator() { return mCameraManipulator; }

    private:
        enum class Mode : uint8_t {
            NONE, ROTATE, TRACK
        };

        filament::Engine& engine;
        filament::Viewport mViewport;
        filament::View* view = nullptr;
        CameraManipulator* mCameraManipulator = nullptr;
        std::string mName;
    };

    class GodView : public CView {
    public:
        using CView::CView;
        void setGodCamera(filament::Camera* camera);
    };

    struct WindowCameraParams {
        int sidebarWidth = 0;
        float focalLength = 28.0f;
        float near = 0.1f;
        float far = 100.0f;

        bool operator==(WindowCameraParams const& params) const noexcept {
            return sidebarWidth == params.sidebarWidth && focalLength == params.focalLength &&
                   near == params.near && far == params.far;
        }

        bool operator!=(WindowCameraParams const& params) const noexcept {
            return !(*this == params);
        }
    };

public:
    class Window {
        friend class FilamentApp;
    public:
        using Handle = void*;
        virtual ~Window();

    private:
        Window(FilamentApp* filamentApp, const Config& config, std::string title,
                WindowCameraParams const& cameraParams, size_t w, size_t h);

        void mouseDown(int button, ssize_t x, ssize_t y);
        void mouseUp(ssize_t x, ssize_t y);
        void mouseMoved(ssize_t x, ssize_t y);
        void mouseWheel(ssize_t x);
        void keyDown(filament::app::AppKey scancode);
        void keyUp(filament::app::AppKey scancode);
        void resize(WindowCameraParams const& cameraParams);

        filament::Renderer* getRenderer() { return mRenderer; }
        filament::SwapChain* getSwapChain() { return mSwapChain; }

        void configureCamerasForWindow(WindowCameraParams const& camera);
        void fixupMouseCoordinatesForHdpi(ssize_t& x, ssize_t& y) const;

        filament::app::DisplayManager* const mDisplayManager = nullptr;
        filament::Engine* const mEngine = nullptr;
        Config mConfig;
        const bool mIsHeadless;

        Handle mWindow = nullptr;
        filament::Renderer* mRenderer = nullptr;
        filament::Engine::Backend mBackend;

        CameraManipulator* mMainCameraMan;
        CameraManipulator* mDebugCameraMan;
        filament::SwapChain* mSwapChain = nullptr;

        utils::Entity mCameraEntities[3];
        filament::Camera* mCameras[3] = { nullptr };
        filament::Camera* mMainCamera;
        filament::Camera* mDebugCamera;
        filament::Camera* mOrthoCamera;

        std::vector<std::unique_ptr<CView>> mViews;
        CView* mMainView;   // well, the main view
        CView* mUiView;     // the imgui ui
        CView* mDepthView;
        GodView* mGodView;  // the debug view with "god" camera
        CView* mOrthoView;  // directional shadow map view

        size_t mWidth = 0;
        size_t mHeight = 0;
        ssize_t mLastX = 0;
        ssize_t mLastY = 0;

        CView* mMouseEventTarget = nullptr;

        // Keep track of which view should receive a key's keyUp event.
        std::unordered_map<filament::app::AppKey, CView*> mKeyEventTarget;
    };

private:
    friend class Window;

    void loadIBL(const Config& config);
    void loadDirt(const Config& config);

    bool doFrame();
    void shutdown();

    filament::Engine* mEngine = nullptr;
    filament::Scene* mScene = nullptr;
    std::unique_ptr<IBL> mIBL;
    filament::SwapChain* mPrimarySwapChain = nullptr;
    filament::Texture* mDirt = nullptr;
    bool mClosed = false;
    double mTime = 0;

    filament::Material const* mDefaultMaterial = nullptr;
    filament::Material const* mTransparentMaterial = nullptr;
    filament::Material const* mDepthMaterial = nullptr;
    filament::MaterialInstance* mDepthMI = nullptr;
    std::unique_ptr<filagui::ImGuiHelper> mImGuiHelper;
    AnimCallback mAnimation;
    ResizeCallback mResize;
    DropCallback mDropHandler;
    size_t mSkippedFrames = 0;
    std::string mWindowTitle;
    std::vector<filament::View*> mOffscreenViews;
    WindowCameraParams mCameraParams{};
    bool mReconfigureCameras = false;
    uint8_t mFroxelInfoAge = 0x42;
    uint8_t mFroxelGridEnabled = 0;
    uint8_t mDirectionalShadowFrustumEnabled = 0x2;
    uint8_t mCameraFrustumEnabled = 0x2;

    filament::app::DisplayManager* mDisplayManager = nullptr;

    filament::backend::Platform* mVulkanPlatform = nullptr;
    filament::backend::Platform* mWebGPUPlatform = nullptr;

    std::unique_ptr<Window> mWindow;
    CleanupCallback mCleanupCallback;
    ImGuiCallback mImguiCallback {};
    PreRenderCallback mPreRender {};
    PostRenderCallback mPostRender {};
    bool mMousePressed[3] = { false };
    bool mIsSplitView = false;

    std::unique_ptr<Cube> mCameraCube;
    std::unique_ptr<Grid> mCameraGrid;

    // we can't cull the light-frustum because it's not applied a rigid transform
    // and currently, filament assumes that for culling
    std::vector<Cube> mLightmapCubes;
};

#endif // TNT_FILAMENT_SAMPLE_FILAMENTAPP_H
