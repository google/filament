//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setCustomProjection](set-custom-projection.md)

# setCustomProjection

[main]\
open fun [setCustomProjection](set-custom-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets a custom projection matrix. 

The projection matrix must define an NDC system that must match the OpenGL convention, that is all 3 axis are mapped to [-1, 1].

#### Parameters

main

| | |
|---|---|
| inProjection | custom projection matrix for rendering and culling |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `near` != `far` for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>`near`for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `far` != `near`for [ORTHO](-projection/-o-r-t-h-o/index.md). |

[main]\
open fun [setCustomProjection](set-custom-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, inProjectionForCulling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets a custom projection matrix. 

The projection matrices must define an NDC system that must match the OpenGL convention, that is all 3 axis are mapped to [-1, 1].

#### Parameters

main

| | |
|---|---|
| inProjection | custom projection matrix for rendering. |
| inProjectionForCulling | custom projection matrix for culling. |
| near | distance in world units from the camera to the near plane. The near plane's position in view space is z = -`near`. Precondition: `near`>0 for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `near` != `far` for [ORTHO](-projection/-o-r-t-h-o/index.md). |
| far | distance in world units from the camera to the far plane. The far plane's position in view space is z = -`far`. Precondition: `far`>`near`for [PERSPECTIVE](-projection/-p-e-r-s-p-e-c-t-i-v-e/index.md) or `far` != `near`for [ORTHO](-projection/-o-r-t-h-o/index.md). |
