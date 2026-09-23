//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setCustomProjection](set-custom-projection.md)

# setCustomProjection

[main]\
open fun [setCustomProjection](set-custom-projection.md)(projection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets a custom projection matrix. 

The projection matrix must define an NDC system that must match the OpenGL convention, that is all 3 axis are mapped to [-1, 1].

#### Parameters

main

| | |
|---|---|
| projection | custom projection matrix used for rendering and culling |
| near | distance in world units from the camera to the near plane. |
| far | distance in world units from the camera to the far plane. `far` != `near`. |

[main]\
open fun [setCustomProjection](set-custom-projection.md)(projection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, projectionForCulling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets the projection matrix. 

The projection matrices must define an NDC system that must match the OpenGL convention, that is all 3 axis are mapped to [-1, 1].

#### Parameters

main

| | |
|---|---|
| projection | custom projection matrix used for rendering |
| projectionForCulling | custom projection matrix used for culling |
| near | distance in world units from the camera to the near plane. |
| far | distance in world units from the camera to the far plane. `far` != `near`. |
