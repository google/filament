//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[MotionVectorsOptions](index.md)

# MotionVectorsOptions

open class [MotionVectorsOptions](index.md)

Options for the motion vectors pass. 

Motion vectors are generated only when post-processing is enabled, stereo is disabled, the engine runs at feature level 1 or higher and the device supports RG16F render targets. They capture the rigid motion of renderables (TransformManager and InstanceBuffer transforms) and of the camera. Skinning/morphing deformation and the VIEW/DEVICE vertex domains are not captured. TAA uses motion vectors automatically when they are enabled.

#### See also

| |
|---|
| [View](../set-motion-vectors-options.md) |

## Constructors

| | |
|---|---|
| [MotionVectorsOptions](-motion-vectors-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enables the motion vectors pass |
