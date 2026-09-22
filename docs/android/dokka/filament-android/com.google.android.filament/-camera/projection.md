//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[projection](projection.md)

# projection

[main]\
open fun [projection](projection.md)(direction: [Camera.Fov](-fov/index.md), fovInDegrees: Double, aspect: Double, near: Double, out: Array&lt;Double&gt;): Array&lt;Double&gt;

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
open fun [projection](projection.md)(direction: [Camera.Fov](-fov/index.md), fovInDegrees: Double, aspect: Double, near: Double, far: Double, out: Array&lt;Double&gt;): Array&lt;Double&gt;

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
open fun [projection](projection.md)(focalLengthInMillimeters: Double, aspect: Double, near: Double, out: Array&lt;Double&gt;): Array&lt;Double&gt;

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
open fun [projection](projection.md)(focalLengthInMillimeters: Double, aspect: Double, near: Double, far: Double, out: Array&lt;Double&gt;): Array&lt;Double&gt;

Returns the projection matrix from the focal length.

#### Parameters

main

| | |
|---|---|
| focalLengthInMillimeters | lens's focal length in millimeters. `focalLength`>0. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |
