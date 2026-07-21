//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[density](density.md)

# density

[main]\
open var [density](density.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Extinction factor in [1/m] at an altitude 'height'. The extinction factor controls how much light is absorbed and out-scattered per unit of distance. Each unit of extinction reduces the incoming light to 37% of its original value. 

Note: The extinction factor is related to the fog density, it's usually some constant K times the density at sea level (more specifically at fog height). The constant K depends on the composition of the fog/atmosphere.

For historical reason this parameter is called `density`.

In `linearFog` mode this is the slope of the linear equation if heightFalloff is set to 0. Otherwise, heightFalloff affects the slope calculation such that it matches the slope of the standard equation at the camera height.
