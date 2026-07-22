//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setStencilBufferEnabled](set-stencil-buffer-enabled.md)

# setStencilBufferEnabled

[main]\
open fun [setStencilBufferEnabled](set-stencil-buffer-enabled.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables use of the stencil buffer. 

 The stencil buffer is an 8-bit, per-fragment unsigned integer stored alongside the depth buffer. The stencil buffer is cleared at the beginning of a frame and discarded after the color pass. 

 Each fragment's stencil value is set during rasterization by specifying stencil operations on a [Material](../-material/index.md). The stencil buffer can be used as a mask for later rendering by setting a [Material](../-material/index.md)'s stencil comparison function and reference value. Fragments that don't pass the stencil test are then discarded. 

 If post-processing is disabled, then the SwapChain must have the CONFIG_HAS_STENCIL_BUFFER flag set in order to use the stencil buffer. 

 A renderable's priority (see [setPriority](../-renderable-manager/set-priority.md)) is useful to control the order in which primitives are drawn. 

#### Parameters

main

| | |
|---|---|
| enabled | True to enable the stencil buffer, false disables it (default) |
