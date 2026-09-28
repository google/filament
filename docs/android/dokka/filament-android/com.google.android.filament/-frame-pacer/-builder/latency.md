//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[FramePacer](../index.md)/[Builder](index.md)/[latency](latency.md)

# latency

[main]\
open fun [latency](latency.md)(latency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.Builder](index.md)

Sets the required latency window in terms of time duration. 

The target latency is measured relative to the hardware VSYNC timestamp (VsyncTick::baseTime), as opposed to the callback entry/dispatch time. Note that the hardware VSYNC time is always equal to or in the past with respect to the actual callback dispatch time.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| latency | Target latency duration (defaults to 33ms). |
