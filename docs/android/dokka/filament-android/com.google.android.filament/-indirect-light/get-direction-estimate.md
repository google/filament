//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndirectLight](index.md)/[getDirectionEstimate](get-direction-estimate.md)

# getDirectionEstimate

[main]\
open fun [getDirectionEstimate](get-direction-estimate.md)(sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the direction of the dominant light in the environment. 

This assumes that there is only a single dominant light (such as the sun in outdoors environments), if it's not the case the direction returned will be an average of the various lights based on their intensity.

If there are no clear dominant light, as is often the case with low dynamic range (LDR) environments, this method may return a wrong or unexpected direction.

The dominant light direction can be used to set a directional light's direction, for instance to produce shadows that match the environment.

#### Return

the `direction` paramter if it was provided, or a newly allocated float array containing a unit vector representing the direction of the dominant light

#### Parameters

main

| | |
|---|---|
| sh | pre-scaled 3-bands spherical harmonics |
| direction | an array of 3 floats to receive a unit vector representing the direction of the dominant light or `null` |

#### See also

| |
|---|
| [LightManager.Builder](../-light-manager/-builder/direction.md) |
| getColorEstimate |

[main]\
open fun [~~getDirectionEstimate~~](get-direction-estimate.md)(direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

---

### Deprecated

---

#### Deprecated
