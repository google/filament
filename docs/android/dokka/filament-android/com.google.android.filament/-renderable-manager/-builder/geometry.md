//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[geometry](geometry.md)

# geometry

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), minIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), maxIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the geometry data for a primitive. 

Associates a vertex buffer and an index buffer with a primitive. Typically, each primitive is specified with a pair of daisy-chained calls: \c geometry(...) and \c material(...).

#### Parameters

main

| | |
|---|---|
| index | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| type | specifies the topology of the primitive (e.g., \c RenderableManager::PrimitiveType::TRIANGLES) |
| vertices | specifies the vertex buffer, which in turn specifies a set of attributes |
| indices | specifies the index buffer (either u16 or u32) |
| offset | specifies where in the index buffer to start reading (expressed as a number of indices) |
| minIndex | specifies the minimum index contained in the index buffer |
| maxIndex | specifies the maximum index contained in the index buffer |
| count | number of indices to read (for triangles, this should be a multiple of 3) |

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md)): [RenderableManager.Builder](index.md)

open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md)): [RenderableManager.Builder](index.md)

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the geometry data for a primitive. (non-indexed version) 

Filament primitives normally have an associated vertex buffer and index buffer. Typically, each primitive is specified with a pair of daisy-chained calls: \c geometry(...) and \c material(...).

Non-indexed rendering: when `indices` is not provided, the primitive is treated as a non-indexed draw and `offset` / `count` refer to vertex offset and vertex count respectively.

Attribute-less rendering: This can be used for procedural rendering, where the vertex shader generates positions, UVs, etc procedurally, typically from \c gl_VertexIndex / \c gl_VertexID / and \c [[vertex_id]], which can be accessed by calling `getVertexIndex()` in vertex shader. The associated VertexBuffer may have \c bufferCount == 0 with no declared attributes (see \c VertexBuffer::Builder). Attribute-less rendering requires \c FEATURE_LEVEL_1 or higher as GLES2 has no `gl_VertexID` and is incompatible with skinning and morphing.

#### Parameters

main

| | |
|---|---|
| index | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| type | specifies the topology of the primitive (e.g., \c RenderableManager::PrimitiveType::TRIANGLES) |
| vertices | specifies the vertex buffer, which in turn specifies a set of attributes |
| offset | specifies where in the vertex buffer to start reading (expressed as a number of vertices) |
| count | number of vertices to read (for triangles, this should be a multiple of 3) |
