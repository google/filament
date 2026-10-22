//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getEyeFromViewMatrix](get-eye-from-view-matrix.md)

# getEyeFromViewMatrix

[main]\
open fun [getEyeFromViewMatrix](get-eye-from-view-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the eye from view matrix for the specified eye.

#### Return

The eye from view matrix

#### Parameters

main

| | |
|---|---|
| out | optional array to store the result, or null to allocate a new one |

[main]\
open fun [getEyeFromViewMatrix](get-eye-from-view-matrix.md)(eyeId: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the eye from view matrix for the specified eye.

#### Return

The eye from view matrix

#### Parameters

main

| | |
|---|---|
| eyeId | the index of the eye to return the eye from view matrix for, must be <config.stereoscopicEyeCount |
