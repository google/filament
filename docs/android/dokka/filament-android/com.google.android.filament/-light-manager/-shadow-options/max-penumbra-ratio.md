//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[maxPenumbraRatio](max-penumbra-ratio.md)

# maxPenumbraRatio

[main]\
open var [maxPenumbraRatio](max-penumbra-ratio.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a light-specific maximum geometric ratio applied to Percentage-Closer Soft Shadows (PCSS), overriding the global default. In PCSS, overlapping occluders (like complex light fixtures) can cause the shadow map's 2.5D depth limitations to calculate an artificially close blocker depth. This drives the geometric ratio toward infinity, resulting in unnatural, massive &quot;ghost&quot; shadows. This parameter allows you to clamp the geometric ratio for this specific light, fixing geometric artifacts caused by layered occluders without compromising the soft shadows of other lights in the scene. The maximum penumbra ratio. Setting this to a value <= 0.0f disables the override and reverts the light to using the global default.

#### See also

| |
|---|
| [View.SoftShadowOptions](../../-view/-soft-shadow-options/max-penumbra-ratio.md) |
