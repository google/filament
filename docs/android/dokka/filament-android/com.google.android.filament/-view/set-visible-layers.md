//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setVisibleLayers](set-visible-layers.md)

# setVisibleLayers

[main]\
open fun [setVisibleLayers](set-visible-layers.md)(select: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), values: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Sets which layers are visible. 

 Renderable objects can have one or several layers associated to them. Layers are represented with an 8-bits bitmask, where each bit corresponds to a layer. By default all layers are visible. 

#### Parameters

main

| | |
|---|---|
| select | a bitmask specifying which layer to set or clear using `values`. |
| values | a bitmask where each bit sets the visibility of the corresponding layer (1: visible, 0: invisible), only layers in `select` are affected. |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/set-layer-mask.md) |
