//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[radiance](radiance.md)

# radiance

[main]\
open fun [radiance](radiance.md)(bands: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [IndirectLight.Builder](index.md)

Sets the irradiance from the radiance expressed as Spherical Harmonics. 

The radiance must be specified as Spherical Harmonics coefficients Ll,m, where each coefficient is comprised of three floats for red, green and blue components, respectively

The index in the `sh` array is given by: `index(l, m) = 3 × (l * (l + 1) + m)``sh[index(l,m) + 0] = LRl,m``sh[index(l,m) + 1] = LGl,m``sh[index(l,m) + 2] = LBl,m`

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| bands | Number of spherical harmonics bands. Must be 1, 2 or 3. |
| sh | Array containing the spherical harmonics coefficients. The size of the array must be 3 × `bands2`(i.e. 1, 4 or 9 `float3` coefficients respectively). |

#### Throws

| | |
|---|---|
| [ArrayIndexOutOfBoundsException](https://developer.android.com/reference/kotlin/java/lang/ArrayIndexOutOfBoundsException.html) | if the `sh` array length is smaller than 3 × bands2 |
