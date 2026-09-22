//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setDirection](set-direction.md)

# setDirection

[main]\
open fun [setDirection](set-direction.md)(i: Int, directionx: Float, directiony: Float, directionz: Float)

Dynamically updates the light's direction

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| directionx | (x component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
| directiony | (y component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
| directionz | (z component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |

[main]\
open fun [setDirection](set-direction.md)(i: Int, direction: Array&lt;Float&gt;)

Dynamically updates the light's direction

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| direction | Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
