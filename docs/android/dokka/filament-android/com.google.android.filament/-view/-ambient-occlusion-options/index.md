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
| [bentNormals](bent-normals.md) | [main]<br>open var [bentNormals](bent-normals.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables bent normals computation from AO, and specular AO |
| [bias](bias.md) | [main]<br>open var [bias](bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Self-occlusion bias in meters. |
| [bilateralThreshold](bilateral-threshold.md) | [main]<br>open var [bilateralThreshold](bilateral-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>depth distance that constitute an edge for filtering |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables or disables screen-space ambient occlusion |
| [gtaoConstThickness](gtao-const-thickness.md) | [main]<br>open var [gtaoConstThickness](gtao-const-thickness.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [gtaoLinearThickness](gtao-linear-thickness.md) | [main]<br>open var [gtaoLinearThickness](gtao-linear-thickness.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [gtaoSampleSliceCount](gtao-sample-slice-count.md) | [main]<br>open var [gtaoSampleSliceCount](gtao-sample-slice-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [gtaoSampleStepsPerSlice](gtao-sample-steps-per-slice.md) | [main]<br>open var [gtaoSampleStepsPerSlice](gtao-sample-steps-per-slice.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [gtaoThicknessHeuristic](gtao-thickness-heuristic.md) | [main]<br>open var [gtaoThicknessHeuristic](gtao-thickness-heuristic.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md) | [main]<br>open var [gtaoUseVisibilityBitmasks](gtao-use-visibility-bitmasks.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Ground Truth-base Ambient Occlusion (GTAO) options |
| [intensity](intensity.md) | [main]<br>open var [intensity](intensity.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Strength of the Ambient Occlusion effect. |
| [lowPassFilter](low-pass-filter.md) | [main]<br>open var [lowPassFilter](low-pass-filter.md): [View.QualityLevel](../-quality-level/index.md)<br>affects AO smoothness. |
| [minHorizonAngleRad](min-horizon-angle-rad.md) | [main]<br>open var [minHorizonAngleRad](min-horizon-angle-rad.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>min angle in radian to consider. |
| [power](power.md) | [main]<br>open var [power](power.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Controls ambient occlusion's contrast. |
| [quality](quality.md) | [main]<br>open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)<br>affects of samples used for AO and params for filtering |
| [radius](radius.md) | [main]<br>open var [radius](radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Ambient Occlusion radius in meters, between 0 and ~10. |
| [resolution](resolution.md) | [main]<br>open var [resolution](resolution.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>How each dimension of the AO buffer is scaled. |
| [ssctContactDistanceMax](ssct-contact-distance-max.md) | [main]<br>open var [ssctContactDistanceMax](ssct-contact-distance-max.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctDepthBias](ssct-depth-bias.md) | [main]<br>open var [ssctDepthBias](ssct-depth-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctDepthSlopeBias](ssct-depth-slope-bias.md) | [main]<br>open var [ssctDepthSlopeBias](ssct-depth-slope-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctEnabled](ssct-enabled.md) | [main]<br>open var [ssctEnabled](ssct-enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctIntensity](ssct-intensity.md) | [main]<br>open var [ssctIntensity](ssct-intensity.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctLightConeRad](ssct-light-cone-rad.md) | [main]<br>open var [ssctLightConeRad](ssct-light-cone-rad.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctLightDirection](ssct-light-direction.md) | [main]<br>open var [ssctLightDirection](ssct-light-direction.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctRayCount](ssct-ray-count.md) | [main]<br>open var [ssctRayCount](ssct-ray-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctSampleCount](ssct-sample-count.md) | [main]<br>open var [ssctSampleCount](ssct-sample-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [ssctShadowDistance](ssct-shadow-distance.md) | [main]<br>open var [ssctShadowDistance](ssct-shadow-distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Screen Space Cone Tracing (SSCT) options Ambient shadows from dominant light |
| [upsampling](upsampling.md) | [main]<br>open var [upsampling](upsampling.md): [View.QualityLevel](../-quality-level/index.md)<br>affects AO buffer upsampling quality |
