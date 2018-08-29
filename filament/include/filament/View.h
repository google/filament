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

//! \file

#ifndef TNT_FILAMENT_VIEW_H
#define TNT_FILAMENT_VIEW_H

#include <filament/Camera.h>
#include <filament/Color.h>
#include <filament/Viewport.h>
#include <filament/FilamentAPI.h>

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>

#include <math/vec2.h>
#include <math/vec3.h>

namespace filament {

class Camera;
class MaterialInstance;
class Scene;

/**
 * A View encompasses all the state needed for rendering a Scene.
 *
 * Renderer::render() operates on View objects. These View objects specify important parameters
 * such as:
 *  - The Scene
 *  - The Camera
 *  - The Viewport
 *  - Some rendering parameters
 *
 * \note
 * View instances are heavy objects that internally cache a lot of date needed for Rendering.
 * It is not advised for an application to use many View objects.
 *
 * For example, in a game, a View could be used for the main scene and another one for the
 * game's user interface. More View instances could be used for creating special effects (e.g.
 * a View is akin to a rendering pass).
 *
 *
 * @see Renderer, Scene, Camera
 */
class UTILS_PUBLIC View : public FilamentAPI {
public:
    using TargetBufferFlags = driver::TargetBufferFlags;

    /**
     * Dynamic resolution can be used to either reach a desired target frame rate
     * by lowering the resolution of a View, or to increase the quality when the
     * rendering is faster than the target frame rate.
     *
     * This structure can be used to specify the minimum scale factor used when
     * lowering the resolution of a View and the maximum scale factor used when
     * increasing the resolution for higher quality rendering. By scale factors
     * can be controlled on each X and Y axis independently. By default, all scale
     * factors are set to 1.0.
     *
     * enabled:   enable or disables dynamic resolution on a View
     * homogeneousScaling: by default the system scales the major axis first. Set this to true
     *                     to force homegeneous scaling.
     * scaleRate: rate at which the scale will change to reach the target frame rate
     *            This value can be computed as 1 / N, where N is the number of frames
     *            needed to reach 64% of the target scale factor.
     *            Higher values make the dynamic resolution react faster.
     * targetFrameTimeMilli: desired frame time in milliseconds
     * headRoomRatio: additional headroom for the GPU as a ratio of the targetFrameTime.
     *                Useful for taking into account constant costs like post-processing or
     *                GPU drivers on different platforms.
     * history:   History size. higher values, tend to filter more (clamped to 30)
     * minScale:  the minimum scale in X and Y this View should use
     * maxScale:  the maximum scale in X and Y this View should use
     *
     * \note
     * Dynamic resolution is only supported on platforms where the time to render
     * a frame can be measured accurately. Dynamic resolution is currently only
     * supported on Android.
     */
    struct DynamicResolutionOptions {
        DynamicResolutionOptions() = default;

        DynamicResolutionOptions(bool enabled, float scaleRate,
                math::float2 minScale, math::float2 maxScale)
                : minScale(minScale), maxScale(maxScale),
                  scaleRate(scaleRate), enabled(enabled) {
            // this one exists for backward compatibility
        }

        explicit DynamicResolutionOptions(bool enabled) : enabled(enabled) { }

        math::float2 minScale = math::float2(0.5f);     //!< minimum scale factors in x and y
        math::float2 maxScale = math::float2(1.0f);     //!< maximum scale factors in x and y
        float scaleRate = 0.125f;                       //!< rate at which the scale will change
        float targetFrameTimeMilli = 1000.0f / 60.0f;   //!< desired frame time, or budget.
        float headRoomRatio = 0.0f;                     //!< additional headroom for the GPU
        float reserved[5] = { 0.0f };                   //!< reserved fields, must be zero
        uint8_t history = 9;                            //!< history size
        bool enabled = false;                           //!< enable or disable dynamic resolution
        bool homogeneousScaling = false;                //!< set to true to force homogeneous scaling
    };

    enum class DepthPrepass : int8_t {
        DEFAULT = -1,
        DISABLED,
        ENABLED,
    };

    /**
     * Sets whether this view is rendered with or without a depth pre-pass.
     *
     * By default, the system picks the most appropriate strategy, this method lets the
     * application override that strategy.
     *
     * When the depth pre-pass is enabled, the renderer will first draw all objects in the
     * depth buffer from front to back, and then draw the objects again but sorted to minimize
     * state changes. With the depth pre-pass disabled, objects are draw only once, but it may
     * result in more state changes or more overdraw.
     *
     * The best strategy may depend on the scene and/or GPU.
     *
     * @param prepass   DepthPrepass::DEFAULT uses the most appropriate strategy,
     *                  DepthPrepass::DISABLED disables the depth pre-pass,
     *                  DepthPrepass::ENABLE enables the depth pre-pass.
     */
    void setDepthPrepass(DepthPrepass prepass) noexcept;

