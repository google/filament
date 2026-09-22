//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[instances](instances.md)

# instances

[main]\
open fun [instances](instances.md)(instanceCount: Int): [RenderableManager.Builder](index.md)

Specifies the number of draw instances of this renderable. 

The default is 1 instance and the maximum number of instances allowed is 32767. 0 is invalid.

All instances are culled using the same bounding box, so care must be taken to make sure all instances render inside the specified bounding box.

The material must set its `instanced` parameter to `true` in order to use getInstanceIndex() in the vertex or fragment shader to get the instance index and possibly adjust the position or transform.

#### Parameters

main

| | |
|---|---|
| instanceCount | the number of instances silently clamped between 1 and 32767. |

[main]\
open fun [instances](instances.md)(instanceCount: Int, instanceBuffer: [InstanceBuffer](../../-instance-buffer/index.md)): [RenderableManager.Builder](index.md)

Specifies the number of draw instances of this renderable and an \c InstanceBuffer containing their local transforms. 

The default is 1 instance and the maximum number of instances allowed when supplying transforms is given by \c Engine::getMaxAutomaticInstances (64 on most platforms). 0 is invalid. The \c InstanceBuffer must not be destroyed before this renderable.

All instances are culled using the same bounding box, so care must be taken to make sure all instances render inside the specified bounding box.

The material must set its `instanced` parameter to `true` in order to use \c getInstanceIndex() in the vertex or fragment shader to get the instance index.

Only the \c VERTEX_DOMAIN_OBJECT vertex domain is supported.

The local transforms of each instance can be updated with \c InstanceBuffer::setLocalTransforms.

#### Parameters

main

| | |
|---|---|
| instanceCount | the number of instances, silently clamped between 1 and the result of Engine::getMaxAutomaticInstances(). |
| instanceBuffer | an InstanceBuffer containing at least instanceCount transforms |

#### See also

| |
|---|
| [InstanceBuffer](../../-instance-buffer/index.md) |
| instances(int, mat4f) |
