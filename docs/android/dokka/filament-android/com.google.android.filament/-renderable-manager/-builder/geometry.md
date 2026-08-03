//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[geometry](geometry.md)

# geometry

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), minIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), maxIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the geometry data for a primitive. Filament primitives must have an associated [VertexBuffer](../../-vertex-buffer/index.md) and [IndexBuffer](../../-index-buffer/index.md). Typically, each primitive is specified with a pair of daisy-chained calls: `geometry()` and `material()`.

#### Parameters

main

| | |
|---|---|
| index | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| type | specifies the topology of the primitive (e.g., [TRIANGLES](../-primitive-type/-t-r-i-a-n-g-l-e-s/index.md)) |
| vertices | specifies the vertex buffer, which in turn specifies a set of attributes |
| indices | specifies the index buffer (either u16 or u32) |
| offset | specifies where in the index buffer to start reading (expressed as a number of indices) |
| minIndex | specifies the minimum index contained in the index buffer |
| maxIndex | specifies the maximum index contained in the index buffer |
| count | number of indices to read (for triangles, this should be a multiple of 3) |

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md)): [RenderableManager.Builder](index.md)

For details, see the geometry primary overload.

[main]\
open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

open fun [geometry](geometry.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md)): [RenderableManager.Builder](index.md)

For details, see the geometry primary overload. (non-indexed version)
