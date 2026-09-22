//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)

# View

open class [View](index.md)

A View encompasses all the state needed for rendering a Scene. 

Renderer::render() operates on View objects. These View objects specify important parameters such as:

- The Scene
- The Camera
- The Viewport
- Some rendering parameters For example, in a game, a View could be used for the main scene and another one for the game's user interface. More View instances could be used for creating special effects (e.g. a View is akin to a rendering pass).

View instances are heavy objects that internally cache a lot of data needed for rendering. It is not advised for an application to use many View objects.

#### See also

| |
|---|
| [Renderer](../-renderer/index.md) |
| [Scene](../-scene/index.md) |
| [Camera](../-camera/index.md) |
| [RenderTarget](../-render-target/index.md) |

## Types

| Name | Summary |
|---|---|
| [AmbientOcclusion](-ambient-occlusion/index.md) | [main]<br>enum [AmbientOcclusion](-ambient-occlusion/index.md)<br>List of available ambient occlusion techniques |
| [AmbientOcclusionOptions](-ambient-occlusion-options/index.md) | [main]<br>open class [AmbientOcclusionOptions](-ambient-occlusion-options/index.md)<br>Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT) |
| [AntiAliasing](-anti-aliasing/index.md) | [main]<br>enum [AntiAliasing](-anti-aliasing/index.md)<br>List of available post-processing anti-aliasing techniques. |
| [BlendMode](-blend-mode/index.md) | [main]<br>enum [BlendMode](-blend-mode/index.md) |
| [BloomOptions](-bloom-options/index.md) | [main]<br>open class [BloomOptions](-bloom-options/index.md)<br>Options to control the bloom effect <br>- enabled: Enable or disable the bloom post-processing effect. Disabled by default. - levels: Number of successive blurs to achieve the blur effect, the minimum is 3 and the maximum is 12. This value together with resolution influences the spread of the blur effect. This value can be silently reduced to accommodate the original image size. - resolution: Resolution of bloom's minor axis. The minimum value is 2^levels and the the maximum is lower of the original resolution and 4096. This parameter is silently clamped to the minimum and maximum. It is highly recommended that this value be smaller than the target resolution after dynamic resolution is applied (horizontally and vertically). - strength: how much of the bloom is added to the original image. Between 0 and 1. - blendMode: Whether the bloom effect is purely additive (false) or mixed with the original image (true). - threshold: When enabled, a threshold at 1.0 is applied on the source image, this is useful for artistic reasons and is usually needed when a dirt texture is used. - dirt: A dirt/scratch/smudges texture (that can be RGB), which gets added to the bloom effect. Smudges are visible where bloom occurs. Threshold must be enabled for the dirt effect to work properly. - dirtStrength: Strength of the dirt texture. |
| [DepthOfFieldOptions](-depth-of-field-options/index.md) | [main]<br>open class [DepthOfFieldOptions](-depth-of-field-options/index.md)<br>Options to control Depth of Field (DoF) effect in the scene. |
| [Dithering](-dithering/index.md) | [main]<br>enum [Dithering](-dithering/index.md)<br>List of available post-processing dithering techniques. |
| [DynamicResolutionOptions](-dynamic-resolution-options/index.md) | [main]<br>open class [DynamicResolutionOptions](-dynamic-resolution-options/index.md)<br>Dynamic resolution can be used to either reach a desired target frame rate by lowering the resolution of a View, or to increase the quality when the rendering is faster than the target frame rate. |
| [FogOptions](-fog-options/index.md) | [main]<br>open class [FogOptions](-fog-options/index.md)<br>Options to control large-scale fog in the scene. |
| [GuardBandOptions](-guard-band-options/index.md) | [main]<br>open class [GuardBandOptions](-guard-band-options/index.md)<br>Options for the screen-space guard band. |
| [MultiSampleAntiAliasingOptions](-multi-sample-anti-aliasing-options/index.md) | [main]<br>open class [MultiSampleAntiAliasingOptions](-multi-sample-anti-aliasing-options/index.md)<br>Options for Multi-Sample Anti-aliasing (MSAA) |
| [OnPickCallback](-on-pick-callback/index.md) | [main]<br>interface [OnPickCallback](-on-pick-callback/index.md)<br>An interface to implement a custom class to receive results of picking queries. |
| [PickingQueryResult](-picking-query-result/index.md) | [main]<br>open class [PickingQueryResult](-picking-query-result/index.md)<br>Result of a picking query |
| [QualityLevel](-quality-level/index.md) | [main]<br>enum [QualityLevel](-quality-level/index.md)<br>Generic quality level. |
| [RenderQuality](-render-quality/index.md) | [main]<br>open class [RenderQuality](-render-quality/index.md)<br>Structure used to set the precision of the color buffer and related quality settings. |
| [ScreenSpaceReflectionsOptions](-screen-space-reflections-options/index.md) | [main]<br>open class [ScreenSpaceReflectionsOptions](-screen-space-reflections-options/index.md)<br>Options for Screen-space Reflections. |
| [ShadowType](-shadow-type/index.md) | [main]<br>enum [ShadowType](-shadow-type/index.md)<br>List of available shadow mapping techniques. |
| [SoftShadowOptions](-soft-shadow-options/index.md) | [main]<br>open class [SoftShadowOptions](-soft-shadow-options/index.md)<br>View-level options for PCSS Shadowing. |
| [StereoscopicOptions](-stereoscopic-options/index.md) | [main]<br>open class [StereoscopicOptions](-stereoscopic-options/index.md)<br>Options for stereoscopic (multi-eye) rendering. |
| [TargetBufferFlags](-target-buffer-flags/index.md) | [main]<br>enum [TargetBufferFlags](-target-buffer-flags/index.md)<br>Used to select buffers. |
| [TemporalAntiAliasingOptions](-temporal-anti-aliasing-options/index.md) | [main]<br>open class [TemporalAntiAliasingOptions](-temporal-anti-aliasing-options/index.md)<br>Options for Temporal Anti-aliasing (TAA) Most TAA parameters are extremely costly to change, as they will trigger the TAA post-process shaders to be recompiled. |
| [ToneMapping](-tone-mapping/index.md) | [main]<br>enum [~~ToneMapping~~](-tone-mapping/index.md)<br>List of available tone-mapping operators |
| [VignetteOptions](-vignette-options/index.md) | [main]<br>open class [VignetteOptions](-vignette-options/index.md)<br>Options to control the vignetting effect. |
| [VsmShadowOptions](-vsm-shadow-options/index.md) | [main]<br>open class [VsmShadowOptions](-vsm-shadow-options/index.md)<br>View-level options for VSM Shadowing. |

