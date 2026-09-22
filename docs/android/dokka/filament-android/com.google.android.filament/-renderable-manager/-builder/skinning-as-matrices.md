//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[skinningAsMatrices](skinning-as-matrices.md)

# skinningAsMatrices

[main]\
open fun [skinningAsMatrices](skinning-as-matrices.md)(boneCount: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be disabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the \c BONE_WEIGHTS slot (float4).

See also RenderableManager::setBones(), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| boneCount | number of elements (structured element count) in `transforms` |
| transforms | the initial set of transforms (one for each bone) |

[main]\
open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;, offset: Int, boneCount: Int): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be disabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the \c BONE_WEIGHTS slot (float4).

See also RenderableManager::setBones(), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| transforms | the initial set of transforms (one for each bone) |
| offset | offset in elements (structured element count) in `transforms` to skip |
| boneCount | number of elements (structured element count) in `transforms` |

[main]\
open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;, boneCount: Int): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be disabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the \c BONE_WEIGHTS slot (float4).

See also RenderableManager::setBones(), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| transforms | the initial set of transforms (one for each bone) |
| boneCount | number of elements (structured element count) in `transforms` |

[main]\
open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be disabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the \c BONE_WEIGHTS slot (float4).

See also RenderableManager::setBones(), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| transforms | the initial set of transforms (one for each bone) |
