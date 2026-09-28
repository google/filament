//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[PickingQueryResult](index.md)/[fragCoords](frag-coords.md)

# fragCoords

[main]\
open var [fragCoords](frag-coords.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

screen space coordinates in GL convention, this can be used to compute the view or world space position of the picking hit. 

For e.g.: clip_space_position = (fragCoords.xy / viewport.wh, fragCoords.z) * 2.0 - 1.0 view_space_position = inverse(projection) * clip_space_position world_space_position = model * view_space_position

The viewport, projection and model matrices can be obtained from Camera. Because pick() has some latency, it might be more accurate to obtain these values at the time the View::pick() call is made.

Note: if the Engine is running at FEATURE_LEVEL_0, the precision or `depth` and `fragCoords.z` is only 8-bits.
