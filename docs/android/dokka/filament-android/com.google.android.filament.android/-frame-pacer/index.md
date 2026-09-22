//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)

# FramePacer

[main]\
open class [FramePacer](index.md)

Android helper for [com.google.android.filament.FramePacer](../../com.google.android.filament/-frame-pacer/index.md). 

Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures and integrates directly with Android's Choreographer APIs (including Android 13+ Choreographer.FrameData).

## Constructors

| | |
|---|---|
| [FramePacer](-frame-pacer.md) | [main]<br>constructor(framePacer: [FramePacer](../../com.google.android.filament/-frame-pacer/index.md)) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Constructs a new `FramePacer` instance. |
| [Configuration](-configuration/index.md) | [main]<br>open class [Configuration](-configuration/index.md) |
| [FrameStatus](-frame-status/index.md) | [main]<br>enum [FrameStatus](-frame-status/index.md) |
| [PacingStatus](-pacing-status/index.md) | [main]<br>enum [PacingStatus](-pacing-status/index.md) |

## Functions

| Name | Summary |
|---|---|
| [applyPresentationTime](apply-presentation-time.md) | [main]<br>open fun [applyPresentationTime](apply-presentation-time.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md))<br>Applies the computed Latency Offset timestamp directly onto the rendering command stream. |
| [clearNativeObject](clear-native-object.md) | [main]<br>open fun [clearNativeObject](clear-native-object.md)() |
| [configure](configure.md) | [main]<br>open fun [configure](configure.md)(config: [FramePacer.Configuration](-configuration/index.md))<br>Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).<br>[main]<br>open fun [configure](configure.md)(targetFrameRate: Float, latencyNanos: Long) |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(engine: [Engine](../../com.google.android.filament/-engine/index.md))<br>Destroys this FramePacer instance and frees all associated native resources. |
| [getEffectiveLatency](get-effective-latency.md) | [main]<br>open fun [getEffectiveLatency](get-effective-latency.md)(): Long<br>Returns the effective target latency in nanoseconds. |
| [getEffectiveLatencyNanos](get-effective-latency-nanos.md) | [main]<br>open fun [getEffectiveLatencyNanos](get-effective-latency-nanos.md)(): Long<br>Backwards-compatibility alias for [getEffectiveLatency](get-effective-latency.md). |
| [getExpectedPresentationTime](get-expected-presentation-time.md) | [main]<br>open fun [getExpectedPresentationTime](get-expected-presentation-time.md)(): Long<br>Returns the target presentation timestamp computed during the most recent call to setupFrame(). |
| [getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md) | [main]<br>open fun [getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md)(): Long<br>Backwards-compatibility alias for [getExpectedPresentationTime](get-expected-presentation-time.md). |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getPacer](get-pacer.md) | [main]<br>open fun [getPacer](get-pacer.md)(): [FramePacer](../../com.google.android.filament/-frame-pacer/index.md) |
| [getPacingStatus](get-pacing-status.md) | [main]<br>open fun [getPacingStatus](get-pacing-status.md)(): [FramePacer.PacingStatus](-pacing-status/index.md)<br>Returns the current flow control status of the pacing pipeline. |
| [getRenderingDeadline](get-rendering-deadline.md) | [main]<br>open fun [getRenderingDeadline](get-rendering-deadline.md)(): Long<br>Returns the target rendering deadline timestamp computed during the most recent call to setupFrame(). |
| [getRenderingDeadlineNanos](get-rendering-deadline-nanos.md) | [main]<br>open fun [getRenderingDeadlineNanos](get-rendering-deadline-nanos.md)(): Long<br>Backwards-compatibility alias for [getRenderingDeadline](get-rendering-deadline.md). |
| [getSelectedFrameRate](get-selected-frame-rate.md) | [main]<br>open fun [getSelectedFrameRate](get-selected-frame-rate.md)(): Float<br>Returns the actual pacing frame rate selected during the active frame pacing cycle. |
| [hasGpuFallenBehind](has-gpu-fallen-behind.md) | [main]<br>open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md)): Boolean<br>Checks if the GPU rendering pipeline has fallen behind the CPU submission rate. |
| [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md) | [main]<br>open fun [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md)(): Boolean<br>Returns whether the selected pacing frame rate is achieved exactly by the display hardware. |
| [resetPacing](reset-pacing.md) | [main]<br>open fun [resetPacing](reset-pacing.md)()<br>Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor to the configured target latency on the next frame. |
| [setupExtraFrame](setup-extra-frame.md) | [main]<br>open fun [setupExtraFrame](setup-extra-frame.md)(): Boolean<br>Advances the internal pacing pipeline to target an extra presentation frame in the future, without advancing the ideal cadence clock (mExpectedBaseTime). |
| [setupFrame](setup-frame.md) | [main]<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: Long): [FramePacer.FrameStatus](-frame-status/index.md)<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: Long, vsyncPeriodNanos: Long): [FramePacer.FrameStatus](-frame-status/index.md)<br>Prepares and evaluates the frame pacing state for the upcoming frame cycle.<br>[main]<br>open fun [setupFrame](setup-frame.md)(frameData: FrameData, vsyncPeriodNanos: Long): [FramePacer.FrameStatus](-frame-status/index.md)<br>Prepares and evaluates the frame pacing state for the upcoming frame cycle using Android 13+ FrameData. |
