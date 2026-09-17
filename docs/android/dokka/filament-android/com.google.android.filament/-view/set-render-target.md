//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setRenderTarget](set-render-target.md)

# setRenderTarget

[main]\
open fun [setRenderTarget](set-render-target.md)(renderTarget: [RenderTarget](../-render-target/index.md))

Specifies an offscreen render target to render into. 

By default, the view's associated render target is nullptr, which corresponds to the SwapChain associated with the engine.

A view with a custom render target cannot rely on Renderer::ClearOptions, which only apply to the SwapChain. Such view can use a Skybox instead.

#### Parameters

main

| | |
|---|---|
| renderTarget | Render target associated with view, or nullptr for the swap chain. |