    /**
     * Sets the View's name. Only useful for debugging.
     * @param name Pointer to the View's name. The string is copied.
     */
    void setName(const char* name) noexcept;

    /**
     * Returns the View's name
     *
     * @return a pointer owned by the View instance to the View's name.
     *
     * @attention Do *not* free the pointer or modify its content.
     */
    const char* getName() const noexcept;

    /**
     * Set this View instance's Scene.
     *
     * @param scene Associate the specified Scene to this View. A Scene can be associated to
     *              several View instances.\n
     *              \p scene can be nullptr to dissociate the currently set Scene
     *              from this View.\n
     *              The View doesn't take ownership of the Scene pointer (which
     *              acts as a reference).
     *
     * @note
     *  There is no reference-counting.
     *  Make sure to dissociate a Scene from all Views before destroying it.
     */
    void setScene(Scene* scene);

    /**
     * Returns the Scene currently associated with this View.
     * @return A pointer to the Scene associated to this View. nullptr if no Scene is set.
     */
    Scene* getScene() noexcept;

    Scene const* getScene() const noexcept {
        return const_cast<View*>(this)->getScene();
    }

    /**
     * Set this View's Camera.
     *
     * @param camera    Associate the specified Camera to this View. A Camera can be associated to
     *                  several View instances.\n
     *                  \p camera can be nullptr to dissociate the currently set Camera from this
     *                  View.\n
     *                  The View doesn't take ownership of the Camera pointer (which
     *                  acts as a reference).
     *
     * @note
     *  There is no reference-counting.
     *  Make sure to dissociate a Camera from all Views before destroying it.
     */
    void setCamera(Camera* camera) noexcept;

    /**
     * Returns the Camera currently associated with this View.
     * @return A reference to the Camera associated to this View.
     */
    Camera& getCamera() noexcept;

    /**
     * Returns the Camera currently associated with this View.
     * @return A reference to the Camera associated to this View.
     */
    Camera const& getCamera() const noexcept {
        return const_cast<View*>(this)->getCamera();
    }

    /**
     * Set this View Viewport.
     *
     * The viewport specifies where the content of the View (i.e. the Scene) is rendered in
     * the render target. The Render target is automatically clipped to the Viewport.
     *
     * @param viewport  The Viewport to render the Scene into. The Viewport is a value-type, it is
     *                  therefore copied. The parameter can be discarded after this call returns.
     */
    void setViewport(Viewport const& viewport) noexcept;

    /**
     * Returns this View's viewport.
     * @return A constant reference to View's viewport.
     */
    Viewport const& getViewport() const noexcept;

    /**
     * Sets the color used to clear the Viewport when rendering this View.
     * @param clearColor The color to use to clear the Viewport.
     * @see setClearTargets
     */
    void setClearColor(LinearColorA const& clearColor) noexcept;

    /**
     * Returns the View clear color.
     * @return A constant reference to the View's clear color.
     */
    LinearColorA const& getClearColor() const noexcept;

    /**
     * Set which targets to clear (default: true, true, false)
     * @param color     Clear the color buffer. The color buffer is cleared with the color set in
     *                  setClearColor().
     * @param depth     Clear the depth buffer. The depth buffer is cleared with an implementation
     *                  defined value representing the farthest depth.
     *
     *                  @note
     *                  Generally the depth clear value is 1.0, but you shouldn't rely on this
     *                  because filament may use "inverted depth" in some situations.
     *
     *
     * @param stencil   Clear the stencil buffer. The stencil buffer is cleared with 0.
     *
     * @see setClearColor
     */
    void setClearTargets(bool color, bool depth, bool stencil) noexcept;

    /**
     * Set which layers are visible.
     *
     * Renderable objects can have one or several layers associated to them. Layers are
     * represented with an 8-bits bitmask, where each bit corresponds to a layer.
     * @see Renderable::setLayer().
     *
     * This call sets which of those layers are visible, Renderable in invisible layers won't be
     * rendered.
     *
     * @param select    a bitmask specifying which layer to set or clear using \p values.
     * @param values    a bitmask where each bit sets the visibility of the corresponding layer
     *                  (1: visible, 0: invisible), only layers in \p select are affected.
     *
     * @note By default all layers are visible.
     * @note This is a convenient way to quickly show or hide sets of Renderable objects.
     */
    void setVisibleLayers(uint8_t select, uint8_t values) noexcept;

