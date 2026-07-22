//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[shadowFarHint](shadow-far-hint.md)

# shadowFarHint

[main]\
open var [shadowFarHint](shadow-far-hint.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Optimize the quality of shadows in front of this distance from the camera. Shadows will be rendered behind this distance, but the quality may not be optimal. This value is always positive. Use std::numerical_limits::infinity() to use the camera far distance.
