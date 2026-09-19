//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)/[set](set.md)

# set

[main]\
open fun [set](set.md)(minx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), miny: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), minz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), maxx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), maxy: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), maxz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Box](index.md)

Initializes the 3D box from its min / max coordinates on each axis

#### Return

This bounding box

#### Parameters

main

| | |
|---|---|
| minx | (x component) lowest coordinates corner of the box |
| miny | (y component) lowest coordinates corner of the box |
| minz | (z component) lowest coordinates corner of the box |
| maxx | (x component) largest coordinates corner of the box |
| maxy | (y component) largest coordinates corner of the box |
| maxz | (z component) largest coordinates corner of the box |

[main]\
open fun [set](set.md)(min: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, max: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Box](index.md)

Initializes the 3D box from its min / max coordinates on each axis

#### Return

This bounding box

#### Parameters

main

| | |
|---|---|
| min | lowest coordinates corner of the box |
| max | largest coordinates corner of the box |
