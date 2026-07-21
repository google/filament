//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setProjection](set-projection.md)

# setProjection

[main]\
open fun [setProjection](set-projection.md)(projection: [Camera.Projection](-projection/index.md), left: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), right: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), bottom: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), top: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the projection matrix from a frustum defined by six planes.

#### Parameters

main

| | |
|---|---|
| projection | type of projection to use |
| left | distance in world units from the camera to the left plane, at the near plane. Precondition: `left` != `right` |
| right | distance in world units from the camera to the right plane, at the near plane. Precondition: `left` != `right` |
| bottom | distance in world units from the camera to the bottom plane, at the near plane. Precondition: `bottom` != `top` |
| top | distance in world units from the camera to the top plane, at the near plane. Precondition: `bottom` != `top` |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `near` != `far` for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>`near`for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `far` != `near`for [ORTHO](-projection/-o-r-t-h-o/index.md). <br>These parameters are silently modified to meet the preconditions above. |

#### See also

| |
|---|
| [Camera.Projection](-projection/index.md) |

[main]\
open fun [setProjection](set-projection.md)(fovInDegrees: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), direction: [Camera.Fov](-fov/index.md))

Sets the projection matrix from the field-of-view.

#### Parameters

main

| | |
|---|---|
| fovInDegrees | full field-of-view in degrees. 0 <`fovInDegrees`<180 |
| aspect | aspect ratio width/height. `aspect`>0 |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `near` != `far` for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>`near`for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `far` != `near`for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| direction | direction of the field-of-view parameter. <br>These parameters are silently modified to meet the preconditions above. |

#### See also

| |
|---|
| [Camera.Fov](-fov/index.md) |
