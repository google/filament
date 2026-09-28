//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setScene](set-scene.md)

# setScene

[main]\
open fun [setScene](set-scene.md)(scene: [Scene](../-scene/index.md))

Set this View instance's Scene. 

There is no reference-counting. If a Scene is destroyed before it is dissociated from a View, it will be automatically dissociated from that View (setting the View's Scene to nullptr).

#### Parameters

main

| | |
|---|---|
| scene | Associate the specified Scene to this View. A Scene can be associated to several View instances. `scene` can be nullptr to dissociate the currently set Scene from this View. The View doesn't take ownership of the Scene pointer (which acts as a reference). |
