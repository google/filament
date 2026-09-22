//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[AmbientOcclusionOptions](index.md)

# AmbientOcclusionOptions

open class [AmbientOcclusionOptions](index.md)

Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT)

#### See also

| |
|---|
| [setAmbientOcclusionOptions](../set-ambient-occlusion-options.md) |

## Constructors

| | |
|---|---|
| [AmbientOcclusionOptions](-ambient-occlusion-options.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [AmbientOcclusionType](-ambient-occlusion-type/index.md) | [main]<br>enum [AmbientOcclusionType](-ambient-occlusion-type/index.md) |

## Properties

| Name | Summary |
|---|---|
| [aoType](ao-type.md) | [main]<br>open var [aoType](ao-type.md): [View.AmbientOcclusionOptions.AmbientOcclusionType](-ambient-occlusion-type/index.md)<br>Type of ambient occlusion algorithm. |
| [bentNormals](bent-normals.md) | [main]<br>open var [bentNormals](bent-normals.md): Boolean<br>enables bent normals computation from AO, and specular AO |
| [bias](bias.md) | [main]<br>open var [bias](bias.md): Float<br>Self-occlusion bias in meters. |
| [bilateralThreshold](bilateral-threshold.md) | [main]<br>open var [bilateralThreshold](bilateral-threshold.md): Float<br>depth distance that constitute an edge for filtering |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): Boolean<br>enables or disables screen-space ambient occlusion |
| [gtaoConstThickness](gtao-const-thickness.md) | [main]<br>open var [gtaoConstThickness](gtao-const-thickness.md): Float<br>constant thickness value of objects on the screen in world space. |
| [gtaoLinearThickness](gtao-linear-thickness.md) | [main]<br>open var [gtaoLinearThickness](gtao-linear-thickness.md): Boolean<br>Increase thickness with distance to maintain detail on distant surfaces. |
| [gtaoSampleSliceCount](gtao-sample-slice-count.md) | [main]<br>open var [gtaoSampleSliceCount](gtao-sample-slice-count.md): Int<br>of slices.<br>Higher value makes less noise. |
| [gtaoSampleStepsPerSlice](gtao-sample-steps-per-slice.md) | [main]<br>open var [gtaoSampleStepsPerSlice](gtao-sample-steps-per-slice.md): Int<br>of steps the radius is divided into for integration.<br>Higher value makes less bias. |
| [gtaoThicknessHeuristic](gtao-thickness-heuristic.md) | [main]<br>open var [gtaoThicknessHeuristic](gtao-thickness-heuristic.md): Float<br>thickness heuristic, should be closed to 0. |
| [gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md) | [main]<br>open var [gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md): Boolean<br>Enables or disables visibility bitmasks mode. |
| [intensity](intensity.md) | [main]<br>open var [intensity](intensity.md): Float<br>Strength of the Ambient Occlusion effect. |
| [lowPassFilter](low-pass-filter.md) | [main]<br>open var [lowPassFilter](low-pass-filter.md): [View.QualityLevel](../-quality-level/index.md)<br>affects AO smoothness. |
| [minHorizonAngleRad](min-horizon-angle-rad.md) | [main]<br>open var [minHorizonAngleRad](min-horizon-angle-rad.md): Float<br>min angle in radian to consider. |
| [power](power.md) | [main]<br>open var [power](power.md): Float<br>Controls ambient occlusion's contrast. |
| [quality](quality.md) | [main]<br>open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)<br>affects of samples used for AO and params for filtering |
| [radius](radius.md) | [main]<br>open var [radius](radius.md): Float<br>Ambient Occlusion radius in meters, between 0 and ~10. |
| [resolution](resolution.md) | [main]<br>open var [resolution](resolution.md): Float<br>How each dimension of the AO buffer is scaled. |
| [ssctContactDistanceMax](ssct-contact-distance-max.md) | [main]<br>open var [ssctContactDistanceMax](ssct-contact-distance-max.md): Float<br>max distance for contact |
| [ssctDepthBias](ssct-depth-bias.md) | [main]<br>open var [ssctDepthBias](ssct-depth-bias.md): Float<br>depth bias in world units (mitigate self shadowing) |
| [ssctDepthSlopeBias](ssct-depth-slope-bias.md) | [main]<br>open var [ssctDepthSlopeBias](ssct-depth-slope-bias.md): Float<br>depth slope bias (mitigate self shadowing) |
| [ssctEnabled](ssct-enabled.md) | [main]<br>open var [ssctEnabled](ssct-enabled.md): Boolean<br>enables or disables SSCT |
| [ssctIntensity](ssct-intensity.md) | [main]<br>open var [ssctIntensity](ssct-intensity.md): Float<br>intensity |
| [ssctLightConeRad](ssct-light-cone-rad.md) | [main]<br>open var [ssctLightConeRad](ssct-light-cone-rad.md): Float<br>full cone angle in radian, between 0 and pi/2 |
| [ssctLightDirection](ssct-light-direction.md) | [main]<br>open var [ssctLightDirection](ssct-light-direction.md): Array&lt;Float&gt;<br>light direction |
| [ssctRayCount](ssct-ray-count.md) | [main]<br>open var [ssctRayCount](ssct-ray-count.md): Int<br>of rays to trace, between 1 and 255 |
| [ssctSampleCount](ssct-sample-count.md) | [main]<br>open var [ssctSampleCount](ssct-sample-count.md): Int<br>tracing sample count, between 1 and 255 |
| [ssctShadowDistance](ssct-shadow-distance.md) | [main]<br>open var [ssctShadowDistance](ssct-shadow-distance.md): Float<br>how far shadows can be cast |
| [upsampling](upsampling.md) | [main]<br>open var [upsampling](upsampling.md): [View.QualityLevel](../-quality-level/index.md)<br>affects AO buffer upsampling quality |
