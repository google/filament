//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setColor](set-color.md)

# setColor

[main]\
open fun [setColor](set-color.md)(i: Int, colorx: Float, colory: Float, colorz: Float)

Dynamically updates the light's hue as linear sRGB

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| colorx | (x component) Color of the light specified in the linear sRGB color-space. The default is white {1,1,1}. |
| colory | (y component) Color of the light specified in the linear sRGB color-space. The default is white {1,1,1}. |
| colorz | (z component) Color of the light specified in the linear sRGB color-space. The default is white {1,1,1}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
| [getInstance](get-instance.md) |

[main]\
open fun [setColor](set-color.md)(i: Int, color: Array&lt;Float&gt;)

Dynamically updates the light's hue as linear sRGB

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| color | Color of the light specified in the linear sRGB color-space. The default is white {1,1,1}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
| [getInstance](get-instance.md) |
