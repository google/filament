//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[TemporalAntiAliasingOptions](index.md)

# TemporalAntiAliasingOptions

open class [TemporalAntiAliasingOptions](index.md)

Options for Temporal Anti-aliasing (TAA) Most TAA parameters are extremely costly to change, as they will trigger the TAA post-process shaders to be recompiled. 

These options should be changed or set during initialization. `filterWidth`, `feedback` and `jitterPattern`, however, can be changed at any time.

`feedback` of 0.1 effectively accumulates a maximum of 19 samples in steady state. see &quot;A Survey of Temporal Antialiasing Techniques&quot; by Lei Yang and all for more information.

#### See also

| |
|---|
| [setTemporalAntiAliasingOptions](../set-temporal-anti-aliasing-options.md) |

## Constructors

| | |
|---|---|
| [TemporalAntiAliasingOptions](-temporal-anti-aliasing-options.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [BoxClipping](-box-clipping/index.md) | [main]<br>enum [BoxClipping](-box-clipping/index.md) |
| [BoxType](-box-type/index.md) | [main]<br>enum [BoxType](-box-type/index.md) |
| [JitterPattern](-jitter-pattern/index.md) | [main]<br>enum [JitterPattern](-jitter-pattern/index.md) |

## Properties

| Name | Summary |
|---|---|
| [boxClipping](box-clipping.md) | [main]<br>open var [boxClipping](box-clipping.md): [View.TemporalAntiAliasingOptions.BoxClipping](-box-clipping/index.md)<br>clipping algorithm |
| [boxType](box-type.md) | [main]<br>open var [boxType](box-type.md): [View.TemporalAntiAliasingOptions.BoxType](-box-type/index.md)<br>type of color gamut box |
| [depthDisocclusion](depth-disocclusion.md) | [main]<br>open var [depthDisocclusion](depth-disocclusion.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables depth-based disocclusion detection to reduce ghosting around moving objects |
| [disocclusionDepthThreshold](disocclusion-depth-threshold.md) | [main]<br>open var [disocclusionDepthThreshold](disocclusion-depth-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>depth difference threshold for disocclusion detection |
| [disocclusionHistoryNeighborhood](disocclusion-history-neighborhood.md) | [main]<br>open var [disocclusionHistoryNeighborhood](disocclusion-history-neighborhood.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>reduces false disocclusions on thin static geometry at a higher GPU cost |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables or disables temporal anti-aliasing |
| [exactLanczosSupport](exact-lanczos-support.md) | [main]<br>open var [exactLanczosSupport](exact-lanczos-support.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>uses a wider reconstruction filter kernel for higher sharpness at 1:1 resolution, at a higher GPU cost |
| [feedback](feedback.md) | [main]<br>open var [feedback](feedback.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA). |
| [filterHistory](filter-history.md) | [main]<br>open var [filterHistory](filter-history.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to filter the history buffer |
| [filterInput](filter-input.md) | [main]<br>open var [filterInput](filter-input.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to apply the reconstruction filter to the input |
| [filterWidth](filter-width.md) | [main]<br>open var [~~filterWidth~~](filter-width.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [hdr](hdr.md) | [main]<br>open var [hdr](hdr.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>set to true for HDR content |
| [historyReprojection](history-reprojection.md) | [main]<br>open var [historyReprojection](history-reprojection.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to apply history reprojection (debug option) |
| [jitterPattern](jitter-pattern.md) | [main]<br>open var [jitterPattern](jitter-pattern.md): [View.TemporalAntiAliasingOptions.JitterPattern](-jitter-pattern/index.md)<br>Jitter Pattern |
| [lodBias](lod-bias.md) | [main]<br>open var [lodBias](lod-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>texture lod bias (typically between -1.0 and 0. |
| [minAlphaStrength](min-alpha-strength.md) | [main]<br>open var [minAlphaStrength](min-alpha-strength.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>minimum history feedback scale when the camera is static, in (0, 1] |
| [minBoxStrength](min-box-strength.md) | [main]<br>open var [minBoxStrength](min-box-strength.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>minimum history clamping strength when the camera is static, in (0, 1] |
| [motionAdaptiveHistory](motion-adaptive-history.md) | [main]<br>open var [motionAdaptiveHistory](motion-adaptive-history.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>relaxes history rejection under low camera motion to reduce flickering on static content |
| [motionDeltaThreshold](motion-delta-threshold.md) | [main]<br>open var [motionDeltaThreshold](motion-delta-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>camera motion threshold in texels above which history rejection is applied at full strength |
| [motionHoldFrames](motion-hold-frames.md) | [main]<br>open var [motionHoldFrames](motion-hold-frames.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>number of frames to maintain full history rejection after camera motion stops (0 to disable) |
| [preventFlickering](prevent-flickering.md) | [main]<br>open var [preventFlickering](prevent-flickering.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>adjust the feedback dynamically to reduce flickering |
| [sharpness](sharpness.md) | [main]<br>open var [sharpness](sharpness.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>post-TAA sharpen, especially useful when upscaling is true. |
| [skyDepthThreshold](sky-depth-threshold.md) | [main]<br>open var [skyDepthThreshold](sky-depth-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>normalized depth threshold below which distant/sky pixels always use full history rejection |
| [upscaling](upscaling.md) | [main]<br>open var [upscaling](upscaling.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Upscaling factor. |
| [useYCoCg](use-y-co-cg.md) | [main]<br>open var [useYCoCg](use-y-co-cg.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to use the YcoCg color-space for history rejection |
| [varianceGamma](variance-gamma.md) | [main]<br>open var [varianceGamma](variance-gamma.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>High values increases ghosting artefact, lower values increases jittering, range [0.6, 1. |
