//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)/[intersects](intersects.md)

# intersects

[main]\
open fun [intersects](intersects.md)(box: [Box](../-box/index.md)): Boolean

Returns whether a box intersects the frustum (i.e. is visible)

#### Return

true if the box may intersects the frustum, false otherwise. In some situations a box that doesn't intersect the frustum might be reported as though it does. However, a box that does intersect the frustum is always reported correctly (true).

#### Parameters

main

| | |
|---|---|
| box | The box to test against the frustum |

[main]\
open fun [intersects](intersects.md)(spherex: Float, spherey: Float, spherez: Float, spherew: Float): Boolean

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
open fun [intersects](intersects.md)(sphere: Array&lt;Float&gt;): Boolean

Returns whether a sphere intersects the frustum (i.e. is visible)

#### Return

true if the sphere may intersects the frustum, false otherwise. In some situations a sphere that doesn't intersect the frustum might be reported as though it does. However, a sphere that does intersect the frustum is always reported correctly (true).

#### Parameters

main

| | |
|---|---|
| sphere | A sphere encoded as a center + radius. |
