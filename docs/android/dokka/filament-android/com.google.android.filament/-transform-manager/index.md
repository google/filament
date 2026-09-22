//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)

# TransformManager

[main]\
open class [TransformManager](index.md)

TransformManager is used to add transform components to entities. 

A Transform component gives an entity a position and orientation in space in the coordinate space of its parent transform. The TransformManager takes care of computing the world-space transform of each component (i.e. its transform relative to the root).

# Creation and destruction

A transform component is created using TransformManager::create() and destroyed by calling TransformManager::destroy().

```kotlin

 filament::Engine* engine = filament::Engine::create();
 utils::Entity object = utils::EntityManager.get().create();

 auto& tcm = engine->getTransformManager();

 // create the transform component
 tcm.create(object);

 // set its transform
 auto i = tcm.getInstance(object);
 tcm.setTransform(i, mat4f::translation({ 0, 0, -1 }));

 // destroy the transform component
 tcm.destroy(object);

```

## Functions

| Name | Summary |
|---|---|
| [commitLocalTransformTransaction](commit-local-transform-transaction.md) | [main]<br>open fun [commitLocalTransformTransaction](commit-local-transform-transaction.md)()<br>Commits the currently open local transform transaction. |
| [create](create.md) | [main]<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>[main]<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Creates a transform component and associate it with the given entity. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys this component from the given entity, children are orphaned. |
| [empty](empty.md) | [main]<br>open fun [empty](empty.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [getAllEntities](get-all-entities.md) | [main]<br>open fun [getAllEntities](get-all-entities.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;<br>[main]<br>open fun [getAllEntities](get-all-entities.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;<br>Retrieve the Entities of all the components of this manager. |
| [getChildCount](get-child-count.md) | [main]<br>open fun [getChildCount](get-child-count.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of children of a transform component. |
| [getChildren](get-children.md) | [main]<br>open fun [getChildren](get-children.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), children: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Gets a list of children for a transform component. |
| [getComponentCount](get-component-count.md) | [main]<br>open fun [getComponentCount](get-component-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getEntity](get-entity.md) | [main]<br>open fun [getEntity](get-entity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Retrieve the `Entity` of the component from its `Instance`. |
| [getInstance](get-instance.md) | [main]<br>open fun [getInstance](get-instance.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Gets an Instance representing the transform component associated with the given Entity. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getParent](get-parent.md) | [main]<br>open fun [getParent](get-parent.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the parent of a transform component, or the null entity if it is a root. |
| [getTransform](get-transform.md) | [main]<br>open fun [getTransform](get-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the local transform of a transform component. |
| [getTransformAccurate](get-transform-accurate.md) | [main]<br>open fun [getTransformAccurate](get-transform-accurate.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Returns the local transform of a transform component. |
| [getWorldTransform](get-world-transform.md) | [main]<br>open fun [getWorldTransform](get-world-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Return the world transform of a transform component. |
| [getWorldTransformAccurate](get-world-transform-accurate.md) | [main]<br>open fun [getWorldTransformAccurate](get-world-transform-accurate.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>Return the world transform of a transform component. |
| [hasComponent](has-component.md) | [main]<br>open fun [hasComponent](has-component.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a particular Entity is associated with a component of this TransformManager |
| [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md) | [main]<br>open fun [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the high precision translation mode is active. |
| [openLocalTransformTransaction](open-local-transform-transaction.md) | [main]<br>open fun [openLocalTransformTransaction](open-local-transform-transaction.md)()<br>Opens a local transform transaction. |
| [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md) | [main]<br>open fun [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Enables or disable the accurate translation mode. |
| [setParent](set-parent.md) | [main]<br>open fun [setParent](set-parent.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), newParent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Re-parents an entity to a new one. |
| [setTransform](set-transform.md) | [main]<br>open fun [setTransform](set-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>Sets a local transform of a transform component and keeps double precision translation.<br>[main]<br>open fun [setTransform](set-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Sets a local transform of a transform component. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [TransformManager](index.md) |
