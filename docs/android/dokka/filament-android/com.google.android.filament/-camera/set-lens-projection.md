//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setLensProjection](set-lens-projection.md)

# setLensProjection

[main]\
open fun [setLensProjection](set-lens-projection.md)(focalLength: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), aspect: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the projection matrix from the focal length.

#### Parameters

main

| | |
|---|---|
| focalLength | lens's focal length in millimeters. `focalLength`>0 |
| aspect | aspect ratio width/height. `aspect`>0 |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `near` != `far` for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>`near`for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `far` != `near`for [ORTHO](-projection/-o-r-t-h-o/index.md). |
