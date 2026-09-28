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
| [getInstanceCount](get-instance-count.md) | [main]<br>open fun [getInstanceCount](get-instance-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the instance count specified when building this InstanceBuffer. |
| [getLocalTransform](get-local-transform.md) | [main]<br>open fun [getLocalTransform](get-local-transform.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the local transform for a given instance. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setLocalTransforms](set-local-transforms.md) | [main]<br>open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [setLocalTransforms](set-local-transforms.md)(localTransforms: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Sets the local transform for each instance. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [InstanceBuffer](index.md) |
