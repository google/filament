//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setModelMatrix](set-model-matrix.md)

# setModelMatrix

[main]\
open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: Array&lt;Double&gt;)

Sets the camera's model matrix. 

Helper method to set the camera's entity transform component. It has the same effect as calling:

```kotlin

 engine.getTransformManager().setTransform(
         engine.getTransformManager().getInstance(camera->getEntity()), model);

```

`model` must be a rigid transform

The Camera &quot;looks&quot; towards its -z axis

#### Parameters

main

| | |
|---|---|
| modelMatrix | The camera position and orientation provided as a rigid transform matrix. |

[main]\
open fun [setModelMatrix](set-model-matrix.md)(modelMatrix: Array&lt;Float&gt;)
