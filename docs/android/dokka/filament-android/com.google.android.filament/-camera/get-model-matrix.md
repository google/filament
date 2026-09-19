//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getModelMatrix](get-model-matrix.md)

# getModelMatrix

[main]\
open fun [getModelMatrix](get-model-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the camera's model matrix 

Helper method to return the camera's entity transform component. It has the same effect as calling:

```kotlin

 engine.getTransformManager().getWorldTransform(
         engine.getTransformManager().getInstance(camera->getEntity()));

```

#### Return

The camera's pose in world space as a rigid transform. Parent transforms, if any, are taken into account.
