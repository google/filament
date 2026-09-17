//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[bufferCount](buffer-count.md)

# bufferCount

[main]\
open fun [bufferCount](buffer-count.md)(bufferCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)

Defines how many buffers will be created in this vertex buffer set. 

These buffers are later referenced by index from 0 to `bufferCount` - 1.

This call is mandatory. The default is 0.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| bufferCount | Number of buffers in this vertex buffer set. The maximum value is 8. |
