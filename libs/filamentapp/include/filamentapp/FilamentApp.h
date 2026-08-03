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

#include <filamentapp/FilamentApp2.h>

#include <utils/Path.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class [[deprecated("Use FilamentApp2 instead. Deadline: 2027/07/20")]] FilamentApp {
public:
    using SetupCallback = FilamentApp2::SetupCallback;
    using CleanupCallback = FilamentApp2::CleanupCallback;
    using PreRenderCallback = FilamentApp2::PreRenderCallback;
    using PostRenderCallback = FilamentApp2::PostRenderCallback;
    using ImGuiCallback = FilamentApp2::ImGuiCallback;
    using AnimCallback = FilamentApp2::AnimCallback;
    using ResizeCallback = FilamentApp2::ResizeCallback;
    using DropCallback = FilamentApp2::DropCallback;

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

    void reconfigureCameras();

    filament::Material const* getDefaultMaterial() const noexcept;
    filament::Material const* getTransparentMaterial() const noexcept;
    IBL* getIBL() const noexcept;
    filament::Texture* getDirtTexture() const noexcept;
    filament::View* getGuiView() const noexcept;
    filament::SwapChain* getPrimarySwapChain() const noexcept;

    void close();

    void onSurfaceCreated(void* nativeWindow);
    void onSurfaceChanged(int width, int height);
    void onSurfaceDestroyed();
    void onTouchEvent(int action, float x, float y);

    void setSidebarWidth(int width);
    void setWindowTitle(const char* title);
    void setCameraFocalLength(float focalLength);
    void setCameraNearFar(float near, float far);
    void addOffscreenView(filament::View* view);

    size_t getSkippedFrameCount() const;

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

    static const utils::Path& getRootAssetsPath();

private:
    FilamentApp();

    std::unique_ptr<FilamentApp2> mImpl;

    AnimCallback mAnimation;
    ResizeCallback mResize;
    DropCallback mDropHandler;
    int mSidebarWidth = 0;
    std::string mWindowTitle;
    float mCameraFocalLength = 0.0f;
    float mCameraNear = 0.0f;
    float mCameraFar = 0.0f;
    std::vector<filament::View*> mOffscreenViews;
};

#endif // TNT_FILAMENT_SAMPLE_FILAMENTAPP_H
