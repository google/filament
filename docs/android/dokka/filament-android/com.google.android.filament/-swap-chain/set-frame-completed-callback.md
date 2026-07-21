//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[setFrameCompletedCallback](set-frame-completed-callback.md)

# setFrameCompletedCallback

[main]\
open fun [setFrameCompletedCallback](set-frame-completed-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

FrameCompletedCallback is a callback function that notifies an application when a frame's contents have completed rendering on the GPU. 

 Use setFrameCompletedCallback to set a callback on an individual SwapChain. Each time a frame completes GPU rendering, the callback will be called. 

 Warning: Only Filament's Metal backend supports frame callbacks. Other backends ignore the callback (which will never be called) and proceed normally. 

#### Parameters

main

| | |
|---|---|
| handler | A [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). |
| callback | The Runnable callback to invoke. |
