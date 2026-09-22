//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[position](position.md)

# position

[main]\
open fun [position](position.md)(positionx: Float, positiony: Float, positionz: Float): [LightManager.Builder](index.md)

Sets the initial position of the light in world space. 

The Light's position is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)

#### Return

This Builder, for chaining calls.

[main]\
open fun [position](position.md)(position: Array&lt;Float&gt;): [LightManager.Builder](index.md)

Sets the initial position of the light in world space. 

The Light's position is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| position | Light's position in world space. The default is at the origin. |
