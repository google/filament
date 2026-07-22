//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[applyPresentationTime](apply-presentation-time.md)

# applyPresentationTime

[main]\
open fun [applyPresentationTime](apply-presentation-time.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md))

Applies the computed Latency Offset timestamp directly onto the rendering command stream. 

This instructs the underlying display compositor (such as SurfaceFlinger) exactly when to latch and present the buffer, eliminating micro-stutter. This must be called before [endFrame](../../com.google.android.filament/-renderer/end-frame.md). 

Calling this method automatically applies the target presentation time, desired presentation time, and rendering deadline onto the target Renderer (via [setPresentationTime](../../com.google.android.filament/-renderer/set-presentation-time.md), [setDesiredPresentationTime](../../com.google.android.filament/-renderer/set-desired-presentation-time.md), and [setRenderingDeadline](../../com.google.android.filament/-renderer/set-rendering-deadline.md)).

#### Parameters

main

| | |
|---|---|
| renderer | The Filament Renderer displaying the target View. |
