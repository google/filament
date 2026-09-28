//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setGeometryAt](set-geometry-at.md)

# setGeometryAt

[main]\
open fun [setGeometryAt](set-geometry-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), indices: [IndexBuffer](../-index-buffer/index.md))

Changes the geometry for the given primitive.

#### Parameters

main

| | |
|---|---|
| instance | Renderable's instance |
| primitiveIndex | Primitive index |
| type | Specifies the topology of the primitive |
| vertices | Specifies the vertex buffer |
| indices | Specifies the index buffer |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |

[main]\
open fun [setGeometryAt](set-geometry-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), indices: [IndexBuffer](../-index-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Changes the geometry for the given primitive.

#### Parameters

main

| | |
|---|---|
| instance | Renderable's instance |
| primitiveIndex | Primitive index |
| type | Specifies the topology of the primitive |
| vertices | Specifies the vertex buffer |
| indices | Specifies the index buffer |
| offset | Specifies where in the index buffer to start reading (expressed as a number of indices) |
| count | Number of indices to read |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |

[main]\
open fun [setGeometryAt](set-geometry-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md))

Changes the geometry for the given primitive. (non-indexed version)

#### Parameters

main

| | |
|---|---|
| instance | Renderable's instance |
| primitiveIndex | Primitive index |
| type | Specifies the topology of the primitive |
| vertices | Specifies the vertex buffer |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |

[main]\
open fun [setGeometryAt](set-geometry-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Changes the geometry for the given primitive. (non-indexed version)

#### Parameters

main

| | |
|---|---|
| instance | Renderable's instance |
| primitiveIndex | Primitive index |
| type | Specifies the topology of the primitive |
| vertices | Specifies the vertex buffer |
| offset | Specifies where in the vertex buffer to start reading (expressed as a number of vertices) |
| count | Number of vertices to read |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |
