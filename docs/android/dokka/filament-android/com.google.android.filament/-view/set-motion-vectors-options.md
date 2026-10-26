//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setMotionVectorsOptions](set-motion-vectors-options.md)

# setMotionVectorsOptions

[main]\
open fun [setMotionVectorsOptions](set-motion-vectors-options.md)(options: [View.MotionVectorsOptions](-motion-vectors-options/index.md))

Enables or disables the motion vectors pass. 

Disabled by default.

Motion vectors are only generated when post-processing is enabled, stereo is disabled, the engine runs at feature level 1 or higher and the device supports RG16F render targets; otherwise this setting has no effect.

#### Parameters

main

| | |
|---|---|
| options | motion vectors options |

#### See also

| |
|---|
| [View.MotionVectorsOptions](-motion-vectors-options/index.md) |
