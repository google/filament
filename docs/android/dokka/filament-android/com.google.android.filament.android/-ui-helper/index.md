//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[UiHelper](index.md)

# UiHelper

[main]\
open class [UiHelper](index.md)

UiHelper is a simple class that can manage either a SurfaceView, TextureView, or a SurfaceHolder so it can be used to render into with Filament. Here is a simple example with a SurfaceView. The code would be exactly the same with a TextureView: 

```kotlin
public class FilamentActivity extends Activity {
    private UiHelper mUiHelper;
    private SurfaceView mSurfaceView;

    // Filament specific APIs
    private Engine mEngine;
    private Renderer mRenderer;
    private View mView; // com.google.android.filament.View, not android.view.View
    private SwapChain mSwapChain;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Create a SurfaceView and add it to the activity
        mSurfaceView = new SurfaceView(this);
        setContentView(mSurfaceView);

        // Create the Filament UI helper
        mUiHelper = new UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK);

        // Attach the SurfaceView to the helper, you could do the same with a TextureView
        mUiHelper.attachTo(mSurfaceView);

        // Set a rendering callback that we will use to invoke Filament
        mUiHelper.setRenderCallback(new UiHelper.RendererCallback() {
            public void onNativeWindowChanged(Surface surface) {
                if (mSwapChain != null) mEngine.destroySwapChain(mSwapChain);
                mSwapChain = mEngine.createSwapChain(surface, mUiHelper.getSwapChainFlags());
            }

            // The native surface went away, we must stop rendering.
            public void onDetachedFromSurface() {
                if (mSwapChain != null) {
                    mEngine.destroySwapChain(mSwapChain);

                    // Required to ensure we don't return before Filament is done executing the
                    // destroySwapChain command, otherwise Android might destroy the Surface
                    // too early
                    mEngine.flushAndWait();

                    mSwapChain = null;
                }
            }

            // The native surface has changed size. This is always called at least once
            // after the surface is created (after onNativeWindowChanged() is invoked).
            public void onResized(int width, int height) {

                // Wait for all pending frames to be processed before returning. This is to
                // avoid a race between the surface being resized before pending frames are
                // rendered into it.
                Fence fence = mEngine.createFence();
                fence.wait(Fence.Mode.FLUSH, Fence.WAIT_FOR_EVER);
                mEngine.destroyFence(fence);

                // Compute camera projection and set the viewport on the view
            }
        });

        mEngine = Engine.create();
        mRenderer = mEngine.createRenderer();
        mView = mEngine.createView();
        // Create scene, camera, etc.
    }

    public void onDestroy() {
        super.onDestroy();
        // Always detach the surface before destroying the engine
        mUiHelper.detach();

        mEngine.destroy();
    }

    // This is an example of a render function. You will most likely invoke this from
    // a Choreographer callback to trigger rendering at vsync.
    public void render() {
        if (mUiHelper.isReadyToRender) {
            // If beginFrame() returns false you should skip the frame
            // This means you are sending frames too quickly to the GPU
            if (mRenderer.beginFrame(swapChain)) {
                mRenderer.render(mView);
                mRenderer.endFrame();
            }
        }
    }
}

```

## Constructors

| | |
|---|---|
| [UiHelper](-ui-helper.md) | [main]<br>constructor()<br>Creates a UiHelper which will help manage the native surface provided by a SurfaceView or a TextureView.<br>constructor(policy: [UiHelper.ContextErrorPolicy](-context-error-policy/index.md))<br>Creates a UiHelper which will help manage the native surface provided by a SurfaceView or a TextureView. |

## Types

| Name | Summary |
|---|---|
| [ContextErrorPolicy](-context-error-policy/index.md) | [main]<br>enum [ContextErrorPolicy](-context-error-policy/index.md)<br>Enum used to decide whether UiHelper should perform extra error checking. |
| [RendererCallback](-renderer-callback/index.md) | [main]<br>interface [RendererCallback](-renderer-callback/index.md)<br>Interface used to know when the native surface is created, destroyed or resized. |

## Functions

| Name | Summary |
|---|---|
| [attachTo](attach-to.md) | [main]<br>open fun [attachTo](attach-to.md)(holder: SurfaceHolder)<br>Associate UiHelper with a SurfaceHolder.<br>[main]<br>open fun [attachTo](attach-to.md)(view: SurfaceView)<br>Associate UiHelper with a SurfaceView.<br>[main]<br>open fun [attachTo](attach-to.md)(view: TextureView)<br>Associate UiHelper with a TextureView. |
| [detach](detach.md) | [main]<br>open fun [detach](detach.md)()<br>Free resources associated to the native window specified in [attachTo](attach-to.md), [attachTo](attach-to.md), or [attachTo](attach-to.md). |
| [getDesiredHeight](get-desired-height.md) | [main]<br>open fun [getDesiredHeight](get-desired-height.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the requested height for the native surface. |
| [getDesiredWidth](get-desired-width.md) | [main]<br>open fun [getDesiredWidth](get-desired-width.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the requested width for the native surface. |
| [getRenderCallback](get-render-callback.md) | [main]<br>open fun [getRenderCallback](get-render-callback.md)(): [UiHelper.RendererCallback](-renderer-callback/index.md)<br>Returns the current render callback associated with this UiHelper. |
| [getSwapChainFlags](get-swap-chain-flags.md) | [main]<br>open fun [getSwapChainFlags](get-swap-chain-flags.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the flags to pass to [createSwapChain](../../com.google.android.filament/-engine/create-swap-chain.md) to honor all the options set on this UiHelper. |
| [isMediaOverlay](is-media-overlay.md) | [main]<br>open fun [isMediaOverlay](is-media-overlay.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns true if the SurfaceView used as a render target should be positioned above other surfaces but below the activity's surface. |
| [isOpaque](is-opaque.md) | [main]<br>open fun [isOpaque](is-opaque.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns true if the render target is opaque. |
| [isReadyToRender](is-ready-to-render.md) | [main]<br>open fun [isReadyToRender](is-ready-to-render.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks whether we are ready to render into the attached surface. |
| [setDesiredSize](set-desired-size.md) | [main]<br>open fun [setDesiredSize](set-desired-size.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Set the size of the render target buffers of the native surface. |
| [setMediaOverlay](set-media-overlay.md) | [main]<br>open fun [setMediaOverlay](set-media-overlay.md)(overlay: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Controls whether the surface of the SurfaceView used as a render target should be positioned above other surfaces but below the activity's surface. |
| [setOpaque](set-opaque.md) | [main]<br>open fun [setOpaque](set-opaque.md)(opaque: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Controls whether the render target (SurfaceView or TextureView) is opaque or not. |
| [setRenderCallback](set-render-callback.md) | [main]<br>open fun [setRenderCallback](set-render-callback.md)(renderCallback: [UiHelper.RendererCallback](-renderer-callback/index.md))<br>Sets the renderer callback that will be notified when the native surface is created, destroyed or resized. |
