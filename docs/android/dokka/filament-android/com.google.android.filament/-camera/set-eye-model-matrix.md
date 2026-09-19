//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setEyeModelMatrix](set-eye-model-matrix.md)

# setEyeModelMatrix

[main]\
open fun [setEyeModelMatrix](set-eye-model-matrix.md)(eyeId: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), model: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Set the position of an eye relative to this Camera (head). 

By default, both eyes' model matrices are identity matrices.

For example, to position Eye 0 3cm leftwards and Eye 1 3cm rightwards:

```kotlin

const mat4 leftEye  = mat4::translation(double3{-0.03, 0.0, 0.0});
const mat4 rightEye = mat4::translation(double3{ 0.03, 0.0, 0.0});
camera.setEyeModelMatrix(0, leftEye);
camera.setEyeModelMatrix(1, rightEye);

```

This method is not intended to be called every frame. Instead, to update the position of the head, use Camera::setModelMatrix.

#### Parameters

main

| | |
|---|---|
| eyeId | the index of the eye to set, must be <config.stereoscopicEyeCount |
| model | the model matrix for an individual eye |
