//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[lookAt](look-at.md)

# lookAt

[main]\
open fun [lookAt](look-at.md)(eyex: Double, eyey: Double, eyez: Double, centerx: Double, centery: Double, centerz: Double)

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
open fun [lookAt](look-at.md)(eyex: Double, eyey: Double, eyez: Double, centerx: Double, centery: Double, centerz: Double, upx: Double, upy: Double, upz: Double)

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
open fun [lookAt](look-at.md)(eye: Array&lt;Double&gt;, center: Array&lt;Double&gt;, up: Array&lt;Double&gt;)

Sets the camera's model matrix

#### Parameters

main

| | |
|---|---|
| eye | The position of the camera in world space. |
| center | The point in world space the camera is looking at. |
| up | A unit vector denoting the camera's &quot;up&quot; direction. |
