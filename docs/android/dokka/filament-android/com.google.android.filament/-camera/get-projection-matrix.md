//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getProjectionMatrix](get-projection-matrix.md)

# getProjectionMatrix

[main]\
open fun [getProjectionMatrix](get-projection-matrix.md)(out: Array&lt;Double&gt;): Array&lt;Double&gt;

Returns the projection matrix used for rendering. 

The projection matrix used for rendering always has its far plane set to infinity. This is why it may differ from the matrix set through setProjection() or setLensProjection().

#### Return

The projection matrix used for rendering

#### Parameters

main

| | |
|---|---|
| out | optional array to store the result, or null to allocate a new one |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |
| [getCullingProjectionMatrix](get-culling-projection-matrix.md) |
| [setCustomEyeProjection](set-custom-eye-projection.md) |

[main]\
open fun [getProjectionMatrix](get-projection-matrix.md)(eyeId: Int, out: Array&lt;Double&gt;): Array&lt;Double&gt;

Returns the projection matrix used for rendering. 

The projection matrix used for rendering always has its far plane set to infinity. This is why it may differ from the matrix set through setProjection() or setLensProjection().

#### Return

The projection matrix used for rendering

#### Parameters

main

| | |
|---|---|
| eyeId | the index of the eye to return the projection matrix for, must be <config.stereoscopicEyeCount |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |
| [getCullingProjectionMatrix](get-culling-projection-matrix.md) |
| [setCustomEyeProjection](set-custom-eye-projection.md) |
