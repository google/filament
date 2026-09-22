//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[create](create.md)

# create

[main]\
open fun [create](create.md)(entity: Int, parent: Int, localTransform: Array&lt;Float&gt;)

Creates a transform component and associate it with the given entity.

#### Parameters

main

| | |
|---|---|
| entity | An Entity to associate a transform component to. |
| parent | The Instance of the parent transform, or Instance{} if no parent. |
| localTransform | The transform to initialize the transform component with. This is always relative to the parent.<br>If this component already exists on the given entity, it is first destroyed as if destroy(utils::Entity e) was called. |

#### See also

| |
|---|
| [destroy](destroy.md) |

[main]\
open fun [create](create.md)(entity: Int, parent: Int, localTransform: Array&lt;Double&gt;)

open fun [create](create.md)(entity: Int)

open fun [create](create.md)(entity: Int, parent: Int)
