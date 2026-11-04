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

#include <filament/Engine.h>
#include <filament/View.h>
#include <filamentapp/DisplayManager.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/HeadlessDisplayManager.h>

#if defined(FILAMENTAPP_HAS_WEB_UI)
#include "filamentapp/HtmlDisplayManager.h"
#endif // defined(FILAMENTAPP_HAS_WEB_UI)

#ifdef FILAMENTAPP_HAS_SDL
#include "filamentapp/SDLDisplayManager.h"
#endif // defined(FILAMENTAPP_HAS_SDL)

#include <utils/Log.h>
#include <utils/Panic.h>

FilamentApp& FilamentApp::get() {
    static FilamentApp app;
    return app;
}

FilamentApp::FilamentApp() = default;

FilamentApp::~FilamentApp() = default;

void FilamentApp::run(const Config& config, SetupCallback setup, CleanupCallback cleanup,
        ImGuiCallback imgui, PreRenderCallback preRender,
        PostRenderCallback postRender, size_t width, size_t height) {

    if (!mDisplayManager) {
        // Config::headless is deprecated, and is honored here by choosing a display manager that
        // reports isHeadless(). WEB is considered first because HtmlDisplayManager is headless in
        // its own right and is the more specific request of the two.
        if (config.displayManager == Config::DisplayManager::WEB) {
#if defined(FILAMENTAPP_HAS_WEB_UI)
            mDisplayManager = std::make_unique<filament::app::HtmlDisplayManager>();
#endif
        } else if (!config.headless && config.displayManager == Config::DisplayManager::SDL) {
#ifdef FILAMENTAPP_HAS_SDL
            mDisplayManager = std::make_unique<filament::app::SDLDisplayManager>(config.backend);
#endif
        }

        if (!mDisplayManager) {
            // This is reached either because headless was requested or because the requested
            // display manager is unavailable in this build. A display manager is mandatory, since
            // FilamentApp2::Builder::build() fails its precondition without one, so headless serves
            // both cases: it keeps a deprecated Config::headless request working, and it turns an
            // unavailable manager into an offscreen run rather than an abort.
            if (!config.headless) {
                utils::slog.w << "FilamentApp: requested display manager is unavailable, "
                                 "falling back to headless."
                              << utils::io::endl;
            }
            mDisplayManager = std::make_unique<filament::app::HeadlessDisplayManager>();
        }
    }

    mImpl = FilamentApp2::Builder()
                    .title(mWindowTitle.empty() ? config.title : mWindowTitle)
                    .backend(config.backend)
                    .size(width, height)
                    .setup(setup)
                    .cleanup(cleanup)
                    .imgui(imgui)
                    .preRender(preRender)
                    .postRender(postRender)
                    .animation(mAnimation)
                    .resize(mResize)
                    .dropHandler(mDropHandler)
                    .iblDirectory(config.iblDirectory)
                    .dirt(config.dirt)
                    .splitView(config.splitView)
                    .featureLevel(config.featureLevel)
                    .cameraMode(config.cameraMode)
                    .resizeable(config.resizeable)
                    .stereoscopicEyeCount(config.stereoscopicEyeCount)
                    .vulkanGPUHint(config.vulkanGPUHint)
                    .forcedWebGPUBackend(
                            static_cast<FilamentApp2::WebGPUBackend>(config.forcedWebGPUBackend))
                    .displayManager(mDisplayManager.get())
                    .asynchronousMode(config.asynchronousMode)
                    .build();

    if (mSidebarWidth != 0) mImpl->setSidebarWidth(mSidebarWidth);
    if (mCameraFocalLength != 0.0f) mImpl->setCameraFocalLength(mCameraFocalLength);
    if (mCameraNear != 0.0f || mCameraFar != 0.0f) mImpl->setCameraNearFar(mCameraNear, mCameraFar);
    for (auto* view : mOffscreenViews) mImpl->addOffscreenView(view);

    mImpl->run();
}

void FilamentApp::reconfigureCameras() {
    if (mImpl) mImpl->reconfigureCameras();
}

filament::Material const* FilamentApp::getDefaultMaterial() const noexcept {
    return mImpl ? mImpl->getDefaultMaterial() : nullptr;
}

filament::Material const* FilamentApp::getTransparentMaterial() const noexcept {
    return mImpl ? mImpl->getTransparentMaterial() : nullptr;
}

IBL* FilamentApp::getIBL() const noexcept {
    return mImpl ? mImpl->getIBL() : nullptr;
}

filament::Texture* FilamentApp::getDirtTexture() const noexcept {
    return mImpl ? mImpl->getDirtTexture() : nullptr;
}

filament::View* FilamentApp::getGuiView() const noexcept {
    return mImpl ? mImpl->getGuiView() : nullptr;
}

filament::SwapChain* FilamentApp::getPrimarySwapChain() const noexcept {
    return mImpl ? mImpl->getPrimarySwapChain() : nullptr;
}

void FilamentApp::close() {
    if (mImpl) mImpl->close();
}

void FilamentApp::setSidebarWidth(int width) {
    mSidebarWidth = width;
    if (mImpl) mImpl->setSidebarWidth(width);
}

void FilamentApp::setWindowTitle(const char* title) {
    mWindowTitle = utils::CString(title ? title : "");
    // mImpl doesn't have setWindowTitle; we handle it via Builder if called before run.
}

void FilamentApp::setCameraFocalLength(float focalLength) {
    mCameraFocalLength = focalLength;
    if (mImpl) mImpl->setCameraFocalLength(focalLength);
}

void FilamentApp::setCameraNearFar(float near, float far) {
    mCameraNear = near;
    mCameraFar = far;
    if (mImpl) mImpl->setCameraNearFar(near, far);
}

void FilamentApp::addOffscreenView(filament::View* view) {
    mOffscreenViews.push_back(view);
    if (mImpl) mImpl->addOffscreenView(view);
}

size_t FilamentApp::getSkippedFrameCount() const {
    return mImpl ? mImpl->getSkippedFrameCount() : 0;
}

void FilamentApp::loadIBL(std::string_view path) {
    if (mImpl) mImpl->loadIBL(path);
}

void FilamentApp::setCameraFrustumEnabled(bool enabled) noexcept {
    if (mImpl) mImpl->setCameraFrustumEnabled(enabled);
}

void FilamentApp::setDirectionalShadowFrustumEnabled(bool enabled) noexcept {
    if (mImpl) mImpl->setDirectionalShadowFrustumEnabled(enabled);
}

void FilamentApp::setFroxelGridEnabled(bool enabled) noexcept {
    if (mImpl) mImpl->setFroxelGridEnabled(enabled);
}

bool FilamentApp::isCameraFrustumEnabled() const noexcept {
    return mImpl ? mImpl->isCameraFrustumEnabled() : false;
}

bool FilamentApp::isDirectionalShadowFrustumEnabled() const noexcept {
    return mImpl ? mImpl->isDirectionalShadowFrustumEnabled() : false;
}

bool FilamentApp::isFroxelGridEnabled() const noexcept {
    return mImpl ? mImpl->isFroxelGridEnabled() : false;
}
