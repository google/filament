//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setScaling](set-scaling.md)

# setScaling

[main]\
open fun [setScaling](set-scaling.md)(xscaling: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), yscaling: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets an additional matrix that scales the projection matrix. 

This is useful to adjust the aspect ratio of the camera independent from its projection. First, pass an aspect of 1.0 to setProjection. Then set the scaling with the desired aspect ratio:`
     double aspect = width / height; 
     
     // with Fov.HORIZONTAL passed to setProjection: 
     camera.setScaling(1.0, aspect); 
     
     // with Fov.VERTICAL passed to setProjection: 
     camera.setScaling(1.0 / aspect, 1.0); 
     ` By default, this is an identity matrix. 

#### Parameters

main

| | |
|---|---|
| xscaling | horizontal scaling to be applied after the projection matrix. |
| yscaling | vertical scaling to be applied after the projection matrix. |

#### See also

| |
|---|
| com.google.android.filament.Camera |

[main]\
open fun [~~setScaling~~](set-scaling.md)(inScaling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

---

### Deprecated

---

Sets an additional matrix that scales the projection matrix. 

This is useful to adjust the aspect ratio of the camera independent from its projection. First, pass an aspect of 1.0 to setProjection. Then set the scaling with the desired aspect ratio:`
     double aspect = width / height; 
     
     // with Fov.HORIZONTAL passed to setProjection: 
     double[] s = {1.0, aspect, 1.0, 1.0}; 
     camera.setScaling(s); 
     
     // with Fov.VERTICAL passed to setProjection: 
     double[] s = {1.0 / aspect, 1.0, 1.0, 1.0}; 
     camera.setScaling(s); 
     ` By default, this is an identity matrix. 

#### Deprecated

use [setScaling](set-scaling.md)

#### Parameters

main

| | |
|---|---|
| inScaling | diagonal of the scaling matrix to be applied after the projection matrix. |

#### See also

| |
|---|
| com.google.android.filament.Camera |
