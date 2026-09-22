//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[InstanceBuffer](index.md)/[setLocalTransforms](set-local-transforms.md)

# setLocalTransforms

[main]\
open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: Array&lt;Float&gt;, count: Int)

Sets the local transform for each instance. 

Each local transform is relative to the transform of the associated renderable. This forms a parent-child relationship between the renderable and its instances, so adjusting the renderable's transform will affect all instances.

#### Parameters

main

| | |
|---|---|
| localTransforms | an array of math::mat4f with length count, need not outlive this call |
| count | the number of local transforms |

[main]\
open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: Array&lt;Float&gt;, count: Int, offset: Int)

Sets the local transform for each instance. 

Each local transform is relative to the transform of the associated renderable. This forms a parent-child relationship between the renderable and its instances, so adjusting the renderable's transform will affect all instances.

#### Parameters

main

| | |
|---|---|
| localTransforms | an array of math::mat4f with length count, need not outlive this call |
| count | the number of local transforms |
| offset | index of the first instance to set local transforms |
