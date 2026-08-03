//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[DynamicResolutionOptions](index.md)

# DynamicResolutionOptions

open class [DynamicResolutionOptions](index.md)

Dynamic resolution can be used to either reach a desired target frame rate by lowering the resolution of a View, or to increase the quality when the rendering is faster than the target frame rate. 

This structure can be used to specify the minimum scale factor used when lowering the resolution of a View, and the maximum scale factor used when increasing the resolution for higher quality rendering. The scale factors can be controlled on each X and Y axis independently. By default, all scale factors are set to 1.0.

- enabled: enable or disables dynamic resolution on a View
- homogeneousScaling: by default the system scales the major axis first. Set this to true to force homogeneous scaling.
- minScale: the minimum scale in X and Y this View should use
- maxScale: the maximum scale in X and Y this View should use
- quality: upscaling quality. LOW: 1 bilinear tap, Medium: 4 bilinear taps, High: 9 bilinear taps (tent)

Note: Dynamic resolution is only supported on platforms where the time to render a frame can be measured accurately. On platforms where this is not supported, Dynamic Resolution can't be enabled unless `minScale == maxScale`.

#### See also

| |
|---|
| [Renderer.FrameRateOptions](../../-renderer/-frame-rate-options/index.md) |

## Constructors

| | |
|---|---|
| [DynamicResolutionOptions](-dynamic-resolution-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enable or disable dynamic resolution |
| [homogeneousScaling](homogeneous-scaling.md) | [main]<br>open var [homogeneousScaling](homogeneous-scaling.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>set to true to force homogeneous scaling |
| [maxScale](max-scale.md) | [main]<br>open var [maxScale](max-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>maximum scale factors in x and y |
| [minScale](min-scale.md) | [main]<br>open var [minScale](min-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>minimum scale factors in x and y |
| [quality](quality.md) | [main]<br>open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)<br>Upscaling quality <br>- LOW: bilinear filtered blit. Fastest, poor quality - MEDIUM: Qualcomm Snapdragon Game Super Resolution (SGSR) 1.0 - HIGH: AMD FidelityFX FSR1 w/ mobile optimizations - ULTRA: AMD FidelityFX FSR1<br> FSR1 and SGSR require a well anti-aliased (MSAA or TAA), noise free scene. |
| [sharpness](sharpness.md) | [main]<br>open var [sharpness](sharpness.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>sharpness when QualityLevel::MEDIUM or higher is used [0 (disabled), 1 (sharpest)] |
