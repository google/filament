//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[skinning](skinning.md)

# skinning

[main]\
open fun [skinning](skinning.md)(skinningBuffer: [SkinningBuffer](../../-skinning-buffer/index.md), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be enabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the \c BONE_WEIGHTS slot (float4).

See also RenderableManager::setSkinningBuffer() or SkinningBuffer::setBones(), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| skinningBuffer | nullptr to disable, otherwise the SkinningBuffer to use |
| count | 0 to disable, otherwise the number of bone transforms (up to 255) |
| offset | offset in the SkinningBuffer |

[main]\
open fun [skinning](skinning.md)(boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bones: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [RenderableManager.Builder](index.md)

#### Parameters

main

| | |
|---|---|
| boneCount | number of elements (structured element count) in `bones` |
| bones | buffer containing bones data |

[main]\
open fun [skinning](skinning.md)(bones: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

#### Parameters

main

| | |
|---|---|
| bones | array containing bones data |
| offset | offset in elements (structured element count) in `bones` to skip |
| boneCount | number of elements (structured element count) in `bones` |

[main]\
open fun [skinning](skinning.md)(bones: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

#### Parameters

main

| | |
|---|---|
| bones | array containing bones data |
| boneCount | number of elements (structured element count) in `bones` |

[main]\
open fun [skinning](skinning.md)(bones: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [RenderableManager.Builder](index.md)

#### Parameters

main

| | |
|---|---|
| bones | array containing bones data |

[main]\
open fun [skinning](skinning.md)(boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)
