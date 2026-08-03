//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[irradiance](irradiance.md)

# irradiance

[main]\
open fun [irradiance](irradiance.md)(bands: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [IndirectLight.Builder](index.md)

Sets the irradiance as Spherical Harmonics. 

The irradiance coefficients must be pre-convolved by `< n &sdot l >` and pre-multiplied by the Lambertian diffuse BRDF `1/&pi` and specified as Spherical Harmonics coefficients.

Additionally, these Spherical Harmonics coefficients must be pre-scaled by the reconstruction factors Al,m.

The final coefficients can be generated using the `cmgen` tool.

The index in the `sh` array is given by: `index(l, m) = 3 × (l * (l + 1) + m)``sh[index(l,m) + 0] = LRl,m
         × 1/&pi 
         × Al,m
         × Cl``sh[index(l,m) + 1] = LGl,m
         × 1/&pi 
         × Al,m
         × Cl``sh[index(l,m) + 2] = LBl,m
         × 1/&pi 
         × Al,m
         × Cl`

Only 1, 2 or 3 bands are allowed.

Because the coefficients are pre-scaled, `sh[0]` is the environment's average irradiance.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| bands | Number of spherical harmonics bands. Must be 1, 2 or 3. |
| sh | Array containing the spherical harmonics coefficients. The size of the array must be `3 × bands2`(i.e. 1, 4 or 9 `float3` coefficients respectively). |

#### Throws

| | |
|---|---|
| [ArrayIndexOutOfBoundsException](https://developer.android.com/reference/kotlin/java/lang/ArrayIndexOutOfBoundsException.html) | if the `sh` array length is smaller than 3 × bands2 |

[main]\
open fun [irradiance](irradiance.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)

Sets the irradiance as a cubemap. 

 The irradiance can alternatively be specified as a cubemap instead of Spherical Harmonics coefficients. It may or may not be more efficient, depending on your hardware (essentially, it's trading ALU for bandwidth).  This irradiance cubemap can be generated with the `cmgen` tool.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| cubemap | Cubemap representing the Irradiance pre-convolved by `< n &sdot l >`. |

#### See also

| |
|---|
| [irradiance(int bands, float[] sh)](irradiance.md) |
