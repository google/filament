//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Adds renderable components to entities using a builder pattern.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor(count: Int)<br>Creates a builder for renderable components. |

## Types

| Name | Summary |
|---|---|
| [GeometryType](-geometry-type/index.md) | [main]<br>enum [GeometryType](-geometry-type/index.md)<br>Type of geometry for a Renderable |
| [MorphType](-morph-type/index.md) | [main]<br>enum [MorphType](-morph-type/index.md)<br>Type of morphing for a Renderable. |
| [Result](-result/index.md) | [main]<br>enum [Result](-result/index.md) |

## Functions

| Name | Summary |
|---|---|
| [blendOrder](blend-order.md) | [main]<br>open fun [blendOrder](blend-order.md)(primitiveIndex: Int, blendOrder: Int): [RenderableManager.Builder](index.md)<br>Sets the drawing order for blended primitives. |
| [boneIndicesAndWeights](bone-indices-and-weights.md) | [main]<br>open fun [boneIndicesAndWeights](bone-indices-and-weights.md)(primitiveIndex: Int, indicesAndWeights: Array&lt;Float&gt;, count: Int, bonesPerVertex: Int): [RenderableManager.Builder](index.md)<br>Define bone indices and weights &quot;pairs&quot; for vertex skinning as a float2. |
| [boundingBox](bounding-box.md) | [main]<br>open fun [boundingBox](bounding-box.md)(axisAlignedBoundingBox: [Box](../../-box/index.md)): [RenderableManager.Builder](index.md)<br>The axis-aligned bounding box of the renderable. |
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: Int)<br>Adds the Renderable component to an entity. |
| [castShadows](cast-shadows.md) | [main]<br>open fun [castShadows](cast-shadows.md)(enable: Boolean): [RenderableManager.Builder](index.md)<br>Controls if this renderable casts shadows, false by default. |
| [channel](channel.md) | [main]<br>open fun [channel](channel.md)(channel: Int): [RenderableManager.Builder](index.md)<br>Set the channel this renderable is associated to. |
| [culling](culling.md) | [main]<br>open fun [culling](culling.md)(enable: Boolean): [RenderableManager.Builder](index.md)<br>Controls frustum culling, true by default. |
| [enableSkinningBuffers](enable-skinning-buffers.md) | [main]<br>open fun [enableSkinningBuffers](enable-skinning-buffers.md)(): [RenderableManager.Builder](index.md)<br>open fun [enableSkinningBuffers](enable-skinning-buffers.md)(enabled: Boolean): [RenderableManager.Builder](index.md)<br>Allows bones to be swapped out and shared using SkinningBuffer. |
| [fog](fog.md) | [main]<br>open fun [fog](fog.md)(): [RenderableManager.Builder](index.md)<br>open fun [fog](fog.md)(enabled: Boolean): [RenderableManager.Builder](index.md)<br>Controls if this renderable is affected by the large-scale fog. |
| [geometry](geometry.md) | [main]<br>open fun [geometry](geometry.md)(index: Int, type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md)): [RenderableManager.Builder](index.md)<br>open fun [geometry](geometry.md)(index: Int, type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md)): [RenderableManager.Builder](index.md)<br>open fun [geometry](geometry.md)(index: Int, type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: Int, count: Int): [RenderableManager.Builder](index.md)<br>[main]<br>open fun [geometry](geometry.md)(index: Int, type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), offset: Int, count: Int): [RenderableManager.Builder](index.md)<br>open fun [geometry](geometry.md)(index: Int, type: [RenderableManager.PrimitiveType](../-primitive-type/index.md), vertices: [VertexBuffer](../../-vertex-buffer/index.md), indices: [IndexBuffer](../../-index-buffer/index.md), offset: Int, minIndex: Int, maxIndex: Int, count: Int): [RenderableManager.Builder](index.md)<br>Specifies the geometry data for a primitive. |
| [geometryType](geometry-type.md) | [main]<br>open fun [geometryType](geometry-type.md)(type: [RenderableManager.Builder.GeometryType](-geometry-type/index.md)): [RenderableManager.Builder](index.md)<br>Specify the type of geometry for this renderable. |
| [globalBlendOrderEnabled](global-blend-order-enabled.md) | [main]<br>open fun [globalBlendOrderEnabled](global-blend-order-enabled.md)(primitiveIndex: Int, enabled: Boolean): [RenderableManager.Builder](index.md)<br>Sets whether the blend order is global or local to this Renderable (by default). |
| [instances](instances.md) | [main]<br>open fun [instances](instances.md)(instanceCount: Int): [RenderableManager.Builder](index.md)<br>Specifies the number of draw instances of this renderable.<br>[main]<br>open fun [instances](instances.md)(instanceCount: Int, instanceBuffer: [InstanceBuffer](../../-instance-buffer/index.md)): [RenderableManager.Builder](index.md)<br>Specifies the number of draw instances of this renderable and an \c InstanceBuffer containing their local transforms. |
| [layerMask](layer-mask.md) | [main]<br>open fun [layerMask](layer-mask.md)(select: Int, values: Int): [RenderableManager.Builder](index.md)<br>Sets bits in a visibility mask. |
| [lightChannel](light-channel.md) | [main]<br>open fun [lightChannel](light-channel.md)(channel: Int): [RenderableManager.Builder](index.md)<br>open fun [lightChannel](light-channel.md)(channel: Int, enable: Boolean): [RenderableManager.Builder](index.md)<br>Enables or disables a light channel. |
| [material](material.md) | [main]<br>open fun [material](material.md)(index: Int, materialInstance: [MaterialInstance](../../-material-instance/index.md)): [RenderableManager.Builder](index.md)<br>Binds a material instance to the specified primitive. |
| [morphing](morphing.md) | [main]<br>open fun [morphing](morphing.md)(morphTargetBuffer: [MorphTargetBuffer](../../-morph-target-buffer/index.md)): [RenderableManager.Builder](index.md)<br>Controls if the renderable has vertex morphing targets, zero by default.<br>[main]<br>open fun [morphing](morphing.md)(targetCount: Int): [RenderableManager.Builder](index.md)<br>Controls if the renderable has legacy vertex morphing targets, zero by default.<br>[main]<br>open fun [morphing](morphing.md)(level: Int, primitiveIndex: Int, offset: Int): [RenderableManager.Builder](index.md)<br>Specifies the the range of the MorphTargetBuffer to use with this primitive. |
| [priority](priority.md) | [main]<br>open fun [priority](priority.md)(priority: Int): [RenderableManager.Builder](index.md)<br>Provides coarse-grained control over draw order. |
| [receiveShadows](receive-shadows.md) | [main]<br>open fun [receiveShadows](receive-shadows.md)(enable: Boolean): [RenderableManager.Builder](index.md)<br>Controls if this renderable receives shadows, true by default. |
| [screenSpaceContactShadows](screen-space-contact-shadows.md) | [main]<br>open fun [screenSpaceContactShadows](screen-space-contact-shadows.md)(enable: Boolean): [RenderableManager.Builder](index.md)<br>Controls if this renderable uses screen-space contact shadows. |
| [skinning](skinning.md) | [main]<br>open fun [skinning](skinning.md)(bones: Array&lt;Float&gt;): [RenderableManager.Builder](index.md)<br>open fun [skinning](skinning.md)(boneCount: Int): [RenderableManager.Builder](index.md)<br>open fun [skinning](skinning.md)(bones: Array&lt;Float&gt;, boneCount: Int): [RenderableManager.Builder](index.md)<br>open fun [skinning](skinning.md)(boneCount: Int, bones: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [RenderableManager.Builder](index.md)<br>open fun [skinning](skinning.md)(bones: Array&lt;Float&gt;, offset: Int, boneCount: Int): [RenderableManager.Builder](index.md)<br>[main]<br>open fun [skinning](skinning.md)(skinningBuffer: [SkinningBuffer](../../-skinning-buffer/index.md), count: Int, offset: Int): [RenderableManager.Builder](index.md)<br>Enables GPU vertex skinning for up to 255 bones, 0 by default. |
| [skinningAsMatrices](skinning-as-matrices.md) | [main]<br>open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;): [RenderableManager.Builder](index.md)<br>open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;, boneCount: Int): [RenderableManager.Builder](index.md)<br>open fun [skinningAsMatrices](skinning-as-matrices.md)(boneCount: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [RenderableManager.Builder](index.md)<br>open fun [skinningAsMatrices](skinning-as-matrices.md)(transforms: Array&lt;Float&gt;, offset: Int, boneCount: Int): [RenderableManager.Builder](index.md)<br>Enables GPU vertex skinning for up to 255 bones, 0 by default. |
