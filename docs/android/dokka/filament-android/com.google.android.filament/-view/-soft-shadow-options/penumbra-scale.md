//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[SoftShadowOptions](index.md)/[penumbraScale](penumbra-scale.md)

# penumbraScale

[main]\
open var [penumbraScale](penumbra-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a global scale factor applied to the final penumbra size of all PCSS shadows. This parameter acts as an artistic modifier, uniformly scaling the overall softness of shadows across the entire scene without altering the physical angular size of the light sources. The final scale applied to a shadow is calculated by modulating this global value with the light's individual penumbraScale (global * local). This allows art directors to shift the global mood (e.g., making all shadows 20% softer) while preserving the relative contrast between different lights. The global penumbra scale multiplier. Default is 1.0 (physically based).

#### See also

| | |
|---|---|
| [LightManager](../../-light-manager/index.md) | ::ShadowOptions::penumbraScale |
