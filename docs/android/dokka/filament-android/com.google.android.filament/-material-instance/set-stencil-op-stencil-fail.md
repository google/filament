//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilOpStencilFail](set-stencil-op-stencil-fail.md)

# setStencilOpStencilFail

[main]\
open fun [setStencilOpStencilFail](set-stencil-op-stencil-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil fail operation (default is [KEEP](-stencil-operation/-k-e-e-p/index.md)). 

 The stencil fail operation is performed to update values in the stencil buffer when the stencil test fails. 

 It's possible to set separate stencil fail operations; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the stencil fail operation(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| op | the stencil fail operation |
| face | the faces to update the stencil fail operation for |

[main]\
open fun [setStencilOpStencilFail](set-stencil-op-stencil-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md))

Sets the stencil fail operation for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilOpStencilFail(StencilOperation, StencilFace)](set-stencil-op-stencil-fail.md) |
