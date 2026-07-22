//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[cutOffDistance](cut-off-distance.md)

# cutOffDistance

[main]\
open var [cutOffDistance](cut-off-distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Distance in world units [m] after which the fog calculation is disabled. This can be used to exclude the skybox, which is desirable if it already contains clouds or fog. The default value is +infinity which applies the fog to everything. 

Note: The SkyBox is typically at a distance of 1e19 in world space (depending on the near plane distance and projection used though).
