//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MorphTargetBuffer](../index.md)/[Builder](index.md)/[withPositions](with-positions.md)

# withPositions

[main]\
open fun [withPositions](with-positions.md)(): [MorphTargetBuffer.Builder](index.md)

Enables and allocates the built-in buffer for position morphing. 

If enabled, `setPositionsAt` can be called to set the position data for each target. The vertex position will be morphed automatically without any further actions.

#### Return

A reference to this Builder for chaining calls.

[main]\
open fun [withPositions](with-positions.md)(enable: Boolean): [MorphTargetBuffer.Builder](index.md)

Enables and allocates the built-in buffer for position morphing. 

If enabled, `setPositionsAt` can be called to set the position data for each target. The vertex position will be morphed automatically without any further actions.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| enable | true to enable, false to disable. Default is true. |
