//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[direction](direction.md)

# direction

[main]\
open fun [direction](direction.md)(directionx: Float, directiony: Float, directionz: Float): [LightManager.Builder](index.md)

Sets the initial direction of a light in world space. 

The Light's direction is ignored for Type.POINT lights.

#### Return

This Builder, for chaining calls.

[main]\
open fun [direction](direction.md)(direction: Array&lt;Float&gt;): [LightManager.Builder](index.md)

Sets the initial direction of a light in world space. 

The Light's direction is ignored for Type.POINT lights.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| direction | Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
