//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[stepCount](step-count.md)

# stepCount

[main]\
open var [stepCount](step-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Number of ray-marching steps for screen-space contact shadows (8 by default). 

**CAUTION:** this parameter is ignored for all lights except the directional/sun light, all other lights use the same value set for the directional/sun light.
