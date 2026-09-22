//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)

# FramePacer

[main]\
open class [FramePacer](index.md)

FramePacer 

Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures.

The FramePacer decouples the CPU rendering loop from fluctuating hardware display cadences, acting as a deterministic filter between incoming platform VSYNC callbacks (such as AChoreographer on Android) and native buffer presentation submissions (via Renderer::setPresentationTime).

# Creation and destruction

A FramePacer object is created using the FramePacer::Builder and destroyed by calling Engine::destroy(const FramePacer*).

```kotlin

 filament::Engine* engine = filament::Engine::create();

 filament::FramePacer* pacer = filament::FramePacer::Builder()
             .targetFrameRate(60.0f)
             .latency(std::chrono::milliseconds(33)) // Configure 33ms target latency duration (~2 60Hz frames)
             .build(*engine);

 // Inside your application's AChoreographer frame callback:
 void onChoreographerTick(const filament::FramePacer::VsyncTick& tick) {
     if (pacer->setupFrame(tick) != filament::FramePacer::FrameStatus::ACCEPTED) {
         // Yield or skip rendering to maintain pacing
         return;
     }

     // Evaluate if the GPU is delayed, bypassing FrameSkipper check via Renderer::shouldRenderFrame()
     if (pacer->hasGpuFallenBehind(renderer)) {
         renderer->skipFrame(tick.baseTime);
         return;
     }

     // Set presentation time which automatically switches Renderer to "paced mode"
     pacer->applyPresentationTime(renderer);

     if (renderer->beginFrame(swapChain)) {
         renderer->render(view);
         renderer->endFrame();
     }
 }

 engine->destroy(pacer);

```

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use Builder to construct a FramePacer object instance |
| [Configuration](-configuration/index.md) | [main]<br>open class [Configuration](-configuration/index.md)<br>Holds dynamic pacing targets and latency pipeline depth requirements. |
| [FrameStatus](-frame-status/index.md) | [main]<br>enum [FrameStatus](-frame-status/index.md) |
| [HardwareTimeline](-hardware-timeline/index.md) | [main]<br>open class [HardwareTimeline](-hardware-timeline/index.md)<br>Telemetry for a single expected hardware presentation timeline. |
| [PacingStatus](-pacing-status/index.md) | [main]<br>enum [PacingStatus](-pacing-status/index.md) |
| [VsyncTick](-vsync-tick/index.md) | [main]<br>open class [VsyncTick](-vsync-tick/index.md)<br>Encapsulates VSYNC synchronization telemetry received from the platform compositor. |

## Functions

| Name | Summary |
|---|---|
| [applyPresentationTime](apply-presentation-time.md) | [main]<br>open fun [applyPresentationTime](apply-presentation-time.md)(renderer: [Renderer](../-renderer/index.md))<br>Applies the computed Latency Offset timestamp directly onto the rendering command stream. |
| [configure](configure.md) | [main]<br>open fun [configure](configure.md)(config: [FramePacer.Configuration](-configuration/index.md))<br>Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation). |
| [getConfiguration](get-configuration.md) | [main]<br>open fun [getConfiguration](get-configuration.md)(): [FramePacer.Configuration](-configuration/index.md)<br>Retrieves the current configuration used by the FramePacer. |
| [getEffectiveLatency](get-effective-latency.md) | [main]<br>open fun [getEffectiveLatency](get-effective-latency.md)(): Long<br>Returns the effective target latency in nanoseconds. |
| [getExpectedPresentationTime](get-expected-presentation-time.md) | [main]<br>open fun [getExpectedPresentationTime](get-expected-presentation-time.md)(): Long<br>Returns the target presentation timepoint computed during the most recent call to `setupFrame()`. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getPacingStatus](get-pacing-status.md) | [main]<br>open fun [getPacingStatus](get-pacing-status.md)(): [FramePacer.PacingStatus](-pacing-status/index.md)<br>Returns the current flow control status of the pacing pipeline. |
| [getRenderingDeadline](get-rendering-deadline.md) | [main]<br>open fun [getRenderingDeadline](get-rendering-deadline.md)(): Long<br>Returns the target rendering deadline timepoint computed during the most recent call to `setupFrame()`. |
| [getSelectedFrameRate](get-selected-frame-rate.md) | [main]<br>open fun [getSelectedFrameRate](get-selected-frame-rate.md)(): Float<br>Returns the actual pacing frame rate selected during the active frame pacing cycle. |
| [hasGpuFallenBehind](has-gpu-fallen-behind.md) | [main]<br>open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(renderer: [Renderer](../-renderer/index.md)): Boolean<br>Checks if the GPU rendering pipeline has fallen behind the CPU submission rate. |
| [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md) | [main]<br>open fun [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md)(): Boolean<br>Returns whether the selected pacing frame rate is achieved exactly by the display hardware. |
| [resetPacing](reset-pacing.md) | [main]<br>open fun [resetPacing](reset-pacing.md)()<br>Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor to the configured target latency on the next frame. |
| [setupExtraFrame](setup-extra-frame.md) | [main]<br>open fun [setupExtraFrame](setup-extra-frame.md)(): Boolean<br>Advances the internal pacing pipeline to target an extra presentation frame in the future, without advancing the ideal cadence clock (mExpectedBaseTime). |
| [setupFrame](setup-frame.md) | [main]<br>open fun [setupFrame](setup-frame.md)(tick: [FramePacer.VsyncTick](-vsync-tick/index.md)): [FramePacer.FrameStatus](-frame-status/index.md)<br>Prepares and evaluates the frame pacing state for the upcoming frame cycle.<br>[main]<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: Long): [FramePacer.FrameStatus](-frame-status/index.md)<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: Long, vsyncPeriodNanos: Long): [FramePacer.FrameStatus](-frame-status/index.md)<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: Long, vsyncPeriodNanos: Long, hardwareTimelines: Array&lt;Long&gt;, timelineCount: Int): [FramePacer.FrameStatus](-frame-status/index.md) |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [FramePacer](index.md) |
