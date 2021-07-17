/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef VIEWER_SIMPLEVIEWER_H
#define VIEWER_SIMPLEVIEWER_H

#include <filament/Box.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <gltfio/Animator.h>
#include <gltfio/FilamentAsset.h>

#include <viewer/Settings.h>

#include <utils/Entity.h>
#include <utils/compiler.h>

#include <math/mat4.h>
#include <math/vec3.h>

namespace filagui {
    class ImGuiHelper;
}

namespace filament {
namespace viewer {

/**
 * \class SimpleViewer SimpleViewer.h viewer/SimpleViewer.h
 * \brief Manages the state for a simple glTF viewer with imgui controls and a tree view.
 *
 * This is a utility that can be used across multiple platforms, including web.
 *
 * \note If you don't need ImGui controls, there is no need to use this class, just use AssetLoader
 * instead.
 */
class UTILS_PUBLIC SimpleViewer {
public:
    using Animator = gltfio::Animator;
    using FilamentAsset = gltfio::FilamentAsset;
    using FilamentInstance =  gltfio::FilamentInstance;

    static constexpr int DEFAULT_SIDEBAR_WIDTH = 350;

    /**
     * Constructs a SimpleViewer that has a fixed association with the given Filament objects.
     *
     * Upon construction, the simple viewer may create some additional Filament objects (such as
     * light sources) that it owns.
     */
    SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
            int sidebarWidth = DEFAULT_SIDEBAR_WIDTH);

    /**
     * Destroys the SimpleViewer and any Filament entities that it owns.
     */
    ~SimpleViewer();

    /**
     * Adds the asset's ready-to-render entities into the scene.
     *
     * The viewer does not claim ownership over the asset or its entities. Clients should use
     * AssetLoader and ResourceLoader to load an asset before passing it in.
     *
     * @param asset The asset to view.
     * @param instanceToAnimate Optional instance from which to get the animator.
     */
    void populateScene(FilamentAsset* asset, FilamentInstance* instanceToAnimate = nullptr);

    /**
     * Removes the current asset from the viewer.
     *
     * This removes all the asset entities from the Scene, but does not destroy them.
     */
    void removeAsset();

    /**
     * Sets or changes the current scene's IBL to allow the UI manipulate it.
     */
    void setIndirectLight(filament::IndirectLight* ibl, filament::math::float3 const* sh3);

    /**
     * Applies the currently-selected glTF animation to the transformation hierarchy and updates
     * the bone matrices on all renderables.
     */
    void applyAnimation(double currentTime);

    /**
     * Constructs ImGui controls for the current frame and responds to everything that the user has
     * changed since the previous frame.
     *
     * If desired this can be used in conjunction with the filagui library, which allows clients to
     * render ImGui controls with Filament.
     */
    void updateUserInterface();

    /**
     * Alternative to updateUserInterface that uses an internal instance of ImGuiHelper.
     *
     * This utility method is designed for clients that do not want to manage their own instance of
     * ImGuiHelper (e.g., JavaScript clients).
     *
     * Behind the scenes this simply calls ImGuiHelper->render() and passes updateUserInterface into
     * its callback. Note that the first call might be slower since it requires the creation of the
     * internal ImGuiHelper instance.
     */
    void renderUserInterface(float timeStepInSeconds, filament::View* guiView, float pixelRatio);

    /**
     * Event-passing methods, useful only when SimpleViewer manages its own instance of ImGuiHelper.
     * The key codes used in these methods are just normal ASCII/ANSI codes.
     * @{
     */
    void mouseEvent(float mouseX, float mouseY, bool mouseButton, float mouseWheelY, bool control);
    void keyDownEvent(int keyCode);
    void keyUpEvent(int keyCode);
    void keyPressEvent(int charCode);
    /** @}*/

    /**
     * Retrieves the current width of the ImGui "window" which we are using as a sidebar.
     * Clients can monitor this value to adjust the size of the view.
     */
    int getSidebarWidth() const { return mSidebarWidth; }

    /**
     * Allows clients to inject custom UI.
     */
    void setUiCallback(std::function<void()> callback) { mCustomUI = callback; }

    /**
     * Draws the bounding box of each renderable.
     * Defaults to false.
     */
    void enableWireframe(bool b) { mEnableWireframe = b; }

    /**
     * Enables a built-in light source (useful for creating shadows).
     * Defaults to true.
     */
    void enableSunlight(bool b) { mSettings.lighting.enableSunlight = b; }

    /**
     * Enables dithering on the view.
     * Defaults to true.
     */
    void enableDithering(bool b) {
        mSettings.view.dithering = b ? Dithering::TEMPORAL : Dithering::NONE;
    }

    /**
     * Enables FXAA antialiasing in the post-process pipeline.
     * Defaults to true.
     */
    void enableFxaa(bool b) {
        mSettings.view.antiAliasing = b ? AntiAliasing::FXAA : AntiAliasing::NONE;
    }

    /**
     * Enables hardware-based MSAA antialiasing.
     * Defaults to true.
     */
    void enableMsaa(bool b) { mSettings.view.sampleCount = b ? 4 : 1; }

    /**
     * Enables screen-space ambient occlusion in the post-process pipeline.
     * Defaults to true.
     */
    void enableSSAO(bool b) { mSettings.view.ssao.enabled = b; }

    /**
     * Enables Bloom.
     * Defaults to true.
     */
    void enableBloom(bool bloom) { mSettings.view.bloom.enabled = bloom; }

    /**
     * Adjusts the intensity of the IBL.
     * See also filament::IndirectLight::setIntensity().
     * Defaults to 30000.0.
     */
    void setIBLIntensity(float brightness) { mSettings.lighting.iblIntensity = brightness; }

    /**
     * Updates the transform at the root node according to the autoScaleEnabled setting.
     */
    void updateRootTransform();

    /**
     * Gets a modifiable reference to stashed state.
     */
    Settings& getSettings() { return mSettings; }

    void stopAnimation() { mCurrentAnimation = 0; }

    int getCurrentCamera() const { return mCurrentCamera; }

private:
    void updateIndirectLight();

    // Immutable properties set from the constructor.
    filament::Engine* const mEngine;
    filament::Scene* const mScene;
    filament::View* const mView;
    const utils::Entity mSunlight;

    // Lazily instantiated fields.
    filagui::ImGuiHelper* mImGuiHelper = nullptr;

    // Properties that can be changed from the application.
    FilamentAsset* mAsset = nullptr;
    Animator* mAnimator = nullptr;
    filament::IndirectLight* mIndirectLight = nullptr;
    std::function<void()> mCustomUI;

    // Properties that can be changed from the UI.
    int mCurrentAnimation = 1;
    bool mResetAnimation = true;
    bool mEnableWireframe = false;
    int mVsmMsaaSamplesLog2 = 1;
    Settings mSettings;
    int mSidebarWidth;
    uint32_t mFlags;

    // 0 is the default "free camera". Additional cameras come from the gltf file (1-based index).
    int mCurrentCamera = 0;

    // Color grading UI state.
    float mToneMapPlot[1024];
    float mRangePlot[1024 * 3];
    float mCurvePlot[1024 * 3];
};

UTILS_PUBLIC
filament::math::mat4f fitIntoUnitCube(const filament::Aabb& bounds, float zoffset);

} // namespace viewer
} // namespace filament

#endif // VIEWER_SIMPLEVIEWER_H
