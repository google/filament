//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilReferenceValue](set-stencil-reference-value.md)

# setStencilReferenceValue

[main]\
open fun [setStencilReferenceValue](set-stencil-reference-value.md)(value: Int)

open fun [setStencilReferenceValue](set-stencil-reference-value.md)(value: Int, face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil reference value (default is 0). 

The stencil reference value is the left-hand side for stencil comparison tests. It's also used as the replacement stencil value when StencilOperation is REPLACE.

It's possible to set separate stencil reference values; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the reference value(s) updated by this call.
