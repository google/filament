//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[InstanceBuffer](index.md)

# InstanceBuffer

open class [InstanceBuffer](index.md)

InstanceBuffer holds draw (GPU) instance transforms. 

These can be provided to a renderable to &quot;offset&quot; each draw instance.

#### See also

| |
|---|
| [RenderableManager.Builder](../-renderable-manager/-builder/instances.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getInstanceCount](get-instance-count.md) | [main]<br>open fun [getInstanceCount](get-instance-count.md)(): Int<br>Returns the instance count specified when building this InstanceBuffer. |
| [getLocalTransform](get-local-transform.md) | [main]<br>open fun [getLocalTransform](get-local-transform.md)(index: Int, out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Returns the local transform for a given instance. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [setLocalTransforms](set-local-transforms.md) | [main]<br>open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: Array&lt;Float&gt;, count: Int)<br>open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: Array&lt;Float&gt;, count: Int, offset: Int)<br>Sets the local transform for each instance. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [InstanceBuffer](index.md) |
