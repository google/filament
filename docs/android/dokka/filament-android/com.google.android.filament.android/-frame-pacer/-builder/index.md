//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[FramePacer](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Constructs a new `FramePacer` instance.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../../com.google.android.filament/-engine/index.md)): [FramePacer](../index.md)<br>Creates the FramePacer object and returns a pointer to it. |
| [latencyFrames](latency-frames.md) | [main]<br>open fun [latencyFrames](latency-frames.md)(frames: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [FramePacer.Builder](index.md)<br>Sets the required latency window in terms of 60Hz display frames. |
| [latencyNanos](latency-nanos.md) | [main]<br>open fun [latencyNanos](latency-nanos.md)(latencyNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.Builder](index.md)<br>Sets the required latency window in terms of time duration. |
| [targetFrameRate](target-frame-rate.md) | [main]<br>open fun [targetFrameRate](target-frame-rate.md)(fps: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [FramePacer.Builder](index.md)<br>Sets the desired frame rendering step in Hz. |
