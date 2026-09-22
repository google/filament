//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilWriteMask](set-stencil-write-mask.md)

# setStencilWriteMask

[main]\
open fun [setStencilWriteMask](set-stencil-write-mask.md)(writeMask: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

open fun [setStencilWriteMask](set-stencil-write-mask.md)(writeMask: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil write mask (default is 0xFF). 

The stencil write mask masks the bits in the stencil buffer updated by stencil operations.

It's possible to set separate stencil write masks; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the stencil write mask(s) updated by this call.
