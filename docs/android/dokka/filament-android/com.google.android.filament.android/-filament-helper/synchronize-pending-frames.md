//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FilamentHelper](index.md)/[synchronizePendingFrames](synchronize-pending-frames.md)

# synchronizePendingFrames

[main]\
open fun [synchronizePendingFrames](synchronize-pending-frames.md)(engine: [Engine](../../com.google.android.filament/-engine/index.md))

Wait for all pending frames to be processed before returning. This is to avoid a race between the surface being resized before pending frames are rendered into it. 

 For android.view.TextureView this must be called before the texture's size is reconfigured, which unfortunately is done by the Android framework before [UiHelper](../-ui-helper/index.md) listeners are invoked. Therefore `synchronizePendingFrames` cannot be called from onSurfaceTextureSizeChanged; instead a subclass of android.view.TextureView must be used in order to call it from onSizeChanged: 

```kotlin
public class MyTextureView extends TextureView {
    private Engine engine;
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        FilamentHelper.synchronizePendingFrames(engine);
        super.onSizeChanged(w, h, oldw, oldh);
    }
}

```
 Otherwise, this is typically called from [onResized](../-ui-helper/-renderer-callback/on-resized.md), surfaceChanged.

#### Parameters

main

| | |
|---|---|
| engine | Filament engine to synchronize |

#### See also

| |
|---|
| [UiHelper.RendererCallback](../-ui-helper/-renderer-callback/on-resized.md) |
| android.view.SurfaceHolder.Callback#surfaceChanged |
| android.view.TextureView#onSizeChanged |
