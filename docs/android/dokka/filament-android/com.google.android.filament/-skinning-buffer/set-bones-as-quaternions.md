//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SkinningBuffer](index.md)/[setBonesAsQuaternions](set-bones-as-quaternions.md)

# setBonesAsQuaternions

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [offset, offset + count).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | buffer of Bone transforms |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (not bytes) in the SkinningBuffer |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [0, count).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | buffer of Bone transforms |
| count | number of elements (structured element count) in `transforms` |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, arrayOffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [offset, offset + count).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | array of Bone transforms |
| arrayOffset | offset in elements (structured element count) in `transforms` to skip |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (not bytes) in the SkinningBuffer |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [offset, offset + count).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | array of Bone transforms |
| count | number of elements (structured element count) in `transforms` |
| offset | offset in elements (not bytes) in the SkinningBuffer |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [0, count).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | array of Bone transforms |
| count | number of elements (structured element count) in `transforms` |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Updates the bone transforms in the range [0, transforms.length / 8).

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this SkinningBuffer with. |
| transforms | array of Bone transforms |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |
