//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilOpDepthStencilPass](set-stencil-op-depth-stencil-pass.md)

# setStencilOpDepthStencilPass

[main]\
open fun [setStencilOpDepthStencilPass](set-stencil-op-depth-stencil-pass.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the depth-stencil pass operation (default is [KEEP](-stencil-operation/-k-e-e-p/index.md)). 

 The depth-stencil pass operation is performed to update values in the stencil buffer when both the stencil test and depth test pass. 

 It's possible to set separate depth-stencil pass operations; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the depth-stencil pass operation(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| op | the depth-stencil pass operation |
| face | the faces to update the depth-stencil operation for |

[main]\
open fun [setStencilOpDepthStencilPass](set-stencil-op-depth-stencil-pass.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md))

Sets the depth-stencil pass operation for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilOpDepthStencilPass(StencilOperation, StencilFace)](set-stencil-op-depth-stencil-pass.md) |
