//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setScene](set-scene.md)

# setScene

[main]\
open fun [setScene](set-scene.md)(scene: [Scene](../-scene/index.md))

Sets this View instance's Scene. 

 This method associates the specified Scene with this View. Note that a particular scene can be associated with several View instances. To remove an existing association, simply pass null. 

 The View does not take ownership of the Scene pointer. Before destroying a Scene, be sure to remove it from all assoicated Views. 

#### See also

| |
|---|
| [getScene](get-scene.md) |
