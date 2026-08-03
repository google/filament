//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[resetPacing](reset-pacing.md)

# resetPacing

[main]\
open fun [resetPacing](reset-pacing.md)()

Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor to the configured target latency on the next frame. The application can call this when recovering from a dropped frame or to manually re-establish the queue depth. 

```kotlin
if (pacer.setupFrame(tick) == FramePacer.FrameStatus.ACCEPTED) {
    // Alternatively to rendering an extra frame via setupExtraFrame(), the application
    // can accept a one-frame judder and recover queue depth by resetting the pacer.
    // This forces the next frame to rigidly re-anchor to the target latency.
    if (pacer.getPacingStatus() == FramePacer.PacingStatus.DISPLAY_STARVING) {
        pacer.resetPacing();
    }

    pacer.applyPresentationTime(renderer);
    if (renderer.beginFrame(swapChain)) {
        renderer.render(view);
        renderer.endFrame();
    }
}

```
