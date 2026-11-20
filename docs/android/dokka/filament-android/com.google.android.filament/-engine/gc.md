//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[gc](gc.md)

# gc

[main]\
open fun [gc](gc.md)()

Runs one amortized garbage-collection cycle on the Engine's internal component managers, scenes, and deferred asynchronous destructions, and advances the EntityManager epoch so destroyed entity IDs can be recycled. 

This is called automatically by Renderer::endFrame() and Renderer::skipFrame(), so applications using a standard beginFrame() / endFrame() loop do not need to call it. Applications that render exclusively via Renderer::renderStandaloneView(), or that create and destroy entities without a Renderer frame loop, should call gc() periodically (for example, once per iteration of their loop) to prevent entity ID exhaustion and component memory growth.

Must be called from the Engine's main thread, outside of any Renderer::beginFrame() / Renderer::endFrame() block.

Calling gc() invalidates all component Instance handles held from TransformManager, RenderableManager, LightManager, and CameraManager.

#### See also

| |
|---|
| [Renderer](../-renderer/end-frame.md) |
