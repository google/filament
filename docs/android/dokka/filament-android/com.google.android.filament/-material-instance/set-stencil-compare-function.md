//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setStencilCompareFunction](set-stencil-compare-function.md)

# setStencilCompareFunction

[main]\
open fun [setStencilCompareFunction](set-stencil-compare-function.md)(func: [TextureSampler.CompareFunction](../-texture-sampler/-compare-function/index.md), face: [MaterialInstance.StencilFace](-stencil-face/index.md))

Sets the stencil comparison function (default is [ALWAYS](../-texture-sampler/-compare-function/-a-l-w-a-y-s/index.md)). 

 It's possible to set separate stencil comparison functions; one for front-facing polygons, and one for back-facing polygons. The face parameter determines the comparison function(s) updated by this call. 

#### Parameters

main

| | |
|---|---|
| func | the stencil comparison function |
| face | the faces to update the comparison function for |

[main]\
open fun [setStencilCompareFunction](set-stencil-compare-function.md)(func: [TextureSampler.CompareFunction](../-texture-sampler/-compare-function/index.md))

Sets the stencil comparison function for both front and back-facing polygons.

#### See also

| |
|---|
| [setStencilCompareFunction(TextureSampler.CompareFunction, StencilFace)](set-stencil-compare-function.md) |
