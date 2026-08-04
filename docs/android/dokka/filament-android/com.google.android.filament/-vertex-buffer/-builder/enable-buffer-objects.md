//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[enableBufferObjects](enable-buffer-objects.md)

# enableBufferObjects

[main]\
open fun [enableBufferObjects](enable-buffer-objects.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [VertexBuffer.Builder](index.md)

Allows buffers to be swapped out and shared using BufferObject. If buffer objects mode is enabled, clients must call setBufferObjectAt rather than setBufferAt. This allows sharing of data between VertexBuffer objects, but it may slightly increase the memory footprint of Filament's internal bookkeeping.

#### Parameters

main

| | |
|---|---|
| enabled | If true, enables buffer object mode. False by default. |
