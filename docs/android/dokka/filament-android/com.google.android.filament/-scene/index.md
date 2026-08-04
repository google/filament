//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)

# Scene

open class [Scene](index.md)

A `Scene` is a flat container of [RenderableManager](../-renderable-manager/index.md) and [LightManager](../-light-manager/index.md) components. 

A `Scene` doesn't provide a hierarchy of objects, i.e.: it's not a scene-graph. However, it manages the list of objects to render and the list of lights. These can be added or removed from a `Scene` at any time. Moreover clients can use [TransformManager](../-transform-manager/index.md) to create a graph of transforms.

A [RenderableManager](../-renderable-manager/index.md) component **must** be added to a `Scene` in order to be rendered, and the `Scene` must be provided to a [View](../-view/index.md).

# Creation and Destruction

 A `Scene` is created using [createScene](../-engine/create-scene.md) and destroyed using [destroyScene](../-engine/destroy-scene.md).

#### See also

| |
|---|
| [View](../-view/index.md) |
| [LightManager](../-light-manager/index.md) |
| [RenderableManager](../-renderable-manager/index.md) |
| [TransformManager](../-transform-manager/index.md) |

## Types

| Name | Summary |
|---|---|
| [EntityProcessor](-entity-processor/index.md) | [main]<br>interface [EntityProcessor](-entity-processor/index.md) |

## Functions

| Name | Summary |
|---|---|
| [addEntities](add-entities.md) | [main]<br>open fun [addEntities](add-entities.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Adds a list of entities to the `Scene`. |
| [addEntity](add-entity.md) | [main]<br>open fun [addEntity](add-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Adds an [Entity](../-entity/index.md) to the `Scene`. |
| [forEach](for-each.md) | [main]<br>open fun [forEach](for-each.md)(entityProcessor: [Scene.EntityProcessor](-entity-processor/index.md))<br>Invokes user functor on each entity in the scene. |
| [getEntities](get-entities.md) | [main]<br>open fun [getEntities](get-entities.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;<br>Returns the list of all entities in the Scene in a newly allocated array.<br>[main]<br>open fun [getEntities](get-entities.md)(outArray: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;<br>Returns the list of all entities in the Scene. |
| [getEntityCount](get-entity-count.md) | [main]<br>open fun [getEntityCount](get-entity-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the total number of Entities in the `Scene`, whether alive or not. |
| [getIndirectLight](get-indirect-light.md) | [main]<br>open fun [getIndirectLight](get-indirect-light.md)(): [IndirectLight](../-indirect-light/index.md) |
| [getLightCount](get-light-count.md) | [main]<br>open fun [getLightCount](get-light-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of active (alive) [LightManager](../-light-manager/index.md) components in the `Scene`. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRenderableCount](get-renderable-count.md) | [main]<br>open fun [getRenderableCount](get-renderable-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of active (alive) [RenderableManager](../-renderable-manager/index.md) components in the `Scene`. |
| [getSkybox](get-skybox.md) | [main]<br>open fun [getSkybox](get-skybox.md)(): [Skybox](../-skybox/index.md) |
| [hasEntity](has-entity.md) | [main]<br>open fun [hasEntity](has-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns true if the given entity is in the Scene. |
| [remove](remove.md) | [main]<br>open fun [~~remove~~](remove.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [removeEntities](remove-entities.md) | [main]<br>open fun [removeEntities](remove-entities.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Removes a list of entities from the `Scene`. |
| [removeEntity](remove-entity.md) | [main]<br>open fun [removeEntity](remove-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Removes an [Entity](../-entity/index.md) from the `Scene`. |
| [setIndirectLight](set-indirect-light.md) | [main]<br>open fun [setIndirectLight](set-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md))<br>Sets the [IndirectLight](../-indirect-light/index.md) to use when rendering the `Scene`. |
| [setSkybox](set-skybox.md) | [main]<br>open fun [setSkybox](set-skybox.md)(skybox: [Skybox](../-skybox/index.md))<br>Sets the [Skybox](../-skybox/index.md). |
