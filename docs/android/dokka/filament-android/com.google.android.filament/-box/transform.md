//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)/[transform](transform.md)

# transform

[main]\
open fun [transform](transform.md)(m: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, tx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), ty: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), box: [Box](index.md)): [Box](index.md)

Transform a Box by a linear transform and a translation.

#### Return

the bounding box of the transformed box

#### Parameters

main

| | |
|---|---|
| m | a 3x3 matrix, the linear transform |
| tx | (x component) a float3, the translation |
| ty | (y component) a float3, the translation |
| tz | (z component) a float3, the translation |
| box | the box to transform |

[main]\
open fun [transform](transform.md)(m: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, t: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, box: [Box](index.md)): [Box](index.md)

Transform a Box by a linear transform and a translation.

#### Return

the bounding box of the transformed box

#### Parameters

main

| | |
|---|---|
| m | a 3x3 matrix, the linear transform |
| t | a float3, the translation |
| box | the box to transform |
