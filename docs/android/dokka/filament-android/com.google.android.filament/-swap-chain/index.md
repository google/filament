//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)

# SwapChain

open class [SwapChain](index.md)

A `SwapChain` represents an Operating System's **native** renderable surface. 

Typically it's a native window or a view. Because a `SwapChain` is initialized from a native object, it is given to filament as an `Object`, which must be of the proper type for each platform filament is running on.

`
 SwapChain swapChain = engine.createSwapChain(nativeWindow); 
 `

The `nativeWindow` parameter above must be of type:

# Examples

## Android

A Surface can be retrieved from a SurfaceView or SurfaceHolder easily using SurfaceHolder.getSurface() and/or SurfaceView.getHolder().

To use a Textureview as a `SwapChain`, it is necessary to first get its SurfaceTexture, for instance using SurfaceTextureListener and then create a Surface:

```kotlin
 // using a TextureView.SurfaceTextureListener:
 public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
     mSurface = new Surface(surfaceTexture);
     // mSurface can now be used with Engine.createSwapChain()
 }

```

#### See also

| |
|---|
| [Engine](../-engine/index.md) |

## Types

| Name | Summary |
|---|---|
| [ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md) | [main]<br>enum [ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md) |
| [FrameRateCompatibility](-frame-rate-compatibility/index.md) | [main]<br>enum [FrameRateCompatibility](-frame-rate-compatibility/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getNativeWindow](get-native-window.md) | [main]<br>open fun [getNativeWindow](get-native-window.md)(): [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html) |
| [isFrameRateChangeSupported](is-frame-rate-change-supported.md) | [main]<br>open fun [isFrameRateChangeSupported](is-frame-rate-change-supported.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Return whether this SwapChain supports the setFrameRate() API. |
| [isFrameScheduledCallbackSet](is-frame-scheduled-callback-set.md) | [main]<br>open fun [isFrameScheduledCallbackSet](is-frame-scheduled-callback-set.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether this SwapChain currently has a FrameScheduledCallback set. |
| [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md) | [main]<br>open fun [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)(engine: [Engine](../-engine/index.md), samples: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Return whether createSwapChain supports the CONFIG_MSAA_*_SAMPLES flag. |
| [isProtectedContentSupported](is-protected-content-supported.md) | [main]<br>open fun [isProtectedContentSupported](is-protected-content-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Return whether createSwapChain supports the CONFIG_PROTECTED_CONTENT flag. |
| [isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md) | [main]<br>open fun [isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Return whether createSwapChain supports the CONFIG_SRGB_COLORSPACE flag. |
| [setFrameCompletedCallback](set-frame-completed-callback.md) | [main]<br>open fun [setFrameCompletedCallback](set-frame-completed-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>FrameCompletedCallback is a callback function that notifies an application when a frame's contents have completed rendering on the GPU. |
| [setFrameRate](set-frame-rate.md) | [main]<br>open fun [setFrameRate](set-frame-rate.md)(frameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>open fun [setFrameRate](set-frame-rate.md)(frameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), compatibility: [SwapChain.FrameRateCompatibility](-frame-rate-compatibility/index.md), strategy: [SwapChain.ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md))<br>Sets the intended frame rate for this SwapChain. |
| [setFrameScheduledCallback](set-frame-scheduled-callback.md) | [main]<br>open fun [setFrameScheduledCallback](set-frame-scheduled-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>FrameScheduledCallback is a callback function that notifies an application about the status of a frame after Filament has finished its processing. |
