//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[flushAndWait](flush-and-wait.md)

# flushAndWait

[main]\
open fun [flushAndWait](flush-and-wait.md)()

Kicks the hardware thread (e.g.: the OpenGL, Vulkan or Metal thread) and blocks until all commands to this point are executed. Note that this does guarantee that the hardware is actually finished. 

This is typically used right after destroying the `SwapChain`, in cases where a guarantee about the SwapChain destruction is needed in a timely fashion, such as when responding to Android's surfaceDestroyed.

[main]\
open fun [flushAndWait](flush-and-wait.md)(timeout: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Kicks the hardware thread (e.g. the OpenGL, Vulkan or Metal thread) and blocks until all commands to this point are executed. Note that does guarantee that the hardware is actually finished. A timeout can be specified, if for some reason this flushAndWait doesn't complete before the timeout, it will return false, true otherwise. 

This is typically used right after destroying the `SwapChain`, in cases where a guarantee about the `SwapChain` destruction is needed in a timely fashion, such as when responding to Android's `android.view.SurfaceHolder.Callback.surfaceDestroyed`

#### Return

true if successful, false if flushAndWait timed out, in which case it wasn't successful and commands might still be executing on both the CPU and GPU sides.

#### Parameters

main

| | |
|---|---|
| timeout | A timeout in nanoseconds |
