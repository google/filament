//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[render](render.md)

# render

[main]\
open fun [render](render.md)(view: [View](../-view/index.md))

Renders a [View](../-view/index.md) into this `Renderer`'s window. 

 This is filament's main rendering method, most of the CPU-side heavy lifting is performed here. The purpose of the `render()` function is to generate render commands which are asynchronously executed by the [Engine](../-engine/index.md)'s render thread. 

`render()` generates commands for each of the following stages:

- Shadow map passes, if needed
- Depth pre-pass
- SSAO pass, if enabled
- Color pass
- Post-processing pass

 A typical render loop looks like this: 

```kotlin
void renderLoop(Renderer renderer, SwapChain swapChain) {
    do {
        // typically we wait for VSYNC and user input events
        if (renderer.beginFrame(swapChain)) {
            renderer.render(mView);
            renderer.endFrame();
        }
    } while (!quit());
}

```

- 
   `render()` must be called **after**[beginFrame](begin-frame.md) and **before**[endFrame](end-frame.md).
- 
   `render()` must be called from the [Engine](../-engine/index.md)'s main thread (or external synchronization must be provided). In particular, calls to `render()` on different `Renderer` instances **must** be synchronized.
- 
   `render()` performs potentially heavy computations and cannot be multi-threaded. However, internally, it is highly multi-threaded to both improve performance and mitigate the call's latency.
- 
   `render()` is typically called once per frame (but not necessarily).

#### Parameters

main

| | |
|---|---|
| view | the [View](../-view/index.md) to render |

#### See also

| |
|---|
| [beginFrame](begin-frame.md) |
| [endFrame](end-frame.md) |
| [View](../-view/index.md) |

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if the backend thread encountered an unrecoverable error, or if called again after a backend exception was already thrown. |
