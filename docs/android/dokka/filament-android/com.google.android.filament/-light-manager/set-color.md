//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setColor](set-color.md)

# setColor

[main]\
open fun [setColor](set-color.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), colorx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colory: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

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
open fun [setColor](set-color.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

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
