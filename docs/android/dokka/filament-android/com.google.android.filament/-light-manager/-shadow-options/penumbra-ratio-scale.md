//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[penumbraRatioScale](penumbra-ratio-scale.md)

# penumbraRatioScale

[main]\
open var [penumbraRatioScale](penumbra-ratio-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a light-specific scale factor applied to the PCSS geometric ratio before clamping. This parameter controls the &quot;contact shadow contrast&quot; for this specific light. It allows artists to dictate how rapidly this light's shadow transitions from razor-sharp at the contact point to its maximum blur radius. It is heavily utilized to stylize lighting and aggressively mask 2.5D shadow map limitations near occluders. - Values >1.0 (e.g., 10.0 or 20.0) create a rapid, cinematic blur acceleration. - Values <1.0 keep the shadow crisp over longer distances. The final ratio scale applied is calculated by modulating this local value with the global SoftShadowOptions::penumbraRatioScale (global * local). The local penumbra ratio scale multiplier. Default is 1.0.

#### See also

| |
|---|
| [View.SoftShadowOptions](../../-view/-soft-shadow-options/penumbra-ratio-scale.md) |
