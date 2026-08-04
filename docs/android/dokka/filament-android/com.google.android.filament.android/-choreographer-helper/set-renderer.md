//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[ChoreographerHelper](index.md)/[setRenderer](set-renderer.md)

# setRenderer

[main]\
open fun [setRenderer](set-renderer.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md))

Attaches an optional Filament [Renderer](../../com.google.android.filament/-renderer/index.md) to be automatically paced by this helper. 

When configured, the helper automatically calls [setDesiredPresentationTime](../../com.google.android.filament/-renderer/set-desired-presentation-time.md) and [setRenderingDeadline](../../com.google.android.filament/-renderer/set-rendering-deadline.md) using the preferred `FrameTimeline` on Android 13+. 

If you are using `FramePacer` in your rendering loop, leave this set to `null`.

#### Parameters

main

| | |
|---|---|
| renderer | The [Renderer](../../com.google.android.filament/-renderer/index.md) to configure, or `null` to disable automated timeline pacing. |
