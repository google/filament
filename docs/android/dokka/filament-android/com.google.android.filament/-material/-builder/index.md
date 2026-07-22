//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [ShadowSamplingQuality](-shadow-sampling-quality/index.md) | [main]<br>enum [ShadowSamplingQuality](-shadow-sampling-quality/index.md) |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Material](../index.md)<br>Creates and returns the Material object. |
| [payload](payload.md) | [main]<br>open fun [payload](payload.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), size: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Material.Builder](index.md)<br>Specifies the material data. |
| [shadowSamplingQuality](shadow-sampling-quality.md) | [main]<br>open fun [shadowSamplingQuality](shadow-sampling-quality.md)(quality: [Material.Builder.ShadowSamplingQuality](-shadow-sampling-quality/index.md)): [Material.Builder](index.md)<br>Set the quality of shadow sampling. |
| [sphericalHarmonicsBandCount](spherical-harmonics-band-count.md) | [main]<br>open fun [sphericalHarmonicsBandCount](spherical-harmonics-band-count.md)(shBandCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Material.Builder](index.md)<br>Sets the quality of the indirect lights computations. |
| [uboBatching](ubo-batching.md) | [main]<br>open fun [uboBatching](ubo-batching.md)(mode: [Material.UboBatchingMode](../-ubo-batching-mode/index.md)): [Material.Builder](index.md)<br>Set the batching mode of the instances created from this material. |
