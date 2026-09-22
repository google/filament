//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)/[irradiance](irradiance.md)

# irradiance

[main]\
open fun [irradiance](irradiance.md)(bands: Int, sh: Array&lt;Float&gt;): [IndirectLight.Builder](index.md)

Sets the irradiance as Spherical Harmonics. 

The irradiance must be pre-convolved by ⟨ n · l ⟩ and pre-multiplied by the Lambertian diffuse BRDF 1 / π and specified as Spherical Harmonics coefficients.

Additionally, these Spherical Harmonics coefficients must be pre-scaled by the reconstruction factors Alm below.

The final coefficients can be generated using the `cmgen` tool.

The index in the `sh` array is given by:

`index(l, m) = l * (l + 1) + m`

sh[index(l,m)] = Llm 1 / π Alm Cl

| | | | | | |
|---|---|---|---|---|---|
|  |  |  |  |  |  |
| 0 | 0 | 0 | 0.282095 | 3.1415926 | 0.282095 |
| 1 | 1 | -1 | -0.488602 | 2.0943951 | -0.325735 |
| 2 | ^ | 0 | 0.488602 | ^ | 0.325735 |
| 3 | ^ | 1 | -0.488602 | ^ | -0.325735 |
| 4 | 2 | -2 | 1.092548 | 0.785398 | 0.273137 |
| 5 | ^ | -1 | -1.092548 | ^ | -0.273137 |
| 6 | ^ | 0 | 0.315392 | ^ | 0.078848 |
| 7 | ^ | 1 | -1.092548 | ^ | -0.273137 |
| 8 | ^ | 2 | 0.546274 | ^ | 0.136569 |

Only 1, 2 or 3 bands are allowed.

Because the coefficients are pre-scaled, `sh[0]` is the environment's average irradiance.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| bands | Number of spherical harmonics bands. Must be 1, 2 or 3. |
| sh | Array containing the spherical harmonics coefficients. The size of the array must be bands2. (i.e. 1, 4 or 9 coefficients respectively). |

#### See also

| |
|---|
| [Material.Builder](../../-material/-builder/spherical-harmonics-band-count.md) |

[main]\
open fun [irradiance](irradiance.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)

Sets the irradiance as a cubemap. 

The irradiance can alternatively be specified as a cubemap instead of Spherical Harmonics coefficients. It may or may not be more efficient, depending on your hardware (essentially, it's trading ALU for bandwidth).

This irradiance cubemap can be generated with the **cmgen** tool.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| cubemap | Cubemap representing the Irradiance pre-convolved by ⟨ n · l ⟩. |

#### See also

| |
|---|
| [irradiance(int, float[])](irradiance.md) |