## Functions

| Name | Summary |
|---|---|
| [clearFrameHistory](clear-frame-history.md) | [main]<br>open fun [clearFrameHistory](clear-frame-history.md)(engine: [Engine](../-engine/index.md))<br>When certain temporal features are used (e.g.: TAA or Screen-space reflections), the view keeps a history of previous frame renders associated with the Renderer the view was last used with. |
| [getAmbientOcclusion](get-ambient-occlusion.md) | [main]<br>open fun [getAmbientOcclusion](get-ambient-occlusion.md)(): [View.AmbientOcclusion](-ambient-occlusion/index.md)<br>Queries the type of ambient occlusion active for this View. |
| [getAmbientOcclusionOptions](get-ambient-occlusion-options.md) | [main]<br>open fun [getAmbientOcclusionOptions](get-ambient-occlusion-options.md)(): [View.AmbientOcclusionOptions](-ambient-occlusion-options/index.md)<br>Gets the ambient occlusion options. |
| [getAntiAliasing](get-anti-aliasing.md) | [main]<br>open fun [getAntiAliasing](get-anti-aliasing.md)(): [View.AntiAliasing](-anti-aliasing/index.md)<br>Queries whether antialiasing is enabled during the post-processing stage. |
| [getBlendMode](get-blend-mode.md) | [main]<br>open fun [getBlendMode](get-blend-mode.md)(): [View.BlendMode](-blend-mode/index.md) |
| [getBloomOptions](get-bloom-options.md) | [main]<br>open fun [getBloomOptions](get-bloom-options.md)(): [View.BloomOptions](-bloom-options/index.md)<br>Queries the bloom options. |
| [getCamera](get-camera.md) | [main]<br>open fun [getCamera](get-camera.md)(): [Camera](../-camera/index.md)<br>Returns the Camera currently associated with this View. |
| [getColorGrading](get-color-grading.md) | [main]<br>open fun [getColorGrading](get-color-grading.md)(): [ColorGrading](../-color-grading/index.md)<br>Returns the color grading transforms currently associated to this view. |
| [getDepthOfFieldOptions](get-depth-of-field-options.md) | [main]<br>open fun [getDepthOfFieldOptions](get-depth-of-field-options.md)(): [View.DepthOfFieldOptions](-depth-of-field-options/index.md)<br>Queries the depth of field options. |
| [getDithering](get-dithering.md) | [main]<br>open fun [getDithering](get-dithering.md)(): [View.Dithering](-dithering/index.md)<br>Queries whether dithering is enabled during the post-processing stage. |
| [getDynamicResolutionOptions](get-dynamic-resolution-options.md) | [main]<br>open fun [getDynamicResolutionOptions](get-dynamic-resolution-options.md)(): [View.DynamicResolutionOptions](-dynamic-resolution-options/index.md)<br>Returns the dynamic resolution options associated with this view. |
| [getEffectiveGridSize](get-effective-grid-size.md) | [main]<br>open fun [getEffectiveGridSize](get-effective-grid-size.md)(): Double<br>Returns the effective grid size used for grid-based world origin snapping. |
| [getFogEntity](get-fog-entity.md) | [main]<br>open fun [getFogEntity](get-fog-entity.md)(): Int<br>Get an Entity representing the large scale fog object. |
| [getFogOptions](get-fog-options.md) | [main]<br>open fun [getFogOptions](get-fog-options.md)(): [View.FogOptions](-fog-options/index.md)<br>Queries the fog options. |
| [getGridSize](get-grid-size.md) | [main]<br>open fun [getGridSize](get-grid-size.md)(): Double<br>Returns the grid size used for grid-based world origin snapping. |
| [getGuardBandOptions](get-guard-band-options.md) | [main]<br>open fun [getGuardBandOptions](get-guard-band-options.md)(): [View.GuardBandOptions](-guard-band-options/index.md)<br>Returns screen-space guard band options. |
| [getLastDynamicResolutionScale](get-last-dynamic-resolution-scale.md) | [main]<br>open fun [getLastDynamicResolutionScale](get-last-dynamic-resolution-scale.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Returns the last dynamic resolution scale factor used by this view. |
| [getMaterialGlobal](get-material-global.md) | [main]<br>open fun [getMaterialGlobal](get-material-global.md)(index: Int, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Get the value of the material global variables. |
| [getMultiSampleAntiAliasingOptions](get-multi-sample-anti-aliasing-options.md) | [main]<br>open fun [getMultiSampleAntiAliasingOptions](get-multi-sample-anti-aliasing-options.md)(): [View.MultiSampleAntiAliasingOptions](-multi-sample-anti-aliasing-options/index.md)<br>Returns multi-sample antialiasing options. |
| [getName](get-name.md) | [main]<br>open fun [getName](get-name.md)(): [String](https://developer.android.com/reference/kotlin/java/lang/String.html)<br>Returns the View's name |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getRenderQuality](get-render-quality.md) | [main]<br>open fun [getRenderQuality](get-render-quality.md)(): [View.RenderQuality](-render-quality/index.md)<br>Returns the render quality used by this view. |
| [getRenderTarget](get-render-target.md) | [main]<br>open fun [getRenderTarget](get-render-target.md)(): [RenderTarget](../-render-target/index.md)<br>Gets the offscreen render target associated with this view. |
| [getSampleCount](get-sample-count.md) | [main]<br>open fun [getSampleCount](get-sample-count.md)(): Int<br>Returns the sample count set by setSampleCount(). |
| [getScene](get-scene.md) | [main]<br>open fun [getScene](get-scene.md)(): [Scene](../-scene/index.md)<br>Returns the Scene currently associated with this View. |
| [getScreenSpaceReflectionsOptions](get-screen-space-reflections-options.md) | [main]<br>open fun [getScreenSpaceReflectionsOptions](get-screen-space-reflections-options.md)(): [View.ScreenSpaceReflectionsOptions](-screen-space-reflections-options/index.md)<br>Returns screen-space reflections options. |
| [getShadowType](get-shadow-type.md) | [main]<br>open fun [getShadowType](get-shadow-type.md)(): [View.ShadowType](-shadow-type/index.md)<br>Returns the shadow mapping technique used by this View. |
| [getSoftShadowOptions](get-soft-shadow-options.md) | [main]<br>open fun [getSoftShadowOptions](get-soft-shadow-options.md)(): [View.SoftShadowOptions](-soft-shadow-options/index.md)<br>Returns the soft shadowing options associated with this View. |
| [getStereoscopicOptions](get-stereoscopic-options.md) | [main]<br>open fun [getStereoscopicOptions](get-stereoscopic-options.md)(): [View.StereoscopicOptions](-stereoscopic-options/index.md)<br>Returns the stereoscopic options associated with this View. |
| [getTemporalAntiAliasingOptions](get-temporal-anti-aliasing-options.md) | [main]<br>open fun [getTemporalAntiAliasingOptions](get-temporal-anti-aliasing-options.md)(): [View.TemporalAntiAliasingOptions](-temporal-anti-aliasing-options/index.md)<br>Returns temporal antialiasing options. |
| [getToneMapping](get-tone-mapping.md) | [main]<br>open fun [~~getToneMapping~~](get-tone-mapping.md)(): [View.ToneMapping](-tone-mapping/index.md) |
| [getViewport](get-viewport.md) | [main]<br>open fun [getViewport](get-viewport.md)(): [Viewport](../-viewport/index.md)<br>Returns the rectangular region that gets rendered to. |
| [getVignetteOptions](get-vignette-options.md) | [main]<br>open fun [getVignetteOptions](get-vignette-options.md)(): [View.VignetteOptions](-vignette-options/index.md)<br>Queries the vignette options. |
| [getVisibleLayers](get-visible-layers.md) | [main]<br>open fun [getVisibleLayers](get-visible-layers.md)(): Int<br>Get the visible layers. |
| [getVisibleRenderableCount](get-visible-renderable-count.md) | [main]<br>open fun [getVisibleRenderableCount](get-visible-renderable-count.md)(): Int<br>Returns the most recent number of visible renderables for the current Scene as calculated the last time Renderer::render() was called with this View and Scene. |
| [getVsmShadowOptions](get-vsm-shadow-options.md) | [main]<br>open fun [getVsmShadowOptions](get-vsm-shadow-options.md)(): [View.VsmShadowOptions](-vsm-shadow-options/index.md)<br>Returns the VSM shadowing options associated with this View. |
| [hasCamera](has-camera.md) | [main]<br>open fun [hasCamera](has-camera.md)(): Boolean<br>Returns whether a Camera is set. |
| [isChannelDepthClearEnabled](is-channel-depth-clear-enabled.md) | [main]<br>open fun [isChannelDepthClearEnabled](is-channel-depth-clear-enabled.md)(channel: Int): Boolean |
| [isFrontFaceWindingInverted](is-front-face-winding-inverted.md) | [main]<br>open fun [isFrontFaceWindingInverted](is-front-face-winding-inverted.md)(): Boolean<br>Returns true if the winding order of front faces is inverted. |
| [isFrustumCullingEnabled](is-frustum-culling-enabled.md) | [main]<br>open fun [isFrustumCullingEnabled](is-frustum-culling-enabled.md)(): Boolean<br>debugging: returns whether frustum culling is enabled. |
| [isPostProcessingEnabled](is-post-processing-enabled.md) | [main]<br>open fun [isPostProcessingEnabled](is-post-processing-enabled.md)(): Boolean<br>Returns true if post-processing is enabled. |
| [isScreenSpaceRefractionEnabled](is-screen-space-refraction-enabled.md) | [main]<br>open fun [isScreenSpaceRefractionEnabled](is-screen-space-refraction-enabled.md)(): Boolean |
| [isShadowingEnabled](is-shadowing-enabled.md) | [main]<br>open fun [isShadowingEnabled](is-shadowing-enabled.md)(): Boolean |
| [isStencilBufferEnabled](is-stencil-buffer-enabled.md) | [main]<br>open fun [isStencilBufferEnabled](is-stencil-buffer-enabled.md)(): Boolean<br>Returns true if the stencil buffer is enabled. |
| [isTransparentPickingEnabled](is-transparent-picking-enabled.md) | [main]<br>open fun [isTransparentPickingEnabled](is-transparent-picking-enabled.md)(): Boolean<br>Returns true if transparent picking is enabled. |
| [pick](pick.md) | [main]<br>open fun [pick](pick.md)(x: Int, y: Int, handler: Any, callback: [View.OnPickCallback](-on-pick-callback/index.md))<br>Creates a picking query. |
| [setAmbientOcclusion](set-ambient-occlusion.md) | [main]<br>open fun [setAmbientOcclusion](set-ambient-occlusion.md)(ambientOcclusion: [View.AmbientOcclusion](-ambient-occlusion/index.md))<br>Activates or deactivates ambient occlusion. |
| [setAmbientOcclusionOptions](set-ambient-occlusion-options.md) | [main]<br>open fun [setAmbientOcclusionOptions](set-ambient-occlusion-options.md)(options: [View.AmbientOcclusionOptions](-ambient-occlusion-options/index.md))<br>Sets ambient occlusion options. |
| [setAntiAliasing](set-anti-aliasing.md) | [main]<br>open fun [setAntiAliasing](set-anti-aliasing.md)(type: [View.AntiAliasing](-anti-aliasing/index.md))<br>Enables or disables antialiasing in the post-processing stage. |
| [setBlendMode](set-blend-mode.md) | [main]<br>open fun [setBlendMode](set-blend-mode.md)(blendMode: [View.BlendMode](-blend-mode/index.md))<br>Sets the blending mode used to draw the view into the SwapChain. |
| [setBloomOptions](set-bloom-options.md) | [main]<br>open fun [setBloomOptions](set-bloom-options.md)(options: [View.BloomOptions](-bloom-options/index.md))<br>Enables or disables bloom in the post-processing stage. |
| [setCamera](set-camera.md) | [main]<br>open fun [setCamera](set-camera.md)(camera: [Camera](../-camera/index.md))<br>Sets this View's Camera. |
| [setChannelDepthClearEnabled](set-channel-depth-clear-enabled.md) | [main]<br>open fun [setChannelDepthClearEnabled](set-channel-depth-clear-enabled.md)(channel: Int, enabled: Boolean)<br>Sets whether a channel must clear the depth buffer before all primitives are rendered. |
| [setColorGrading](set-color-grading.md) | [main]<br>open fun [setColorGrading](set-color-grading.md)(colorGrading: [ColorGrading](../-color-grading/index.md))<br>Sets this View's color grading transforms. |
| [setDebugCamera](set-debug-camera.md) | [main]<br>open fun [setDebugCamera](set-debug-camera.md)(camera: [Camera](../-camera/index.md))<br>debugging: sets the Camera used for rendering. |
| [setDepthOfFieldOptions](set-depth-of-field-options.md) | [main]<br>open fun [setDepthOfFieldOptions](set-depth-of-field-options.md)(options: [View.DepthOfFieldOptions](-depth-of-field-options/index.md))<br>Enables or disables Depth of Field. |
| [setDithering](set-dithering.md) | [main]<br>open fun [setDithering](set-dithering.md)(dithering: [View.Dithering](-dithering/index.md))<br>Enables or disables dithering in the post-processing stage. |
| [setDynamicLightingOptions](set-dynamic-lighting-options.md) | [main]<br>open fun [setDynamicLightingOptions](set-dynamic-lighting-options.md)(zLightNear: Float, zLightFar: Float)<br>Sets options relative to dynamic lighting for this view. |
| [setDynamicResolutionOptions](set-dynamic-resolution-options.md) | [main]<br>open fun [setDynamicResolutionOptions](set-dynamic-resolution-options.md)(options: [View.DynamicResolutionOptions](-dynamic-resolution-options/index.md))<br>Sets the dynamic resolution options for this view. |
| [setFogOptions](set-fog-options.md) | [main]<br>open fun [setFogOptions](set-fog-options.md)(options: [View.FogOptions](-fog-options/index.md))<br>Enables or disables fog. |
| [setFrontFaceWindingInverted](set-front-face-winding-inverted.md) | [main]<br>open fun [setFrontFaceWindingInverted](set-front-face-winding-inverted.md)(inverted: Boolean)<br>Inverts the winding order of front faces. |
| [setFrustumCullingEnabled](set-frustum-culling-enabled.md) | [main]<br>open fun [setFrustumCullingEnabled](set-frustum-culling-enabled.md)(culling: Boolean)<br>debugging: allows to entirely disable frustum culling. |
| [setGridSize](set-grid-size.md) | [main]<br>open fun [setGridSize](set-grid-size.md)(size: Double)<br>Sets the grid size for grid-based world origin snapping. |
| [setGuardBandOptions](set-guard-band-options.md) | [main]<br>open fun [setGuardBandOptions](set-guard-band-options.md)(options: [View.GuardBandOptions](-guard-band-options/index.md))<br>Enables or disable screen-space guard band. |
| [setLayerEnabled](set-layer-enabled.md) | [main]<br>open fun [setLayerEnabled](set-layer-enabled.md)(layer: Int, enabled: Boolean)<br>Helper function to enable or disable a visibility layer. |
| [setMaterialGlobal](set-material-global.md) | [main]<br>open fun [setMaterialGlobal](set-material-global.md)(index: Int, value: Array&lt;Float&gt;)<br>open fun [setMaterialGlobal](set-material-global.md)(index: Int, valuex: Float, valuey: Float, valuez: Float, valuew: Float)<br>Set the value of material global variables. |
| [setMultiSampleAntiAliasingOptions](set-multi-sample-anti-aliasing-options.md) | [main]<br>open fun [setMultiSampleAntiAliasingOptions](set-multi-sample-anti-aliasing-options.md)(options: [View.MultiSampleAntiAliasingOptions](-multi-sample-anti-aliasing-options/index.md))<br>Enables or disable multi-sample antialiasing (MSAA). |
| [setName](set-name.md) | [main]<br>open fun [setName](set-name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html))<br>Sets the View's name. |
| [setPostProcessingEnabled](set-post-processing-enabled.md) | [main]<br>open fun [setPostProcessingEnabled](set-post-processing-enabled.md)(enabled: Boolean)<br>Enables or disables post-processing. |
| [setRenderQuality](set-render-quality.md) | [main]<br>open fun [setRenderQuality](set-render-quality.md)(renderQuality: [View.RenderQuality](-render-quality/index.md))<br>Sets the rendering quality for this view. |
| [setRenderTarget](set-render-target.md) | [main]<br>open fun [setRenderTarget](set-render-target.md)(renderTarget: [RenderTarget](../-render-target/index.md))<br>Specifies an offscreen render target to render into. |
| [setSampleCount](set-sample-count.md) | [main]<br>open fun [setSampleCount](set-sample-count.md)()<br>open fun [setSampleCount](set-sample-count.md)(count: Int)<br>Sets how many samples are to be used for MSAA in the post-process stage. |
| [setScene](set-scene.md) | [main]<br>open fun [setScene](set-scene.md)(scene: [Scene](../-scene/index.md))<br>Set this View instance's Scene. |
| [setScreenSpaceReflectionsOptions](set-screen-space-reflections-options.md) | [main]<br>open fun [setScreenSpaceReflectionsOptions](set-screen-space-reflections-options.md)(options: [View.ScreenSpaceReflectionsOptions](-screen-space-reflections-options/index.md))<br>Enables or disable screen-space reflections. |
| [setScreenSpaceRefractionEnabled](set-screen-space-refraction-enabled.md) | [main]<br>open fun [setScreenSpaceRefractionEnabled](set-screen-space-refraction-enabled.md)(enabled: Boolean)<br>Enables or disables screen space refraction. |
| [setShadowingEnabled](set-shadowing-enabled.md) | [main]<br>open fun [setShadowingEnabled](set-shadowing-enabled.md)(enabled: Boolean)<br>Enables or disables shadow mapping. |
| [setShadowType](set-shadow-type.md) | [main]<br>open fun [setShadowType](set-shadow-type.md)(shadow: [View.ShadowType](-shadow-type/index.md)) |
| [setSoftShadowOptions](set-soft-shadow-options.md) | [main]<br>open fun [setSoftShadowOptions](set-soft-shadow-options.md)(options: [View.SoftShadowOptions](-soft-shadow-options/index.md))<br>Sets soft shadowing options that apply across the entire View. |
| [setStencilBufferEnabled](set-stencil-buffer-enabled.md) | [main]<br>open fun [setStencilBufferEnabled](set-stencil-buffer-enabled.md)(enabled: Boolean)<br>Enables use of the stencil buffer. |
| [setStereoscopicOptions](set-stereoscopic-options.md) | [main]<br>open fun [setStereoscopicOptions](set-stereoscopic-options.md)(options: [View.StereoscopicOptions](-stereoscopic-options/index.md))<br>Sets the stereoscopic rendering options for this view. |
| [setTemporalAntiAliasingOptions](set-temporal-anti-aliasing-options.md) | [main]<br>open fun [setTemporalAntiAliasingOptions](set-temporal-anti-aliasing-options.md)(options: [View.TemporalAntiAliasingOptions](-temporal-anti-aliasing-options/index.md))<br>Enables or disable temporal antialiasing (TAA). |
| [setToneMapping](set-tone-mapping.md) | [main]<br>open fun [~~setToneMapping~~](set-tone-mapping.md)(type: [View.ToneMapping](-tone-mapping/index.md)) |
| [setTransparentPickingEnabled](set-transparent-picking-enabled.md) | [main]<br>open fun [setTransparentPickingEnabled](set-transparent-picking-enabled.md)(enabled: Boolean)<br>Enables or disables transparent picking. |
| [setViewport](set-viewport.md) | [main]<br>open fun [setViewport](set-viewport.md)(viewport: [Viewport](../-viewport/index.md))<br>Sets the rectangular region to render to. |
| [setVignetteOptions](set-vignette-options.md) | [main]<br>open fun [setVignetteOptions](set-vignette-options.md)(options: [View.VignetteOptions](-vignette-options/index.md))<br>Enables or disables the vignetted effect in the post-processing stage. |
| [setVisibleLayers](set-visible-layers.md) | [main]<br>open fun [setVisibleLayers](set-visible-layers.md)(select: Int, values: Int)<br>Sets which layers are visible. |
| [setVsmShadowOptions](set-vsm-shadow-options.md) | [main]<br>open fun [setVsmShadowOptions](set-vsm-shadow-options.md)(options: [View.VsmShadowOptions](-vsm-shadow-options/index.md))<br>Sets VSM shadowing options that apply across the entire View. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [View](index.md) |
