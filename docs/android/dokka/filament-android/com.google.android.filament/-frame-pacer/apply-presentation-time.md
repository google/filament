//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)/[applyPresentationTime](apply-presentation-time.md)

# applyPresentationTime

[main]\
open fun [applyPresentationTime](apply-presentation-time.md)(renderer: [Renderer](../-renderer/index.md))

Applies the computed Latency Offset timestamp directly onto the rendering command stream. 

This instructs the underlying display compositor (such as SurfaceFlinger) exactly when to latch and present the buffer, eliminating micro-stutter. This must be called before `Renderer::endFrame()`.

Calling this method automatically applies the target presentation time, desired presentation time, and rendering deadline onto the target Renderer (via `Renderer::setPresentationTime`, `Renderer::setDesiredPresentationTime`, and `Renderer::setRenderingDeadline`).

#### Parameters

main

| | |
|---|---|
| renderer | The Filament Renderer displaying the target View. |
