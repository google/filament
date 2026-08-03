//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[VsmShadowOptions](index.md)

# VsmShadowOptions

open class [VsmShadowOptions](index.md)

View-level options for VSM Shadowing.

#### See also

| | |
|---|---|
| [setVsmShadowOptions](../set-vsm-shadow-options.md) | **Warning:** This API is still experimental and subject to change. |

## Constructors

| | |
|---|---|
| [VsmShadowOptions](-vsm-shadow-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [anisotropy](anisotropy.md) | [main]<br>open var [anisotropy](anisotropy.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Sets the number of anisotropic samples to use when sampling a VSM shadow map. |
| [highPrecision](high-precision.md) | [main]<br>open var [highPrecision](high-precision.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether to use a 32-bits or 16-bits texture format for VSM shadow maps. |
| [lightBleedReduction](light-bleed-reduction.md) | [main]<br>open var [lightBleedReduction](light-bleed-reduction.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>VSM light bleeding reduction amount, between 0 and 1. |
| [minVarianceScale](min-variance-scale.md) | [main]<br>open var [~~minVarianceScale~~](min-variance-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [mipmapping](mipmapping.md) | [main]<br>open var [mipmapping](mipmapping.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Whether to generate mipmaps for all VSM shadow maps. |
| [msaaSamples](msaa-samples.md) | [main]<br>open var [msaaSamples](msaa-samples.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>The number of MSAA samples to use when rendering VSM shadow maps. |
