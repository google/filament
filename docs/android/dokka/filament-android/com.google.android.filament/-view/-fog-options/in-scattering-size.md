//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[inScatteringSize](in-scattering-size.md)

# inScatteringSize

[main]\
open var [inScatteringSize](in-scattering-size.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Very inaccurately simulates the Sun's in-scattering. That is, the light from the sun that is scattered (by the fog) towards the camera. Size of the Sun in-scattering (>0 to activate). Good values are >>1 (e.g. ~10 - 100). Smaller values result is a larger scattering size. Ignored in `linearFog` mode.
