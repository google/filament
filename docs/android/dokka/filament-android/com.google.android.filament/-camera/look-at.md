//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[lookAt](look-at.md)

# lookAt

[main]\
open fun [lookAt](look-at.md)(eyex: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyey: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyez: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerx: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centery: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerz: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the camera's model matrix

#### Parameters

main

| | |
|---|---|
| eyex | (x component) The position of the camera in world space. |
| eyey | (y component) The position of the camera in world space. |
| eyez | (z component) The position of the camera in world space. |
| centerx | (x component) The point in world space the camera is looking at. |
| centery | (y component) The point in world space the camera is looking at. |
| centerz | (z component) The point in world space the camera is looking at. |

[main]\
open fun [lookAt](look-at.md)(eyex: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyey: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), eyez: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerx: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centery: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), centerz: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upx: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upy: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), upz: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the camera's model matrix

#### Parameters

main

| | |
|---|---|
| eyex | (x component) The position of the camera in world space. |
| eyey | (y component) The position of the camera in world space. |
| eyez | (z component) The position of the camera in world space. |
| centerx | (x component) The point in world space the camera is looking at. |
| centery | (y component) The point in world space the camera is looking at. |
| centerz | (z component) The point in world space the camera is looking at. |
| upx | (x component) A unit vector denoting the camera's &quot;up&quot; direction. |
| upy | (y component) A unit vector denoting the camera's &quot;up&quot; direction. |
| upz | (z component) A unit vector denoting the camera's &quot;up&quot; direction. |

[main]\
open fun [lookAt](look-at.md)(eye: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, center: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, up: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Sets the camera's model matrix

#### Parameters

main

| | |
|---|---|
| eye | The position of the camera in world space. |
| center | The point in world space the camera is looking at. |
| up | A unit vector denoting the camera's &quot;up&quot; direction. |
