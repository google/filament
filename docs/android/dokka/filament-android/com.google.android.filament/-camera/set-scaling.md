//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setScaling](set-scaling.md)

# setScaling

[main]\
open fun [setScaling](set-scaling.md)(scalingx: Double, scalingy: Double)

Sets an additional matrix that scales the projection matrix. 

This is useful to adjust the aspect ratio of the camera independent of its projection. First, pass an aspect of 1.0 to setProjection. Then set the scaling with the desired aspect ratio:

const double aspect = width / height;

// with Fov::HORIZONTAL passed to setProjection: camera->setScaling(double4 {1.0, aspect});

// with Fov::VERTICAL passed to setProjection: camera->setScaling(double4 {1.0 / aspect, 1.0});

By default, this is an identity matrix.

#### Parameters

main

| | |
|---|---|
| scalingx | (x component) diagonal of the 2x2 scaling matrix to be applied after the projection matrix. |
| scalingy | (y component) diagonal of the 2x2 scaling matrix to be applied after the projection matrix. |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |

[main]\
open fun [setScaling](set-scaling.md)(scaling: Array&lt;Double&gt;)

Sets an additional matrix that scales the projection matrix. 

This is useful to adjust the aspect ratio of the camera independent of its projection. First, pass an aspect of 1.0 to setProjection. Then set the scaling with the desired aspect ratio:

const double aspect = width / height;

// with Fov::HORIZONTAL passed to setProjection: camera->setScaling(double4 {1.0, aspect});

// with Fov::VERTICAL passed to setProjection: camera->setScaling(double4 {1.0 / aspect, 1.0});

By default, this is an identity matrix.

#### Parameters

main

| | |
|---|---|
| scaling | diagonal of the 2x2 scaling matrix to be applied after the projection matrix. |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |
