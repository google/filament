//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)

# SwapChain

open class [SwapChain](index.md)

A swap chain represents an Operating System's *native* renderable surface. 

Typically, it's a native window or a view. Because a SwapChain is initialized from a native object, it is given to filament as a `void *`, which must be of the proper type for each platform filament is running on.

```kotlin

SwapChain* swapChain = engine->createSwapChain(nativeWindow);

```

When Engine::create() is used without specifying a Platform, the `nativeWindow` parameter above must be of type:

| | |
|---|---|
|  |  |
| Android | ANativeWindow* |
| macOS - OpenGL | NSView* |
| macOS - Metal | CAMetalLayer* |
| iOS - OpenGL | CAEAGLLayer* |
| iOS - Metal | CAMetalLayer* |
| X11 | Window |
| Windows | HWND |

Otherwise, the `nativeWindow` is defined by the concrete implementation of Platform.

Examples:

## Android

On Android, an `ANativeWindow*` can be obtained from a Java `Surface` object using:

```kotlin

 #include <android/native_window_jni.h>
 // parameters
 // env:         JNIEnv*
 // surface:     jobject
 ANativeWindow* win = ANativeWindow_fromSurface(env, surface);

```

A `Surface` can be retrieved from a `SurfaceView` or `SurfaceHolder` easily using `SurfaceHolder.getSurface()` and/or `SurfaceView.getHolder()`.

```kotlin

 // using a TextureView.SurfaceTextureListener:
 public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
     mSurface = new Surface(surfaceTexture);
     // mSurface can now be used in JNI to create an ANativeWindow.
 }

```

## Linux

Example using SDL:

```kotlin

SDL_SysWMinfo wmi;
SDL_VERSION(&wmi.version);
SDL_GetWindowWMInfo(sdlWindow, &wmi);
Window nativeWindow = (Window) wmi.info.x11.window;

using namespace filament;
Engine* engine       = Engine::create();
SwapChain* swapChain = engine->createSwapChain((void*) nativeWindow);

```

## Windows

Example using SDL:

```kotlin

SDL_SysWMinfo wmi;
SDL_VERSION(&wmi.version);
FILAMENT_CHECK_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi)) << "SDL version unsupported!";
HDC nativeWindow = (HDC) wmi.info.win.hdc;

using namespace filament;
Engine* engine       = Engine::create();
SwapChain* swapChain = engine->createSwapChain((void*) nativeWindow);

```

## OSX

On OSX, any `NSView` can be used *directly* as a `nativeWindow` with createSwapChain().

Example using SDL/Objective-C:

```kotlin

 #include <filament/Engine.h>

 #include <Cocoa/Cocoa.h>
 #include <SDL_syswm.h>

 SDL_SysWMinfo wmi;
 SDL_VERSION(&wmi.version);
 NSWindow* win = (NSWindow*) wmi.info.cocoa.window;
 NSView* view = [win contentView];
 void* nativeWindow = view;

 using namespace filament;
 Engine* engine       = Engine::create();
 SwapChain* swapChain = engine->createSwapChain(nativeWindow);

```

Don't use reflection to access the `mNativeObject` field, it won't work.

To use a `TextureView` as a SwapChain, it is necessary to first get its `SurfaceTexture`, for instance using `TextureView.SurfaceTextureListener` and then create a `Surface`:

#### See also

| |
|---|
| [Engine](../-engine/index.md) |

## Types

| Name | Summary |
|---|---|
| [ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md) | [main]<br>enum [ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md)<br>Frame rate change strategy for setFrameRate(). |
| [FrameRateCompatibility](-frame-rate-compatibility/index.md) | [main]<br>enum [FrameRateCompatibility](-frame-rate-compatibility/index.md)<br>Frame rate compatibility mode for setFrameRate(). |

## Properties

| Name | Summary |
|---|---|
| [CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER](-c-a-l-l-b-a-c-k_-d-e-f-a-u-l-t_-u-s-e_-m-e-t-a-l_-c-o-m-p-l-e-t-i-o-n_-h-a-n-d-l-e-r.md) | [main]<br>val [CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER](-c-a-l-l-b-a-c-k_-d-e-f-a-u-l-t_-u-s-e_-m-e-t-a-l_-c-o-m-p-l-e-t-i-o-n_-h-a-n-d-l-e-r.md): Long = 1<br>If this flag is passed to setFrameScheduledCallback, then the behavior of the default CallbackHandler (when nullptr is passed as the handler argument) is altered to call the callback on the Metal completion handler thread (as opposed to the main Filament thread). |
| [CONFIG_APPLE_CVPIXELBUFFER](-c-o-n-f-i-g_-a-p-p-l-e_-c-v-p-i-x-e-l-b-u-f-f-e-r.md) | [main]<br>val [CONFIG_APPLE_CVPIXELBUFFER](-c-o-n-f-i-g_-a-p-p-l-e_-c-v-p-i-x-e-l-b-u-f-f-e-r.md): Long = 8<br>Indicates that the native window is a CVPixelBufferRef. |
| [CONFIG_ENABLE_XCB](-c-o-n-f-i-g_-e-n-a-b-l-e_-x-c-b.md) | [main]<br>val [CONFIG_ENABLE_XCB](-c-o-n-f-i-g_-e-n-a-b-l-e_-x-c-b.md): Long = 4<br>Indicates that the native X11 window is an XCB window rather than an XLIB window. |
| [CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md) | [main]<br>val [CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md): Long = 32<br>Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer. |
| [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md) | [main]<br>val [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md): Long = 128<br>Indicates that the SwapChain is configured to use Multi-Sample Anti-Aliasing (MSAA) with the given sample points within each pixel. |
| [CONFIG_PROTECTED_CONTENT](-c-o-n-f-i-g_-p-r-o-t-e-c-t-e-d_-c-o-n-t-e-n-t.md) | [main]<br>val [CONFIG_PROTECTED_CONTENT](-c-o-n-f-i-g_-p-r-o-t-e-c-t-e-d_-c-o-n-t-e-n-t.md): Long = 64<br>The SwapChain contains protected content. |
| [CONFIG_READABLE](-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md) | [main]<br>val [CONFIG_READABLE](-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md): Long = 2<br>This flag indicates that the swap chain may be used as a source surface for reading back render results. |
| [CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md) | [main]<br>val [CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md): Long = 16<br>Indicates that the SwapChain must automatically perform linear to sRGB encoding. |
| [CONFIG_TRANSPARENT](-c-o-n-f-i-g_-t-r-a-n-s-p-a-r-e-n-t.md) | [main]<br>val [CONFIG_TRANSPARENT](-c-o-n-f-i-g_-t-r-a-n-s-p-a-r-e-n-t.md): Long = 1<br>Requests a SwapChain with an alpha channel. |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getNativeWindow](get-native-window.md) | [main]<br>open fun [getNativeWindow](get-native-window.md)(): Any |
| [isFrameRateChangeSupported](is-frame-rate-change-supported.md) | [main]<br>open fun [isFrameRateChangeSupported](is-frame-rate-change-supported.md)(): Boolean<br>Return whether this SwapChain supports the setFrameRate() API. |
| [isFrameScheduledCallbackSet](is-frame-scheduled-callback-set.md) | [main]<br>open fun [isFrameScheduledCallbackSet](is-frame-scheduled-callback-set.md)(): Boolean<br>Returns whether this SwapChain currently has a FrameScheduledCallback set. |
| [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md) | [main]<br>open fun [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md)(engine: [Engine](../-engine/index.md), samples: Int): Boolean<br>Return whether createSwapChain supports the CONFIG_MSAA_*_SAMPLES flag. |
| [isProtectedContentSupported](is-protected-content-supported.md) | [main]<br>open fun [isProtectedContentSupported](is-protected-content-supported.md)(engine: [Engine](../-engine/index.md)): Boolean<br>Return whether createSwapChain supports the CONFIG_PROTECTED_CONTENT flag. |
| [isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md) | [main]<br>open fun [isSRGBSwapChainSupported](is-s-r-g-b-swap-chain-supported.md)(engine: [Engine](../-engine/index.md)): Boolean<br>Return whether createSwapChain supports the CONFIG_SRGB_COLORSPACE flag. |
| [setFrameCompletedCallback](set-frame-completed-callback.md) | [main]<br>open fun [setFrameCompletedCallback](set-frame-completed-callback.md)(handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>FrameCompletedCallback is a callback function that notifies an application when a frame's contents have completed rendering on the GPU. |
| [setFrameRate](set-frame-rate.md) | [main]<br>open fun [setFrameRate](set-frame-rate.md)(frameRate: Float)<br>open fun [setFrameRate](set-frame-rate.md)(frameRate: Float, compatibility: [SwapChain.FrameRateCompatibility](-frame-rate-compatibility/index.md))<br>open fun [setFrameRate](set-frame-rate.md)(frameRate: Float, compatibility: [SwapChain.FrameRateCompatibility](-frame-rate-compatibility/index.md), strategy: [SwapChain.ChangeFrameRateStrategy](-change-frame-rate-strategy/index.md))<br>Sets the intended frame rate for this SwapChain. |
| [setFrameScheduledCallback](set-frame-scheduled-callback.md) | [main]<br>open fun [setFrameScheduledCallback](set-frame-scheduled-callback.md)(handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>open fun [setFrameScheduledCallback](set-frame-scheduled-callback.md)(handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html), flags: Long)<br>FrameScheduledCallback is a callback function that notifies an application about the status of a frame after Filament has finished its processing. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [SwapChain](index.md)<br>open fun [wrap](wrap.md)(nativeObject: Long, nativeWindow: Any): [SwapChain](index.md) |
