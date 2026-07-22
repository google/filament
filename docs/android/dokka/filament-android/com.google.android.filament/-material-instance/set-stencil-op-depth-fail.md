//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilOpDepthFail](set-stencil-op-depth-fail.md)

# setStencilOpDepthFail

[main]\
open fun [setStencilOpDepthFail](set-stencil-op-depth-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the depth fail operation (default is [KEEP](-stencil-operation/-k-e-e-p/index.md)). 

 The depth fail operation is performed to update values in the stencil buffer when the depth test fails. 

 It's possible to set separate depth fail operations; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the depth fail operation(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| op | the depth fail operation |
| face | the faces to update the depth fail operation for |

[main]\
open fun [setStencilOpDepthFail](set-stencil-op-depth-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md))

Sets the depth fail operation for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilOpDepthFail(StencilOperation, StencilFace)](set-stencil-op-depth-fail.md) |
