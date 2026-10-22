//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[GenericToneMapper](index.md)/[isOneDimensional](is-one-dimensional.md)

# isOneDimensional

[main]\
open fun [isOneDimensional](is-one-dimensional.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x.b)) 

This may be used to indicate that the color grading's LUT only requires a 1D texture instead of a 3D texture, potentially saving a significant amount of memory and generation time.
