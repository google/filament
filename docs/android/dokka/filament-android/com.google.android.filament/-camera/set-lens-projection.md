//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setLensProjection](set-lens-projection.md)

# setLensProjection

[main]\
open fun [setLensProjection](set-lens-projection.md)(focalLengthInMillimeters: Double, aspect: Double, near: Double, far: Double)

Utility to set the projection matrix from the focal length.

#### Parameters

main

| | |
|---|---|
| focalLengthInMillimeters | lens's focal length in millimeters. `focalLength`>0. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |
