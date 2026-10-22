//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilCompareFunction](set-stencil-compare-function.md)

# setStencilCompareFunction

[main]\
open fun [setStencilCompareFunction](set-stencil-compare-function.md)(func: [TextureSampler.CompareFunc](../-texture-sampler/-compare-func/index.md))

open fun [setStencilCompareFunction](set-stencil-compare-function.md)(func: [TextureSampler.CompareFunc](../-texture-sampler/-compare-func/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil comparison function (default is StencilCompareFunc::A). 

It's possible to set separate stencil comparison functions; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the comparison function(s) updated by this call.
