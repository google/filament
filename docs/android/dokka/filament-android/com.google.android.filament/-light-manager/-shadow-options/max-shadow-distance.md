//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[maxShadowDistance](max-shadow-distance.md)

# maxShadowDistance

[main]\
open var [maxShadowDistance](max-shadow-distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Maximum shadow-occluder distance for screen-space contact shadows (world units). (30 cm by default) 

**CAUTION:** this parameter is ignored for all lights except the directional/sun light, all other lights use the same value set for the directional/sun light.
