//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setCamera](set-camera.md)

# setCamera

[main]\
open fun [setCamera](set-camera.md)(camera: [Camera](../-camera/index.md))

Sets this View's Camera. 

There is no reference-counting. Make sure to dissociate a Camera from all Views before destroying it.

#### Parameters

main

| | |
|---|---|
| camera | Associate the specified Camera to this View. A Camera can be associated to several View instances. `camera` can be nullptr to dissociate the currently set Camera from this View. The View doesn't take ownership of the Camera pointer (which acts as a reference). If the camera isn't set, Renderer::render() will result in a no-op. |
