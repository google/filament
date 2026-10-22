//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[projection](projection.md)

# projection

[main]\
open fun [projection](projection.md)(direction: [Camera.Fov](-fov/index.md), fovInDegrees: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the projection matrix from the field-of-view.

#### Parameters

main

| | |
|---|---|
| direction | direction of the `fovInDegrees` parameter. |
| fovInDegrees | full field-of-view in degrees. 0 <`fov`<180. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| out | optional array to store the result, or null to allocate a new one |

#### See also

| |
|---|
| [Camera.Fov](-fov/index.md) |

[main]\
open fun [projection](projection.md)(direction: [Camera.Fov](-fov/index.md), fovInDegrees: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the projection matrix from the field-of-view.

#### Parameters

main

| | |
|---|---|
| direction | direction of the `fovInDegrees` parameter. |
| fovInDegrees | full field-of-view in degrees. 0 <`fov`<180. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |

#### See also

| |
|---|
| [Camera.Fov](-fov/index.md) |

[main]\
open fun [projection](projection.md)(focalLengthInMillimeters: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the projection matrix from the focal length.

#### Parameters

main

| | |
|---|---|
| focalLengthInMillimeters | lens's focal length in millimeters. `focalLength`>0. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| out | optional array to store the result, or null to allocate a new one |

[main]\
open fun [projection](projection.md)(focalLengthInMillimeters: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Returns the projection matrix from the focal length.

#### Parameters

main

| | |
|---|---|
| focalLengthInMillimeters | lens's focal length in millimeters. `focalLength`>0. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |
