//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[renderStandaloneView](render-standalone-view.md)

# renderStandaloneView

[main]\
open fun [renderStandaloneView](render-standalone-view.md)(view: [View](../-view/index.md))

Render a standalone View into its associated RenderTarget 

This call is mostly equivalent to calling render(View*) inside a beginFrame / endFrame block, but incurs less overhead. It can be used as a poor man's compute API.

renderStandaloneView() must be called from the Engine's main thread (or external synchronization must be provided). In particular, calls to renderStandaloneView() on different Renderer instances **must** be synchronized.

#### Parameters

main

| | |
|---|---|
| view | A pointer to the view to render. This View must have a RenderTarget associated to it.<br>@attention renderStandaloneView() must be called outside of beginFrame() / endFrame().<br>@remark renderStandaloneView() perform potentially heavy computations and cannot be multi-threaded. However, internally, renderStandaloneView() is highly multi-threaded to both improve performance in mitigate the call's latency. |
