//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[FramePacer](../index.md)/[Builder](index.md)/[latencyNanos](latency-nanos.md)

# latencyNanos

[main]\
open fun [latencyNanos](latency-nanos.md)(latencyNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.Builder](index.md)

Sets the required latency window in terms of time duration. 

The target latency is measured relative to the hardware VSYNC timestamp (`frameTimeNanos`), as opposed to the callback entry/dispatch time. Note that the hardware VSYNC time is always equal to or in the past with respect to the actual callback dispatch time.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| latencyNanos | Target latency duration in nanoseconds (defaults to 33.3ms). Must be greater than 0. |
