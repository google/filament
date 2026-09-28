//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilOpDepthFail](set-stencil-op-depth-fail.md)

# setStencilOpDepthFail

[main]\
open fun [setStencilOpDepthFail](set-stencil-op-depth-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md))

open fun [setStencilOpDepthFail](set-stencil-op-depth-fail.md)(op: [MaterialInstance.StencilOperation](-stencil-operation/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the depth fail operation (default is StencilOperation::KEEP). 

The depth fail operation is performed to update values in the stencil buffer when the depth test fails.

It's possible to set separate depth fail operations; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the depth fail operation(s) updated by this call.
