//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilReferenceValue](set-stencil-reference-value.md)

# setStencilReferenceValue

[main]\
open fun [setStencilReferenceValue](set-stencil-reference-value.md)(value: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil reference value (default is 0). 

 The stencil reference value is the left-hand side for stencil comparison tests. It's also used as the replacement stencil value when [StencilOperation](-stencil-operation/index.md) is [REPLACE](-stencil-operation/-r-e-p-l-a-c-e/index.md). 

 It's possible to set separate stencil reference values; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the reference value(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| value | the stencil reference value (only the least significant 8 bits are used) |
| face | the faces to update the reference value for |

[main]\
open fun [setStencilReferenceValue](set-stencil-reference-value.md)(value: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Sets the stencil reference value for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilReferenceValue(int, StencilFace)](set-stencil-reference-value.md) |
