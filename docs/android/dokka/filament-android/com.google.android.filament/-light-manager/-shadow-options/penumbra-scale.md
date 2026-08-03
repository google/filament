//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[penumbraScale](penumbra-scale.md)

# penumbraScale

[main]\
open var [penumbraScale](penumbra-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a light-specific scale factor applied to the final penumbra size of PCSS shadows. This parameter acts as an artistic modifier, allowing you to artificially soften or sharpen the shadows cast by this specific light without changing its physical light bulb size or altering the global scene lighting. The final scale applied to the shadow is calculated by modulating this local value with the global SoftShadowOptions::penumbraScale (global * local). The local penumbra scale multiplier. Default is 1.0.

#### See also

| |
|---|
| [View.SoftShadowOptions](../../-view/-soft-shadow-options/penumbra-scale.md) |
