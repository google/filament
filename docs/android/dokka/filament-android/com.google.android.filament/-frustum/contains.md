//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)/[contains](contains.md)

# contains

[main]\
open fun [contains](contains.md)(px: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), py: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), pz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns whether the frustum contains a given point.

#### Return

the maximum signed distance to the frustum. Negative if p is inside.

#### Parameters

main

| | |
|---|---|
| px | (x component) the point to test |
| py | (y component) the point to test |
| pz | (z component) the point to test |

[main]\
open fun [contains](contains.md)(p: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns whether the frustum contains a given point.

#### Return

the maximum signed distance to the frustum. Negative if p is inside.

#### Parameters

main

| | |
|---|---|
| p | the point to test |
