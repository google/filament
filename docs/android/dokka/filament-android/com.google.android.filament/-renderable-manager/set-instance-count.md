//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setInstanceCount](set-instance-count.md)

# setInstanceCount

[main]\
open fun [setInstanceCount](set-instance-count.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), instanceCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Changes the number of draw instances of this renderable. 

For renderables using manual instancing (i.e. built with Builder::instances(size_t)), the instance count is silently clamped between 0 and 32767.

For renderables built with an InstanceBuffer (i.e. Builder::instances(size_t, InstanceBuffer*)), the instance count must be less than or equal to both the InstanceBuffer's instance count and Engine::getMaxAutomaticInstances().

An instance count of 0 means the renderable is not drawn in any pass (including shadow passes), but its component is otherwise left untouched. This allows, for instance, to allocate resources for a maximum number of instances up front and only draw the ones currently needed.

All instances are culled using the same bounding box, so care must be taken to make sure all instances render inside the renderable's bounding box.

#### Parameters

main

| | |
|---|---|
| instance | Instance of the component obtained from getInstance(). |
| instanceCount | The new number of instances. |

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |
| [getInstanceCount](get-instance-count.md) |
