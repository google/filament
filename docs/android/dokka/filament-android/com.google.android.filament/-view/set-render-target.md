//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setRenderTarget](set-render-target.md)

# setRenderTarget

[main]\
open fun [setRenderTarget](set-render-target.md)(target: [RenderTarget](../-render-target/index.md))

Specifies an offscreen render target to render into. 

 By default, the view's associated render target is null, which corresponds to the SwapChain associated with the engine. 

 A view with a custom render target cannot rely on Renderer.ClearOptions, which only applies to the SwapChain. Such view can use a Skybox instead. 

#### Parameters

main

| | |
|---|---|
| target | render target associated with view, or null for the swap chain |
