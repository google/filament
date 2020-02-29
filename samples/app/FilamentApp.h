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

#include <memory>
#include <map>
#include <string>
#include <vector>

#include <SDL.h>

#include <filament/Engine.h>
#include <filament/Viewport.h>

#include <camutils/Manipulator.h>

#include <utils/Path.h>

#include "Config.h"
#include "IBL.h"

namespace filament {
class Renderer;
class Scene;
class View;
} // namespace filament

namespace filagui {
class ImGuiHelper;
} // namespace filagui

class IBL;
class MeshAssimp;

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
    using DropCallback = std::function<void(std::string)>;

    static FilamentApp& get();

    ~FilamentApp();

    void animate(AnimCallback animation) { mAnimation = animation; }

    void setDropHandler(DropCallback handler) { mDropHandler = handler; }

    void run(const Config& config, SetupCallback setup, CleanupCallback cleanup,
            ImGuiCallback imgui = ImGuiCallback(), PreRenderCallback preRender = PreRenderCallback(),
            PostRenderCallback postRender = PostRenderCallback(),
            size_t width = 1024, size_t height = 640);

    filament::Material const* getDefaultMaterial() const noexcept { return mDefaultMaterial; }
    filament::Material const* getTransparentMaterial() const noexcept { return mTransparentMaterial; }
    IBL* getIBL() const noexcept { return mIBL.get(); }
    filament::Texture* getDirtTexture() const noexcept { return mDirt; }
    filament::View* getGuiView() const noexcept;

    void close() { mClosed = true; }

    void setSidebarWidth(int width) { mSidebarWidth = width; }
    void setWindowTitle(const char* title) { mWindowTitle = title; }
    float& getCameraFocalLength() { return mCameraFocalLength; }

    void addOffscreenView(filament::View* view) { mOffscreenViews.push_back(view); }

    size_t getSkippedFrameCount() const { return mSkippedFrames; }

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

        filament::View const* getView() const { return view; }
        filament::View* getView() { return view; }

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

    class Window {
        friend class FilamentApp;
    public:
        Window(FilamentApp* filamentApp, const Config& config,
               std::string title, size_t w, size_t h);
        virtual ~Window();

        void mouseDown(int button, ssize_t x, ssize_t y);
        void mouseUp(ssize_t x, ssize_t y);
        void mouseMoved(ssize_t x, ssize_t y);
        void mouseWheel(ssize_t x);
        void resize();

        filament::Renderer* getRenderer() { return mRenderer; }
        filament::SwapChain* getSwapChain() { return mSwapChain; }

        SDL_Window* getSDLWindow() {
            return mWindow;
        }

    private:
        void configureCamerasForWindow();
        void fixupMouseCoordinatesForHdpi(ssize_t& x, ssize_t& y) const;

        SDL_Window* mWindow = nullptr;
        FilamentApp* mFilamentApp = nullptr;
        filament::Renderer* mRenderer = nullptr;
        filament::Engine::Backend mBackend;

        CameraManipulator* mMainCameraMan;
        CameraManipulator* mDebugCameraMan;
        filament::SwapChain* mSwapChain = nullptr;

        filament::Camera* mCameras[4] = { nullptr };
        filament::Camera* mUiCamera;
        filament::Camera* mMainCamera;
        filament::Camera* mDebugCamera;
        filament::Camera* mOrthoCamera;

        std::vector<std::unique_ptr<CView>> mViews;
        CView* mMainView;
        CView* mUiView;
        CView* mDepthView;
        GodView* mGodView;
        CView* mOrthoView;

        size_t mWidth = 0;
        size_t mHeight = 0;
        ssize_t mLastX = 0;
        ssize_t mLastY = 0;
        CView* mEventTarget = nullptr;
    };

    friend class Window;
    void initSDL();

    void loadIBL(const Config& config);
    void loadDirt(const Config& config);

    filament::Engine* mEngine = nullptr;
    filament::Scene* mScene = nullptr;
    std::unique_ptr<IBL> mIBL;
    filament::Texture* mDirt = nullptr;
    bool mClosed = false;
    uint64_t mTime = 0;

    filament::Material const* mDefaultMaterial = nullptr;
    filament::Material const* mTransparentMaterial = nullptr;
    filament::Material const* mDepthMaterial = nullptr;
    filament::MaterialInstance* mDepthMI = nullptr;
    std::unique_ptr<filagui::ImGuiHelper> mImGuiHelper;
    AnimCallback mAnimation;
    DropCallback mDropHandler;
    int mSidebarWidth = 0;
    size_t mSkippedFrames = 0;
    std::string mWindowTitle;
    std::vector<filament::View*> mOffscreenViews;
    float mCameraFocalLength = 28.0f;
};

#endif // TNT_FILAMENT_SAMPLE_FILAMENTAPP_H
