//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[setFrameScheduledCallback](set-frame-scheduled-callback.md)

# setFrameScheduledCallback

[main]\
open fun [setFrameScheduledCallback](set-frame-scheduled-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

FrameScheduledCallback is a callback function that notifies an application about the status of a frame after Filament has finished its processing. 

 The exact timing and semantics of this callback differ depending on the graphics backend in use. 

### Metal Backend

 With the Metal backend, this callback signifies that Filament has completed all CPU-side processing for a frame and the frame is ready to be scheduled for presentation. 

 Typically, Filament is responsible for scheduling the frame's presentation to the SwapChain. If a FrameScheduledCallback is set, however, the application bears the responsibility of scheduling the frame for presentation by calling the PresentCallable passed to the callback function. In this mode, Filament will not automatically schedule the frame for presentation. 

 When using the Metal backend, if your application delays the call to the PresentCallable (e.g., by invoking it on a separate thread), you must ensure all PresentCallables have been called before shutting down the Filament Engine. You can guarantee this by calling Engine.flushAndWait() before Engine.shutdown(). This is necessary to ensure the Engine has a chance to clean up all memory related to frame presentation. 

### Other Backends (OpenGL, Vulkan, WebGPU)

 On other backends, this callback serves as a notification that Filament has completed all CPU-side processing for a frame. Filament proceeds with its normal presentation logic automatically, and the PresentCallable passed to the callback is a no-op that can be safely ignored. 

### General Behavior

 A FrameScheduledCallback can be set on an individual SwapChain through setFrameScheduledCallback. Each SwapChain can have only one callback set per frame. If setFrameScheduledCallback is called multiple times on the same SwapChain before Renderer.endFrame(), the most recent call effectively overwrites any previously set callback. 

 The callback set by setFrameScheduledCallback is &quot;latched&quot; when Renderer.endFrame() is executed. At this point, the callback is fixed for the frame that was just encoded. Subsequent calls to setFrameScheduledCallback after endFrame() will apply to the next frame. 

 Use setFrameScheduledCallback() (with default arguments) to unset the callback. 

#### Parameters

main

| | |
|---|---|
| handler | A [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). |
| callback | The Runnable callback to invoke when frame processing is complete. |
