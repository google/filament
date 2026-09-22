//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)/[contains](contains.md)

# contains

[main]\
open fun [contains](contains.md)(px: Float, py: Float, pz: Float): Float

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
open fun [contains](contains.md)(p: Array&lt;Float&gt;): Float

Returns whether the frustum contains a given point.

#### Return

the maximum signed distance to the frustum. Negative if p is inside.

#### Parameters

main

| | |
|---|---|
| p | the point to test |
