//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[shadowFar](shadow-far.md)

# shadowFar

[main]\
open var [shadowFar](shadow-far.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Distance from the camera after which shadows are clipped. This is used to clip shadows that are too far and wouldn't contribute to the scene much, improving performance and quality. This value is always positive. Use 0.0f to use the camera far distance. This only affect directional lights.
