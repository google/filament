//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)

# RenderableManager

[main]\
open class [RenderableManager](index.md)

Factory and manager for \em renderables, which are entities that can be drawn. 

Renderables are bundles of \em primitives, each of which has its own geometry and material. All primitives in a particular renderable share a set of rendering attributes, such as whether they cast shadows or use vertex skinning.

Usage example:

```kotlin

auto renderable = utils::EntityManager::get().create();

RenderableManager::Builder(1)
        .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
        .material(0, matInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertBuffer, indBuffer, 0, 3)
        .receiveShadows(false)
        .build(engine, renderable);

scene->addEntity(renderable);

```

To modify the state of an existing renderable, clients should first use RenderableManager to get a temporary handle called an \em instance. The instance can then be used to get or set the renderable's state. Please note that instances are ephemeral; clients should store entities, not instances.

- For details about constructing renderables, see RenderableManager::Builder.
- To associate a 4x4 transform with an entity, see TransformManager.
- To associate a human-readable label with an entity, see utils::NameComponentManager.

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Adds renderable components to entities using a builder pattern. |
| [PrimitiveType](-primitive-type/index.md) | [main]<br>enum [PrimitiveType](-primitive-type/index.md)<br>Primitive types |

## Functions

| Name | Summary |
|---|---|
| [clearMaterialInstanceAt](clear-material-instance-at.md) | [main]<br>open fun [clearMaterialInstanceAt](clear-material-instance-at.md)(instance: Int, primitiveIndex: Int)<br>Clear the MaterialInstance for the given primitive. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(e: Int)<br>Destroys the renderable component in the given entity. |
| [empty](empty.md) | [main]<br>open fun [empty](empty.md)(): Boolean |
| [getAllEntities](get-all-entities.md) | [main]<br>open fun [getAllEntities](get-all-entities.md)(): Array&lt;Int&gt;<br>[main]<br>open fun [getAllEntities](get-all-entities.md)(out: Array&lt;Int&gt;): Array&lt;Int&gt;<br>Retrieve the Entities of all the components of this manager. |
| [getAxisAlignedBoundingBox](get-axis-aligned-bounding-box.md) | [main]<br>open fun [getAxisAlignedBoundingBox](get-axis-aligned-bounding-box.md)(instance: Int): [Box](../-box/index.md)<br>[main]<br>open fun [getAxisAlignedBoundingBox](get-axis-aligned-bounding-box.md)(instance: Int, out: [Box](../-box/index.md)): [Box](../-box/index.md)<br>Gets the bounding box used for frustum culling. |
| [getBlendOrderAt](get-blend-order-at.md) | [main]<br>open fun [getBlendOrderAt](get-blend-order-at.md)(instance: Int, primitiveIndex: Int): Int<br>Get the drawing order for blended primitives. |
| [getChannel](get-channel.md) | [main]<br>open fun [getChannel](get-channel.md)(instance: Int): Int<br>Get the channel a renderable is associated to. |
| [getComponentCount](get-component-count.md) | [main]<br>open fun [getComponentCount](get-component-count.md)(): Int |
| [getEnabledAttributesAt](get-enabled-attributes-at.md) | [main]<br>open fun [getEnabledAttributesAt](get-enabled-attributes-at.md)(instance: Int, primitiveIndex: Int): [Set](https://developer.android.com/reference/kotlin/java/util/Set.html)&lt;[VertexBuffer.VertexAttribute](../-vertex-buffer/-vertex-attribute/index.md)&gt;<br>Retrieves the set of enabled attribute slots in the given primitive's VertexBuffer. |
| [getEntity](get-entity.md) | [main]<br>open fun [getEntity](get-entity.md)(i: Int): Int<br>Retrieve the `Entity` of the component from its `Instance`. |
| [getFogEnabled](get-fog-enabled.md) | [main]<br>open fun [getFogEnabled](get-fog-enabled.md)(instance: Int): Boolean<br>Returns whether large-scale fog is enabled for this renderable. |
| [getInstance](get-instance.md) | [main]<br>open fun [getInstance](get-instance.md)(e: Int): Int<br>Gets a temporary handle that can be used to access the renderable state. |
| [getInstanceCount](get-instance-count.md) | [main]<br>open fun [getInstanceCount](get-instance-count.md)(instance: Int): Int<br>Returns the number of instances for this renderable. |
| [getLayerMask](get-layer-mask.md) | [main]<br>open fun [getLayerMask](get-layer-mask.md)(instance: Int): Int<br>Get the visibility bits. |
| [getLightChannel](get-light-channel.md) | [main]<br>open fun [getLightChannel](get-light-channel.md)(instance: Int, channel: Int): Boolean<br>Returns whether a light channel is enabled on a specified renderable. |
| [getMaterialInstanceAt](get-material-instance-at.md) | [main]<br>open fun [getMaterialInstanceAt](get-material-instance-at.md)(instance: Int, primitiveIndex: Int): [MaterialInstance](../-material-instance/index.md)<br>Retrieves the material instance that is bound to the given primitive. |
| [getMorphTargetBuffer](get-morph-target-buffer.md) | [main]<br>open fun [getMorphTargetBuffer](get-morph-target-buffer.md)(instance: Int): [MorphTargetBuffer](../-morph-target-buffer/index.md)<br>Get a MorphTargetBuffer to the given renderable or null if it doesn't exist. |
| [getMorphTargetCount](get-morph-target-count.md) | [main]<br>open fun [getMorphTargetCount](get-morph-target-count.md)(instance: Int): Int<br>Gets the number of morphing in the given entity. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getPrimitiveCount](get-primitive-count.md) | [main]<br>open fun [getPrimitiveCount](get-primitive-count.md)(instance: Int): Int<br>Gets the immutable number of primitives in the given renderable. |
| [getPriority](get-priority.md) | [main]<br>open fun [getPriority](get-priority.md)(instance: Int): Int<br>Get the coarse-level draw ordering. |
| [hasComponent](has-component.md) | [main]<br>open fun [hasComponent](has-component.md)(e: Int): Boolean<br>Checks if the given entity already has a renderable component. |
| [isCullingEnabled](is-culling-enabled.md) | [main]<br>open fun [isCullingEnabled](is-culling-enabled.md)(instance: Int): Boolean<br>Get whether or not frustum culling is on. |
| [isGlobalBlendOrderEnabledAt](is-global-blend-order-enabled-at.md) | [main]<br>open fun [isGlobalBlendOrderEnabledAt](is-global-blend-order-enabled-at.md)(instance: Int, primitiveIndex: Int): Boolean<br>Get whether the blend order is global or local to this Renderable (by default). |
| [isScreenSpaceContactShadowsEnabled](is-screen-space-contact-shadows-enabled.md) | [main]<br>open fun [isScreenSpaceContactShadowsEnabled](is-screen-space-contact-shadows-enabled.md)(instance: Int): Boolean<br>Checks if the renderable can use screen-space contact shadows. |
| [isShadowCaster](is-shadow-caster.md) | [main]<br>open fun [isShadowCaster](is-shadow-caster.md)(instance: Int): Boolean<br>Checks if the renderable can cast shadows. |
| [isShadowReceiver](is-shadow-receiver.md) | [main]<br>open fun [isShadowReceiver](is-shadow-receiver.md)(instance: Int): Boolean<br>Checks if the renderable can receive shadows. |
| [setAxisAlignedBoundingBox](set-axis-aligned-bounding-box.md) | [main]<br>open fun [setAxisAlignedBoundingBox](set-axis-aligned-bounding-box.md)(instance: Int, aabb: [Box](../-box/index.md))<br>Changes the bounding box used for frustum culling. |
| [setBlendOrderAt](set-blend-order-at.md) | [main]<br>open fun [setBlendOrderAt](set-blend-order-at.md)(instance: Int, primitiveIndex: Int, order: Int)<br>Changes the drawing order for blended primitives. |
| [setBonesAsMatrices](set-bones-as-matrices.md) | [main]<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: Array&lt;Float&gt;)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int, offset: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int, offset: Int)<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(instance: Int, transforms: Array&lt;Float&gt;, arrayOffset: Int, count: Int, offset: Int) |
| [setBonesAsQuaternions](set-bones-as-quaternions.md) | [main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;)<br>Updates the bone transforms in the range [0, transforms.length / 8).<br>[main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int)<br>Updates the bone transforms in the range [0, count).<br>[main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, count: Int, offset: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), count: Int, offset: Int)<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(instance: Int, transforms: Array&lt;Float&gt;, arrayOffset: Int, count: Int, offset: Int)<br>Updates the bone transforms in the range [offset, offset + count). |
| [setCastShadows](set-cast-shadows.md) | [main]<br>open fun [setCastShadows](set-cast-shadows.md)(instance: Int, enable: Boolean)<br>Changes whether or not the renderable casts shadows. |
| [setChannel](set-channel.md) | [main]<br>open fun [setChannel](set-channel.md)(instance: Int, channel: Int)<br>Changes the channel a renderable is associated to. |
| [setCulling](set-culling.md) | [main]<br>open fun [setCulling](set-culling.md)(instance: Int, enable: Boolean)<br>Changes whether or not frustum culling is on. |
| [setFogEnabled](set-fog-enabled.md) | [main]<br>open fun [setFogEnabled](set-fog-enabled.md)(instance: Int, enable: Boolean)<br>Changes whether or not the large-scale fog is applied to this renderable |
| [setGeometryAt](set-geometry-at.md) | [main]<br>open fun [setGeometryAt](set-geometry-at.md)(instance: Int, primitiveIndex: Int, type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md))<br>open fun [setGeometryAt](set-geometry-at.md)(instance: Int, primitiveIndex: Int, type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), indices: [IndexBuffer](../-index-buffer/index.md))<br>open fun [setGeometryAt](set-geometry-at.md)(instance: Int, primitiveIndex: Int, type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), offset: Int, count: Int)<br>open fun [setGeometryAt](set-geometry-at.md)(instance: Int, primitiveIndex: Int, type: [RenderableManager.PrimitiveType](-primitive-type/index.md), vertices: [VertexBuffer](../-vertex-buffer/index.md), indices: [IndexBuffer](../-index-buffer/index.md), offset: Int, count: Int)<br>Changes the geometry for the given primitive. |
| [setGlobalBlendOrderEnabledAt](set-global-blend-order-enabled-at.md) | [main]<br>open fun [setGlobalBlendOrderEnabledAt](set-global-blend-order-enabled-at.md)(instance: Int, primitiveIndex: Int, enabled: Boolean)<br>Changes whether the blend order is global or local to this Renderable (by default). |
| [setLayerMask](set-layer-mask.md) | [main]<br>open fun [setLayerMask](set-layer-mask.md)(instance: Int, select: Int, values: Int)<br>Changes the visibility bits. |
| [setLightChannel](set-light-channel.md) | [main]<br>open fun [setLightChannel](set-light-channel.md)(instance: Int, channel: Int, enable: Boolean)<br>Enables or disables a light channel. |
| [setMaterialInstanceAt](set-material-instance-at.md) | [main]<br>open fun [setMaterialInstanceAt](set-material-instance-at.md)(instance: Int, primitiveIndex: Int, materialInstance: [MaterialInstance](../-material-instance/index.md))<br>Changes the material instance binding for the given primitive. |
| [setMorphTargetBufferOffsetAt](set-morph-target-buffer-offset-at.md) | [main]<br>open fun [setMorphTargetBufferOffsetAt](set-morph-target-buffer-offset-at.md)(instance: Int, level: Int, primitiveIndex: Int, offset: Int)<br>Associates a MorphTargetBuffer to the given primitive. |
| [setMorphWeights](set-morph-weights.md) | [main]<br>open fun [setMorphWeights](set-morph-weights.md)(instance: Int, weights: Array&lt;Float&gt;)<br>open fun [setMorphWeights](set-morph-weights.md)(instance: Int, weights: Array&lt;Float&gt;, offset: Int)<br>Updates the vertex morphing weights on a renderable, all zeroes by default. |
| [setPriority](set-priority.md) | [main]<br>open fun [setPriority](set-priority.md)(instance: Int, priority: Int)<br>Changes the coarse-level draw ordering. |
| [setReceiveShadows](set-receive-shadows.md) | [main]<br>open fun [setReceiveShadows](set-receive-shadows.md)(instance: Int, enable: Boolean)<br>Changes whether or not the renderable can receive shadows. |
| [setScreenSpaceContactShadows](set-screen-space-contact-shadows.md) | [main]<br>open fun [setScreenSpaceContactShadows](set-screen-space-contact-shadows.md)(instance: Int, enable: Boolean)<br>Changes whether or not the renderable can use screen-space contact shadows. |
| [setSkinningBuffer](set-skinning-buffer.md) | [main]<br>open fun [setSkinningBuffer](set-skinning-buffer.md)(instance: Int, skinningBuffer: [SkinningBuffer](../-skinning-buffer/index.md), count: Int, offset: Int)<br>Associates a region of a SkinningBuffer to a renderable instance Note: due to hardware limitations offset + 256 must be smaller or equal to skinningBuffer->getBoneCount() |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [RenderableManager](index.md) |
