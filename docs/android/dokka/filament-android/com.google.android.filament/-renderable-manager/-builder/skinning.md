//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[skinning](skinning.md)

# skinning

[main]\
open fun [skinning](skinning.md)(skinningBuffer: [SkinningBuffer](../../-skinning-buffer/index.md), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be enabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the BONE_INDICES slot (uvec4) and the BONE_WEIGHTS slot (float4).

See also [setSkinningBuffer](../set-skinning-buffer.md), [setBonesAsMatrices](../../-skinning-buffer/set-bones-as-matrices.md) or [setBonesAsQuaternions](../../-skinning-buffer/set-bones-as-quaternions.md), which can be called on a per-frame basis to advance the animation.

#### Return

this `Builder` object for chaining calls

#### Parameters

main

| | |
|---|---|
| skinningBuffer | null to disable, otherwise the [SkinningBuffer](../../-skinning-buffer/index.md) to use |
| boneCount | 0 to disable, otherwise the number of bone transforms (up to 255) |
| offset | offset in the [SkinningBuffer](../../-skinning-buffer/index.md) |

#### See also

| |
|---|
| [setSkinningBuffer](../set-skinning-buffer.md) |
| [SkinningBuffer](../../-skinning-buffer/set-bones-as-quaternions.md) |

[main]\
open fun [skinning](skinning.md)(boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

[main]\
open fun [skinning](skinning.md)(boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bones: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [RenderableManager.Builder](index.md)

Enables GPU vertex skinning for up to 255 bones, 0 by default. 

Skinning Buffer mode must be disabled.

Each vertex can be affected by up to 4 bones simultaneously. The attached VertexBuffer must provide data in the `BONE_INDICES` slot (uvec4) and the `BONE_WEIGHTS` slot (float4).

See also [setBonesAsMatrices](../set-bones-as-matrices.md), which can be called on a per-frame basis to advance the animation.

#### Parameters

main

| | |
|---|---|
| boneCount | Number of bones associated with this component |
| bones | A FloatBuffer containing boneCount transforms. Each transform consists of 8 float. float 0 to 3 encode a unit quaternion w+ix+jy+kz stored as x,y,z,w. float 4 to 7 encode a translation stored as x,y,z,1 |

#### See also

| |
|---|
| [SkinningBuffer](../../-skinning-buffer/set-bones-as-matrices.md) |
