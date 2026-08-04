//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[Builder](index.md)/[sphericalHarmonicsBandCount](spherical-harmonics-band-count.md)

# sphericalHarmonicsBandCount

[main]\
open fun [sphericalHarmonicsBandCount](spherical-harmonics-band-count.md)(shBandCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Material.Builder](index.md)

Sets the quality of the indirect lights computations. This is only taken into account if this material is lit and in the surface domain. This setting will affect the IndirectLight computation if one is specified on the Scene and Spherical Harmonics are used for the irradiance.

#### Return

Reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| shBandCount | Number of spherical harmonic bands. Must be 1, 2 or 3 (default). |

#### See also

| |
|---|
| [IndirectLight](../../-indirect-light/index.md) |
