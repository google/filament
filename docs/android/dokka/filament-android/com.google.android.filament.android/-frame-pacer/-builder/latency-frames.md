//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[FramePacer](../index.md)/[Builder](index.md)/[latencyFrames](latency-frames.md)

# latencyFrames

[main]\
open fun [latencyFrames](latency-frames.md)(frames: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [FramePacer.Builder](index.md)

Sets the required latency window in terms of 60Hz display frames.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| frames | The latency window in units of 60Hz frames (e.g. 2 frames = 33.3ms). Must be greater than 0. |
