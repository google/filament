//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setModelMatrix](set-model-matrix.md)

# setModelMatrix

[main]\
open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Sets the camera's model matrix. 

 Helper method to set the camera's entity transform component. Remember that the Camera &quot;looks&quot; towards its -z axis. 

 This has the same effect as calling: 

```kotlin
 engine.getTransformManager().setTransform(
         engine.getTransformManager().getInstance(camera->getEntity()), modelMatrix);

```

#### Parameters

main

| | |
|---|---|
| modelMatrix | The camera position and orientation provided as a **rigid transform** matrix. |

[main]\
open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Sets the camera's model matrix. 

 Helper method to set the camera's entity transform component. Remember that the Camera &quot;looks&quot; towards its -z axis. 

#### Parameters

main

| | |
|---|---|
| modelMatrix | The camera position and orientation provided as a **rigid transform** matrix. |