    /**
     * Enable or disable shadow mapping. Enabled by default.
     *
     * @param enabled true enables shadow mapping, false disables it.
     *
     * @see Light::Builder::castShadows(),
     *      Renderable::Builder::receiveShadows(),
     *      Renderable::Builder::castShadows(),
     */
    void setShadowsEnabled(bool enabled) noexcept;

    /**
     * Specifies which buffers can be discarded before rendering.
     *
     * For performance reasons, the default is to discard all buffers, which is generally
     * correct when rendering a single view.
     *
     * However, when rendering a View on top of another one on the same render target,
     * it is necessary toindicate that the color buffer cannot be discarded.
     *
     * @param discard Buffers that need to be discarded before rendering.
     *
     * @note
     * In the future this API will also allow to set the render target.
     */
    void setRenderTarget(TargetBufferFlags discard = TargetBufferFlags::ALL) noexcept;

    /**
     * Sets how many samples are to be used for MSAA. Default is 1.
     *
     * @param count number of samples to use for multi-sampled anti-aliasing.\n
     *              0: treated as 1
     *              1: no anti-aliasing
     *              n: sample count. Effective sample could be different depending on the
     *                 GPU capabilities.
     *
     */
    void setSampleCount(uint8_t count = 1) noexcept;

    /**
     * Returns the sample count set by setSampleCount(). Effective sample could be different
     * @return value set by setSampleCount().
     */
    uint8_t getSampleCount() const noexcept;

    enum AntiAliasing : uint8_t {
        NONE = 0,
        FXAA = 1
    };

    /**
     * Enables or disables in the post-processing stage. Enabled by default.
     *
     * @param type FXAA for enabling, NONE for disabling anti-aliasing.
     */
    void setAntiAliasing(AntiAliasing type) noexcept;

    /**
     * Queries whether FXAA is enabled during the post-processing stage.
     *
     * @return true if FXAA is enabled, false if not.
     */
    AntiAliasing getAntiAliasing() const noexcept;

    /**
     * Sets the dynamic resolution options for this view. Dynamic resolution options
     * controls whether dynamic resolution is enabled, and if it is, how it behaves.
     *
     * @param options The dynamic resolution options to use on this view
     */
    void setDynamicResolutionOptions(DynamicResolutionOptions const& options) noexcept;

    /**
     * Returns the dynamic resolution options associated with this view.
     * @return value set by setDynamicResolutionOptions().
     */
    DynamicResolutionOptions getDynamicResolutionOptions() const noexcept;

    /**
     * Sets options relative to dynamic lighting for this view.
     *
     * @param zLightNear Distance from the camera where the lights are expected to shine.
     *                   This parameter can affect performance and is useful because depending
     *                   on the scene, lights that shine close to the camera may not be
     *                   visible -- in this case, using a larger value can improve performance.
     *                   e.g. when standing and looking straight, several meters of the ground
     *                   isn't visible and if lights are expected to shine there, there is no
     *                   point using a short zLightNear. (Default 5m).
     *
     * @param zLightFar Distance from the camera after which lighits are not expected to be visible.
     *                  Similarly to zLightNear, setting this value properly can improve
     *                  performance. (Default 100m).
     *
     *
     * Together zLightNear and zLightFar must be chosen so that the visible influence of lights
     * is spread between these two values.
     *
     */
    void setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept;

    /**
     * Enable or disable post processing. Enabled by default.
     *
     * Post-processing includes:
     *  - MSAA
     *  - Tone-mapping & gamma encoding
     *  - FXAA
     *  - Dynamic scaling
     *
     * For now, disabling post-processing forgoes color correctness as well as anti-aliasing and
     * should only be used experimentally (e.g., for UI overlays).
     *
     * @param enabled true enables post processing, false disables it.
     */
    void setPostProcessingEnabled(bool enabled) noexcept;

    bool isPostProcessingEnabled() const noexcept;

    // for debugging...

    //! debugging: allows to entirely disable culling. (culling enabled by default).
    void setCulling(bool culling) noexcept;

    //! debugging: returns whether culling is enabled.
    bool isCullingEnabled() const noexcept;

    //! debugging: sets the Camera used for rendering. It may be different from the culling camera.
    void setDebugCamera(Camera* camera) noexcept;

    //! debugging: returns a Camera from the point of view of *the* dominant directional light used for shadowing.
    Camera const* getDirectionalLightCamera() const noexcept;
};


} // namespace filament

#endif // TNT_FILAMENT_VIEW_H
