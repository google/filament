//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)/[intersects](intersects.md)

# intersects

[main]\
open fun [intersects](intersects.md)(box: [Box](../-box/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Returns whether a box intersects the frustum (i.e. is visible)

#### Return

true if the box may intersects the frustum, false otherwise. In some situations a box that doesn't intersect the frustum might be reported as though it does. However, a box that does intersect the frustum is always reported correctly (true).

#### Parameters

main

| | |
|---|---|
| box | The box to test against the frustum |

[main]\
open fun [intersects](intersects.md)(spherex: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherey: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherez: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherew: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Returns whether a sphere intersects the frustum (i.e. is visible)

#### Return

true if the sphere may intersects the frustum, false otherwise. In some situations a sphere that doesn't intersect the frustum might be reported as though it does. However, a sphere that does intersect the frustum is always reported correctly (true).

#### Parameters

main

| | |
|---|---|
| spherex | (x component) A sphere encoded as a center + radius. |
| spherey | (y component) A sphere encoded as a center + radius. |
| spherez | (z component) A sphere encoded as a center + radius. |
| spherew | (w component) A sphere encoded as a center + radius. |

[main]\
open fun [intersects](intersects.md)(sphere: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Returns whether a sphere intersects the frustum (i.e. is visible)

#### Return

true if the sphere may intersects the frustum, false otherwise. In some situations a sphere that doesn't intersect the frustum might be reported as though it does. However, a sphere that does intersect the frustum is always reported correctly (true).

#### Parameters

main

| | |
|---|---|
| sphere | A sphere encoded as a center + radius. |
