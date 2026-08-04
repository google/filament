//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)

# TransformManager

[main]\
open class [TransformManager](index.md)

`TransformManager` is used to add transform components to entities. 

A transform component gives an entity a position and orientation in space in the coordinate space of its parent transform. The `TransformManager` takes care of computing the world-space transform of each component (i.e. its transform relative to the root).

# Creation and destruction

 A transform component is created using create and destroyed by calling [destroy](destroy.md). ```kotlin
 Engine engine = Engine.create();
 EntityManager entityManager = EntityManager().get();
 int object = entityManager.create();

 TransformManager tcm = engine.getTransformManager();

 // create the transform component
 tcm.create(object);

 // set its transform
 float[] transform = ...; // transform to set
 EntityInstance i = tcm.getInstance(object);
 tcm.setTransform(i, transform));

 // destroy the transform component
 tcm.destroy(object);

```

## Functions

| Name | Summary |
|---|---|
| [commitLocalTransformTransaction](commit-local-transform-transaction.md) | [main]<br>open fun [commitLocalTransformTransaction](commit-local-transform-transaction.md)()<br>Commits the currently open local transform transaction. |
| [create](create.md) | [main]<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Creates a transform component and associates it with the given entity.<br>[main]<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Creates a transform component with a parent and associates it with the given entity. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys this component from the given entity, children are orphaned. |
| [getChildCount](get-child-count.md) | [main]<br>open fun [getChildCount](get-child-count.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of children of an [EntityInstance](../-entity-instance/index.md). |
| [getChildren](get-children.md) | [main]<br>open fun [getChildren](get-children.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outEntities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;<br>Gets a list of children for a transform component. |
| [getInstance](get-instance.md) | [main]<br>open fun [getInstance](get-instance.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Gets an [EntityInstance](../-entity-instance/index.md) representing the transform component associated with the given [Entity](../-entity/index.md). |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getParent](get-parent.md) | [main]<br>open fun [getParent](get-parent.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the actual parent entity of an [EntityInstance](../-entity-instance/index.md) originally defined by [setParent](set-parent.md). |
| [getTransform](get-transform.md) | [main]<br>open fun [getTransform](get-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outLocalTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>open fun [getTransform](get-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outLocalTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the local transform of a transform component. |
| [getWorldTransform](get-world-transform.md) | [main]<br>open fun [getWorldTransform](get-world-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outWorldTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;<br>open fun [getWorldTransform](get-world-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outWorldTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the world transform of a transform component. |
| [hasComponent](has-component.md) | [main]<br>open fun [hasComponent](has-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a particular [Entity](../-entity/index.md) is associated with a component of this `TransformManager` |
| [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md) | [main]<br>open fun [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the high precision translation mode is active. |
| [openLocalTransformTransaction](open-local-transform-transaction.md) | [main]<br>open fun [openLocalTransformTransaction](open-local-transform-transaction.md)()<br>Opens a local transform transaction. |
| [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md) | [main]<br>open fun [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Enables or disable the accurate translation mode. |
| [setParent](set-parent.md) | [main]<br>open fun [setParent](set-parent.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), newParent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Re-parents an entity to a new one. |
| [setTransform](set-transform.md) | [main]<br>open fun [setTransform](set-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)<br>open fun [setTransform](set-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Sets a local transform of a transform component. |
