//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)

# Scene

open class [Scene](index.md)

A Scene is a flat container of Renderable and Light instances. 

A Scene doesn't provide a hierarchy of Renderable objects, i.e.: it's not a scene-graph. However, it manages the list of objects to render and the list of lights. Renderable and Light objects can be added or removed from a Scene at any time.

A Renderable *must* be added to a Scene in order to be rendered, and the Scene must be provided to a View.

# Creation and Destruction

A Scene is created using Engine.createScene() and destroyed using Engine.destroy(const Scene*).

```kotlin

#include <filament/Scene.h>
#include <filament/Engine.h>
using namespace filament;

Engine* engine = Engine::create();

Scene* scene = engine->createScene();
engine->destroy(&scene);

```

#### See also

| |
|---|
| [View](../-view/index.md) |
| Renderable |
| Light |

## Types

| Name | Summary |
|---|---|
| [ForEachCallback](-for-each-callback/index.md) | [main]<br>@[FunctionalInterface](https://developer.android.com/reference/kotlin/java/lang/FunctionalInterface.html)<br>interface [ForEachCallback](-for-each-callback/index.md) |

## Functions

| Name | Summary |
|---|---|
| [addEntities](add-entities.md) | [main]<br>open fun [addEntities](add-entities.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Adds a list of entities to the Scene. |
| [addEntity](add-entity.md) | [main]<br>open fun [addEntity](add-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Adds an Entity to the Scene. |
| [forEach](for-each.md) | [main]<br>open fun [forEach](for-each.md)(functor: [Scene.ForEachCallback](-for-each-callback/index.md))<br>Invokes user functor on each entity in the scene. |
| [getEntityCount](get-entity-count.md) | [main]<br>open fun [getEntityCount](get-entity-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the total number of Entities in the Scene, whether alive or not. |
| [getIndirectLight](get-indirect-light.md) | [main]<br>open fun [getIndirectLight](get-indirect-light.md)(): [IndirectLight](../-indirect-light/index.md)<br>Get the IndirectLight or nullptr if none is set. |
| [getLightCount](get-light-count.md) | [main]<br>open fun [getLightCount](get-light-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of active (alive) Light objects in the Scene. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRenderableCount](get-renderable-count.md) | [main]<br>open fun [getRenderableCount](get-renderable-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of active (alive) Renderable objects in the Scene. |
| [getSkybox](get-skybox.md) | [main]<br>open fun [getSkybox](get-skybox.md)(): [Skybox](../-skybox/index.md)<br>Returns the Skybox associated with the Scene. |
| [hasEntity](has-entity.md) | [main]<br>open fun [hasEntity](has-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns true if the given entity is in the Scene. |
| [remove](remove.md) | [main]<br>open fun [remove](remove.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Removes the Renderable from the Scene. |
| [removeAllEntities](remove-all-entities.md) | [main]<br>open fun [removeAllEntities](remove-all-entities.md)()<br>Remove all entities to the Scene. |
| [removeEntities](remove-entities.md) | [main]<br>open fun [removeEntities](remove-entities.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Removes a list of entities to the Scene. |
| [setIndirectLight](set-indirect-light.md) | [main]<br>open fun [setIndirectLight](set-indirect-light.md)(ibl: [IndirectLight](../-indirect-light/index.md))<br>Set the IndirectLight to use when rendering the Scene. |
| [setSkybox](set-skybox.md) | [main]<br>open fun [setSkybox](set-skybox.md)(skybox: [Skybox](../-skybox/index.md))<br>Sets the Skybox. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Scene](index.md) |
