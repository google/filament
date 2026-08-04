//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[TemporalAntiAliasingOptions](index.md)

# TemporalAntiAliasingOptions

open class [TemporalAntiAliasingOptions](index.md)

Options for Temporal Anti-aliasing (TAA) Most TAA parameters are extremely costly to change, as they will trigger the TAA post-process shaders to be recompiled. These options should be changed or set during initialization. `filterWidth`, `feedback` and `jitterPattern`, however, can be changed at any time. 

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
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables or disables temporal anti-aliasing |
| [feedback](feedback.md) | [main]<br>open var [feedback](feedback.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA). |
| [filterHistory](filter-history.md) | [main]<br>open var [filterHistory](filter-history.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to filter the history buffer |
| [filterInput](filter-input.md) | [main]<br>open var [filterInput](filter-input.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to apply the reconstruction filter to the input |
| [filterWidth](filter-width.md) | [main]<br>open var [~~filterWidth~~](filter-width.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [hdr](hdr.md) | [main]<br>open var [hdr](hdr.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>set to true for HDR content |
| [historyReprojection](history-reprojection.md) | [main]<br>open var [historyReprojection](history-reprojection.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to apply history reprojection (debug option) |
| [jitterPattern](jitter-pattern.md) | [main]<br>open var [jitterPattern](jitter-pattern.md): [View.TemporalAntiAliasingOptions.JitterPattern](-jitter-pattern/index.md)<br>Jitter Pattern |
| [lodBias](lod-bias.md) | [main]<br>open var [lodBias](lod-bias.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>texturing lod bias (typically -1 or -2) |
| [preventFlickering](prevent-flickering.md) | [main]<br>open var [preventFlickering](prevent-flickering.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>adjust the feedback dynamically to reduce flickering |
| [sharpness](sharpness.md) | [main]<br>open var [sharpness](sharpness.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>post-TAA sharpen, especially useful when upscaling is true. |
| [upscaling](upscaling.md) | [main]<br>open var [upscaling](upscaling.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Upscaling factor. |
| [useYCoCg](use-y-co-cg.md) | [main]<br>open var [useYCoCg](use-y-co-cg.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to use the YcoCg color-space for history rejection |
| [varianceGamma](variance-gamma.md) | [main]<br>open var [varianceGamma](variance-gamma.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>High values increases ghosting artefact, lower values increases jittering, range [0.75, 1. |
