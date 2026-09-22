//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)/[transform](transform.md)

# transform

[main]\
open fun [transform](transform.md)(m: Array&lt;Float&gt;, tx: Float, ty: Float, tz: Float, box: [Box](index.md)): [Box](index.md)

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
open fun [transform](transform.md)(m: Array&lt;Float&gt;, t: Array&lt;Float&gt;, box: [Box](index.md)): [Box](index.md)

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
