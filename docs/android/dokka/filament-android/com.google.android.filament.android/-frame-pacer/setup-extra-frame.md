//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[setupExtraFrame](setup-extra-frame.md)

# setupExtraFrame

[main]\
open fun [setupExtraFrame](setup-extra-frame.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Advances the internal pacing pipeline to target an extra presentation frame in the future, without advancing the ideal cadence clock (mExpectedBaseTime). This method is explicitly designed for latency recovery (Buffer Stuffing). If the pipeline reports PacingStatus.DISPLAY_STARVING, the application may yield, OR it may choose to render an extra frame during the current Vsync callback to mechanically stuff the queue depth back to its target latency. Calling this method instantly extrapolates the presentation timestamp one target frame period into the future. The subsequent call to `applyPresentationTime` will emit this new timestamp, allowing the queue to grow without the FramePacer erroneously rejecting the next real Vsync as spurious. This method acts as a safeguard against runaway clock extrapolation. It returns true if the timestamp was successfully advanced, or false if it refused to stuff another frame because the pipeline is already at or beyond the configured target latency. 

```kotlin
if (pacer.setupFrame(tick) == FramePacer.FrameStatus.ACCEPTED) {
    pacer.applyPresentationTime(renderer);
    if (renderer.beginFrame(swapChain)) {
        renderer.render(view);
        renderer.endFrame();
    }

    // Automatically recover queue depth by stuffing an extra frame
    // If the application was starved by multiple frames, it will receive DISPLAY_STARVING
    // over consecutive Vsync callbacks, naturally amortizing the recovery over time.
    if (pacer.getPacingStatus() == FramePacer.PacingStatus.DISPLAY_STARVING) {
        if (pacer.setupExtraFrame()) {
            pacer.applyPresentationTime(renderer); // Emit the future-shifted timestamp
            if (renderer.beginFrame(swapChain)) {
                // Optional: advance simulation state again here
                renderer.render(view);
                renderer.endFrame();
            }
        }
    }
}

```

#### Return

true if the timestamp was safely advanced, false if refused to prevent over-stuffing.
