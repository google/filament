//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MorphTargetBuffer](../index.md)/[Builder](index.md)/[withTangents](with-tangents.md)

# withTangents

[main]\
open fun [withTangents](with-tangents.md)(): [MorphTargetBuffer.Builder](index.md)

Enables and allocates the built-in buffer for tangent/normal morphing. 

If enabled, `setTangentsAt` can be called to set the tangent data for each target. The vertex position will be morphed automatically without any further actions.

#### Return

A reference to this Builder for chaining calls.

[main]\
open fun [withTangents](with-tangents.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)

Enables and allocates the built-in buffer for tangent/normal morphing. 

If enabled, `setTangentsAt` can be called to set the tangent data for each target. The vertex position will be morphed automatically without any further actions.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| enable | true to enable, false to disable. Default is true. |
