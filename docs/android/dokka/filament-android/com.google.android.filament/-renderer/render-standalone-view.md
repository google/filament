//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[renderStandaloneView](render-standalone-view.md)

# renderStandaloneView

[main]\
open fun [renderStandaloneView](render-standalone-view.md)(view: [View](../-view/index.md))

Renders a standalone [View](../-view/index.md) into its associated `RenderTarget`. 

 This call is mostly equivalent to calling [render](render.md) inside a [beginFrame](begin-frame.md) / [endFrame](end-frame.md) block, but incurs less overhead. It can be used as a poor man's compute API. 

- 
   `renderStandaloneView()` must be called **outside** of [beginFrame](begin-frame.md) / [endFrame](end-frame.md).
- 
   `renderStandaloneView()` must be called from the [Engine](../-engine/index.md)'s main thread (or external synchronization must be provided). In particular, calls to `renderStandaloneView()` on different `Renderer` instances **must** be synchronized.
- 
   `renderStandaloneView()` performs potentially heavy computations and cannot be multi-threaded. However, internally, it is highly multi-threaded to both improve performance and mitigate the call's latency.

#### Parameters

main

| | |
|---|---|
| view | the [View](../-view/index.md) to render. This View must have an associated [RenderTarget](../-render-target/index.md) |

#### See also

| |
|---|
| [View](../-view/index.md) |
