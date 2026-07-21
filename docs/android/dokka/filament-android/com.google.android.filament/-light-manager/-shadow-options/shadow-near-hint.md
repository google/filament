//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[shadowNearHint](shadow-near-hint.md)

# shadowNearHint

[main]\
open var [shadowNearHint](shadow-near-hint.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Optimize the quality of shadows from this distance from the camera. Shadows will be rendered in front of this distance, but the quality may not be optimal. This value is always positive. Use 0.0f to use the camera near distance. The default of 1m works well with many scenes. The quality of shadows may drop rapidly when this value decreases.
