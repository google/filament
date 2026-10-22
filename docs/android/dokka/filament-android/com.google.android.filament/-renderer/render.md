//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[render](render.md)

# render

[main]\
open fun [render](render.md)(view: [View](../-view/index.md))

Render a View into this renderer's window. 

This is filament main rendering method, most of the CPU-side heavy lifting is performed here. render() main function is to generate render commands which are asynchronously executed by the Engine's render thread.

render() generates commands for each of the following stages:

1. Shadow map passes, if needed.
2. Depth pre-pass.
3. Color pass.
4. Post-processing pass. A typical render loop looks like this:

```kotlin

#include <filament/Renderer.h>
#include <filament/View.h>
using namespace filament;

void renderLoop(Renderer* renderer, SwapChain* swapChain) {
    do {
        // typically we wait for VSYNC and user input events
        if (renderer->beginFrame(swapChain)) {
            renderer->render(mView);
            renderer->endFrame();
        }
    } while (!quit());
}

```

render() must be called from the Engine's main thread (or external synchronization must be provided). In particular, calls to render() on different Renderer instances **must** be synchronized.

#### Parameters

main

| | |
|---|---|
| view | A pointer to the view to render.<br>@attention render() must be called *after* beginFrame() and *before* endFrame().<br>@remark render() perform potentially heavy computations and cannot be multi-threaded. However, internally, render() is highly multi-threaded to both improve performance in mitigate the call's latency.<br>@remark render() is typically called once per frame (but not necessarily). |

#### See also

| |
|---|
| beginFrame |
| [endFrame](end-frame.md) |
| [View](../-view/index.md) |
