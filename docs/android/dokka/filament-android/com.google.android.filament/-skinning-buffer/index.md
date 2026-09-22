//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SkinningBuffer](index.md)

# SkinningBuffer

open class [SkinningBuffer](index.md)

SkinningBuffer is used to hold skinning data (bones). 

It is a simple wraper around a structured UBO.

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-skinning-buffer.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getBoneCount](get-bone-count.md) | [main]<br>open fun [getBoneCount](get-bone-count.md)(): Int<br>Returns the size of this SkinningBuffer in elements. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [setBonesAsMatrices](set-bones-as-matrices.md) | [main]<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;)<br>Updates the bone transforms in the range [0, transforms.length / 16).<br>[main]<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, count: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int)<br>Updates the bone transforms in the range [0, count).<br>[main]<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, count: Int, offset: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int, offset: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, arrayOffset: Int, count: Int, offset: Int)<br>Updates the bone transforms in the range [offset, offset + count). |
| [setBonesAsQuaternions](set-bones-as-quaternions.md) | [main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;)<br>Updates the bone transforms in the range [0, transforms.length / 8).<br>[main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, count: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int)<br>Updates the bone transforms in the range [0, count).<br>[main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, count: Int, offset: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int, offset: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), transforms: Array&lt;Float&gt;, arrayOffset: Int, count: Int, offset: Int)<br>Updates the bone transforms in the range [offset, offset + count). |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [SkinningBuffer](index.md) |
