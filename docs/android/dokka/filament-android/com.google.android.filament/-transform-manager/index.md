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
| [create](create.md) | [main]<br>open fun [create](create.md)(entity: Int)<br>open fun [create](create.md)(entity: Int, parent: Int)<br>open fun [create](create.md)(entity: Int, parent: Int, localTransform: Array&lt;Double&gt;)<br>[main]<br>open fun [create](create.md)(entity: Int, parent: Int, localTransform: Array&lt;Float&gt;)<br>Creates a transform component and associate it with the given entity. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(e: Int)<br>Destroys this component from the given entity, children are orphaned. |
| [empty](empty.md) | [main]<br>open fun [empty](empty.md)(): Boolean |
| [getAllEntities](get-all-entities.md) | [main]<br>open fun [getAllEntities](get-all-entities.md)(): Array&lt;Int&gt;<br>[main]<br>open fun [getAllEntities](get-all-entities.md)(out: Array&lt;Int&gt;): Array&lt;Int&gt;<br>Retrieve the Entities of all the components of this manager. |
| [getChildCount](get-child-count.md) | [main]<br>open fun [getChildCount](get-child-count.md)(i: Int): Int<br>Returns the number of children of a transform component. |
| [getChildren](get-children.md) | [main]<br>open fun [getChildren](get-children.md)(i: Int, children: Array&lt;Int&gt;, count: Int): Int<br>Gets a list of children for a transform component. |
| [getComponentCount](get-component-count.md) | [main]<br>open fun [getComponentCount](get-component-count.md)(): Int |
| [getEntity](get-entity.md) | [main]<br>open fun [getEntity](get-entity.md)(i: Int): Int<br>Retrieve the `Entity` of the component from its `Instance`. |
| [getInstance](get-instance.md) | [main]<br>open fun [getInstance](get-instance.md)(e: Int): Int<br>Gets an Instance representing the transform component associated with the given Entity. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getParent](get-parent.md) | [main]<br>open fun [getParent](get-parent.md)(i: Int): Int<br>Returns the parent of a transform component, or the null entity if it is a root. |
| [getTransform](get-transform.md) | [main]<br>open fun [getTransform](get-transform.md)(ci: Int, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Returns the local transform of a transform component. |
| [getTransformAccurate](get-transform-accurate.md) | [main]<br>open fun [getTransformAccurate](get-transform-accurate.md)(ci: Int, out: Array&lt;Double&gt;): Array&lt;Double&gt;<br>Returns the local transform of a transform component. |
| [getWorldTransform](get-world-transform.md) | [main]<br>open fun [getWorldTransform](get-world-transform.md)(ci: Int, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Return the world transform of a transform component. |
| [getWorldTransformAccurate](get-world-transform-accurate.md) | [main]<br>open fun [getWorldTransformAccurate](get-world-transform-accurate.md)(ci: Int, out: Array&lt;Double&gt;): Array&lt;Double&gt;<br>Return the world transform of a transform component. |
| [hasComponent](has-component.md) | [main]<br>open fun [hasComponent](has-component.md)(e: Int): Boolean<br>Returns whether a particular Entity is associated with a component of this TransformManager |
| [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md) | [main]<br>open fun [isAccurateTranslationsEnabled](is-accurate-translations-enabled.md)(): Boolean<br>Returns whether the high precision translation mode is active. |
| [openLocalTransformTransaction](open-local-transform-transaction.md) | [main]<br>open fun [openLocalTransformTransaction](open-local-transform-transaction.md)()<br>Opens a local transform transaction. |
| [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md) | [main]<br>open fun [setAccurateTranslationsEnabled](set-accurate-translations-enabled.md)(enable: Boolean)<br>Enables or disable the accurate translation mode. |
| [setParent](set-parent.md) | [main]<br>open fun [setParent](set-parent.md)(i: Int, newParent: Int)<br>Re-parents an entity to a new one. |
| [setTransform](set-transform.md) | [main]<br>open fun [setTransform](set-transform.md)(ci: Int, localTransform: Array&lt;Double&gt;)<br>Sets a local transform of a transform component and keeps double precision translation.<br>[main]<br>open fun [setTransform](set-transform.md)(ci: Int, localTransform: Array&lt;Float&gt;)<br>Sets a local transform of a transform component. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [TransformManager](index.md) |
