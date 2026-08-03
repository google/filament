//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setFrontFaceWindingInverted](set-front-face-winding-inverted.md)

# setFrontFaceWindingInverted

[main]\
open fun [setFrontFaceWindingInverted](set-front-face-winding-inverted.md)(inverted: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Inverts the winding order of front faces. By default front faces use a counter-clockwise winding order. When the winding order is inverted, front faces are faces with a clockwise winding order. Changing the winding order will directly affect the culling mode in materials (see Material#getCullingMode). Inverting the winding order of front faces is useful when rendering mirrored reflections (water, mirror surfaces, front camera in AR, etc.).

#### Parameters

main

| | |
|---|---|
| inverted | True to invert front faces, false otherwise. |
