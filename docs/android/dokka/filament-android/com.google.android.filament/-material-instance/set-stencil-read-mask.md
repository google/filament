//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilReadMask](set-stencil-read-mask.md)

# setStencilReadMask

[main]\
open fun [setStencilReadMask](set-stencil-read-mask.md)(readMask: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil read mask (default is 0xFF). 

 The stencil read mask masks the bits of the values participating in the stencil comparison test- both the value read from the stencil buffer and the reference value. 

 It's possible to set separate stencil read masks; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the stencil read mask(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| readMask | the read mask (only the least significant 8 bits are used) |
| face | the faces to update the read mask for |

[main]\
open fun [setStencilReadMask](set-stencil-read-mask.md)(readMask: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Sets the stencil read mask for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilReadMask(int, StencilFace)](set-stencil-read-mask.md) |
