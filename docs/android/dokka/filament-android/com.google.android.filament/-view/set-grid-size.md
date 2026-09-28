//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setGridSize](set-grid-size.md)

# setGridSize

[main]\
open fun [setGridSize](set-grid-size.md)(size: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the grid size for grid-based world origin snapping. 

The world origin used for rendering will snap to a grid of this size. This avoids recomputing all transforms every frame when the camera moves within a grid cell.

Hysteresis is applied automatically to avoid rapid snapping near edges.

#### Parameters

main

| | |
|---|---|
| size | The size of the grid cell in world units. If set to 0 or negative, the grid size is automatically calculated based on the camera frustum. |
