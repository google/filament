//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[setFrameCompletedCallback](set-frame-completed-callback.md)

# setFrameCompletedCallback

[main]\
open fun [setFrameCompletedCallback](set-frame-completed-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

FrameCompletedCallback is a callback function that notifies an application when a frame's contents have completed rendering on the GPU. 

Use SwapChain::setFrameCompletedCallback to set a callback on an individual SwapChain. Each time a frame completes GPU rendering, the callback will be called.

If handler is nullptr, the callback is guaranteed to be called on the main Filament thread.

Use \c setFrameCompletedCallback() (with default arguments) to unset the callback.

#### Parameters

main

| | |
|---|---|
| handler | Handler to dispatch the callback or nullptr for the default handler. |
| callback | Callback called when each frame completes.<br>@remark Only Filament's Metal backend supports frame callbacks. Other backends ignore the callback (which will never be called) and proceed normally. |

#### See also

| |
|---|
| CallbackHandler |
