//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)/[getNormalizedPlane](get-normalized-plane.md)

# getNormalizedPlane

[main]\
open fun [getNormalizedPlane](get-normalized-plane.md)(plane: [Frustum.Plane](-plane/index.md), out: Array&lt;Float&gt;): Array&lt;Float&gt;

Returns the plane equation parameters with normalized normals

#### Return

A plane equation encoded a float4 R such as R.x*x + R.y*y + R.z*z + R.w = 0

#### Parameters

main

| | |
|---|---|
| plane | Identifier of the plane to retrieve the equation of |
