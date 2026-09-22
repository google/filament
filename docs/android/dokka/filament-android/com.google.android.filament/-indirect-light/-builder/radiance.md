//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[radiance](radiance.md)

# radiance

[main]\
open fun [radiance](radiance.md)(bands: Int, sh: Array&lt;Float&gt;): [IndirectLight.Builder](index.md)

Sets the irradiance from the radiance expressed as Spherical Harmonics. 

The radiance must be specified as Spherical Harmonics coefficients Llm

The index in the `sh` array is given by:

`index(l, m) = l * (l + 1) + m`

sh[index(l,m)] = Llm

| | | |
|---|---|---|
|  |  |  |
| 0 | 0 | 0 |
| 1 | 1 | -1 |
| 2 | ^ | 0 |
| 3 | ^ | 1 |
| 4 | 2 | -2 |
| 5 | ^ | -1 |
| 6 | ^ | 0 |
| 7 | ^ | 1 |
| 8 | ^ | 2 |

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| bands | Number of spherical harmonics bands. Must be 1, 2 or 3. |
| sh | Array containing the spherical harmonics coefficients. The size of the array must be bands2. (i.e. 1, 4 or 9 coefficients respectively). |
