//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilReadMask](set-stencil-read-mask.md)

# setStencilReadMask

[main]\
open fun [setStencilReadMask](set-stencil-read-mask.md)(readMask: Int)

open fun [setStencilReadMask](set-stencil-read-mask.md)(readMask: Int, face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil read mask (default is 0xFF). 

The stencil read mask masks the bits of the values participating in the stencil comparison test- both the value read from the stencil buffer and the reference value.

It's possible to set separate stencil read masks; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the stencil read mask(s) updated by this call.
