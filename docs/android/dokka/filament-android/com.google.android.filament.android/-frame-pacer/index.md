//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)

# FramePacer

[main]\
open class [FramePacer](index.md)

Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures. 

The FramePacer decouples the CPU rendering loop from fluctuating hardware display cadences, acting as a deterministic filter between incoming platform VSYNC events (such as Android's Choreographer) and native buffer presentation submissions (via [setPresentationTime](../../com.google.android.filament/-renderer/set-presentation-time.md)).

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
| [configure](configure.md) | [main]<br>open fun [configure](configure.md)(config: [FramePacer.Configuration](-configuration/index.md))<br>Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).<br>[main]<br>open fun [configure](configure.md)(targetFrameRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), latencyNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(engine: [Engine](../../com.google.android.filament/-engine/index.md))<br>Destroys this FramePacer instance and frees all associated native resources. |
| [getEffectiveLatencyNanos](get-effective-latency-nanos.md) | [main]<br>open fun [getEffectiveLatencyNanos](get-effective-latency-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the effective target latency in nanoseconds. |
| [getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md) | [main]<br>open fun [getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the target presentation timestamp computed during the most recent call to setupFrame(). |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getPacingStatus](get-pacing-status.md) | [main]<br>open fun [getPacingStatus](get-pacing-status.md)(): [FramePacer.PacingStatus](-pacing-status/index.md)<br>Returns the current flow control status of the pacing pipeline. |
| [getRenderingDeadlineNanos](get-rendering-deadline-nanos.md) | [main]<br>open fun [getRenderingDeadlineNanos](get-rendering-deadline-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the target rendering deadline timestamp computed during the most recent call to setupFrame(). |
| [getSelectedFrameRate](get-selected-frame-rate.md) | [main]<br>open fun [getSelectedFrameRate](get-selected-frame-rate.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the actual pacing frame rate selected during the active frame pacing cycle. |
| [hasGpuFallenBehind](has-gpu-fallen-behind.md) | [main]<br>open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks if the GPU rendering pipeline has fallen behind the CPU submission rate. |
| [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md) | [main]<br>open fun [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether the selected pacing frame rate is achieved exactly by the display hardware. |
| [resetPacing](reset-pacing.md) | [main]<br>open fun [resetPacing](reset-pacing.md)()<br>Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor to the configured target latency on the next frame. |
| [setupExtraFrame](setup-extra-frame.md) | [main]<br>open fun [setupExtraFrame](setup-extra-frame.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Advances the internal pacing pipeline to target an extra presentation frame in the future, without advancing the ideal cadence clock (mExpectedBaseTime). |
| [setupFrame](setup-frame.md) | [main]<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)<br>open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)<br>Prepares and evaluates the frame pacing state for the upcoming frame cycle.<br>[main]<br>open fun [setupFrame](setup-frame.md)(frameData: FrameData, vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)<br>Prepares and evaluates the frame pacing state for the upcoming frame cycle using Android 13+ FrameData. |
