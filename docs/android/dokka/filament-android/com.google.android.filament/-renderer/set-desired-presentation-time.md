//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setDesiredPresentationTime](set-desired-presentation-time.md)

# setDesiredPresentationTime

[main]\
open fun [setDesiredPresentationTime](set-desired-presentation-time.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Set the real desired presentation time targeted for this frame. Unlike setPresentationTime(), which configures hardware headroom, this is the exact target presentation time and is used for FrameInfo frame history reporting. This must be called before [endFrame](end-frame.md).

#### Parameters

main

| | |
|---|---|
| monotonicClockNanos | The desired presentation timestamp in nanoseconds on the steady clock. |
