//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[bufferCount](buffer-count.md)

# bufferCount

[main]\
open fun [bufferCount](buffer-count.md)(bufferCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)

Defines how many buffers will be created in this vertex buffer set. These buffers are later referenced by index from 0 to `bufferCount` - 1. 

For non-indexed / attribute-less (procedural) rendering, `bufferCount` can be set to 0 if no vertex attributes are declared. This requires `FEATURE_LEVEL_1` or higher.

 This call is mandatory. The default is 0.

#### Return

this `Builder` for chaining calls

#### Parameters

main

| | |
|---|---|
| bufferCount | number of buffers in this vertex buffer set. The maximum value is 8. |
