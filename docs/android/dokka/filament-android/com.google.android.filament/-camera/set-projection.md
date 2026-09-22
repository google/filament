//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setProjection](set-projection.md)

# setProjection

[main]\
open fun [setProjection](set-projection.md)(projection: [Camera.Projection](-projection/index.md), left: Double, right: Double, bottom: Double, top: Double, near: Double, far: Double)

Sets the projection matrix from a frustum defined by six planes.

#### Parameters

main

| | |
|---|---|
| projection | type of #Projection to use. |
| left | distance in world units from the camera to the left plane, at the near plane. Precondition: `left` != `right`. |
| right | distance in world units from the camera to the right plane, at the near plane. Precondition: `left` != `right`. |
| bottom | distance in world units from the camera to the bottom plane, at the near plane. Precondition: `bottom` != `top`. |
| top | distance in world units from the camera to the top plane, at the near plane. Precondition: `left` != `right`. |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for PROJECTION::PERSPECTIVE or `near` != far for PROJECTION::ORTHO |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>near for PROJECTION::PERSPECTIVE or `far` != near for PROJECTION::ORTHO |

#### See also

| |
|---|
| [Camera.Projection](-projection/index.md) |
| [Frustum](../-frustum/index.md) |

[main]\
open fun [setProjection](set-projection.md)(fovInDegrees: Double, aspect: Double, near: Double, far: Double)

Utility to set the projection matrix from the field-of-view.

#### Parameters

main

| | |
|---|---|
| fovInDegrees | full field-of-view in degrees. 0 <`fov`<180. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |

#### See also

| |
|---|
| [Camera.Fov](-fov/index.md) |

[main]\
open fun [setProjection](set-projection.md)(fovInDegrees: Double, aspect: Double, near: Double, far: Double, direction: [Camera.Fov](-fov/index.md))

Utility to set the projection matrix from the field-of-view.

#### Parameters

main

| | |
|---|---|
| fovInDegrees | full field-of-view in degrees. 0 <`fov`<180. |
| aspect | aspect ratio width / height. `aspect`>0. |
| near | distance in world units from the camera to the near plane. `near`>0. |
| far | distance in world units from the camera to the far plane. `far`>`near`. |
| direction | direction of the `fovInDegrees` parameter. |

#### See also

| |
|---|
| [Camera.Fov](-fov/index.md) |
