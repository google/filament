//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[color](color.md)

# color

[main]\
open var [color](color.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Fog's color is used for ambient light in-scattering, a good value is to use the average of the ambient light, possibly tinted towards blue for outdoors environments. Color component's values should be between 0 and 1, values above one are allowed but could create a non energy-conservative fog (this is dependant on the IBL's intensity as well). 

We assume that our fog has no absorption and therefore all the light it scatters out becomes ambient light in-scattering and has lost all directionality, i.e.: scattering is isotropic. This somewhat simulates Rayleigh scattering.

This value is used as a tint instead, when fogColorFromIbl is enabled.

#### See also

| |
|---|
| [fogColorFromIbl](fog-color-from-ibl.md) |
