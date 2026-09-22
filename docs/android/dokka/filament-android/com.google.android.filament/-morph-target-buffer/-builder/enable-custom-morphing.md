//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MorphTargetBuffer](../index.md)/[Builder](index.md)/[enableCustomMorphing](enable-custom-morphing.md)

# enableCustomMorphing

[main]\
open fun [enableCustomMorphing](enable-custom-morphing.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)

Enables the custom morphing pipeline. 

When enabled, the `morphData2`, `morphData3`, and `morphData4` helper functions are available in the vertex shader. You must provide a 2D array texture containing the morph deltas, bind it to a `sampler2DArray` uniform, and call the appropriate `morphData` function to apply the morphing to your custom attributes.

Note: Unlike `withPositions` or `withTangents`, this does NOT allocate any internal storage. You are responsible for managing the morph data texture.

Custom morphing can be used together with automatic position and/or tangent morphing.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| enable | true to enable, false to disable. Default is false. |
