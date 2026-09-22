//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[FramePacer](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct a FramePacer object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [FramePacer](../index.md)<br>Creates the FramePacer object and returns a pointer to it. |
| [latency](latency.md) | [main]<br>open fun [latency](latency.md)(latency: Long): [FramePacer.Builder](index.md)<br>Sets the required latency window in terms of time duration. |
| [latencyFrames](latency-frames.md) | [main]<br>open fun [latencyFrames](latency-frames.md)(frames: Int): [FramePacer.Builder](index.md)<br>Sets the required latency window in terms of 60Hz display frames. |
| [targetFrameRate](target-frame-rate.md) | [main]<br>open fun [targetFrameRate](target-frame-rate.md)(fps: Float): [FramePacer.Builder](index.md)<br>Sets the desired frame rendering step in Hz. |
