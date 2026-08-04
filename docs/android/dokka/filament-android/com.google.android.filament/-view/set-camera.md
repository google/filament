//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setCamera](set-camera.md)

# setCamera

[main]\
open fun [setCamera](set-camera.md)(camera: [Camera](../-camera/index.md))

Sets this View's Camera. 

 This method associates the specified Camera with this View. A Camera can be associated with several View instances. To remove an existing association, simply pass null. 

 The View does not take ownership of the Scene pointer. Before destroying a Camera, be sure to remove it from all associated Views. If the camera isn't set, Renderer::render() will result in a no-op. 

#### See also

| |
|---|
| [getCamera](get-camera.md) |
