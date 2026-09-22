//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setDesiredPresentationTime](set-desired-presentation-time.md)

# setDesiredPresentationTime

[main]\
open fun [setDesiredPresentationTime](set-desired-presentation-time.md)(monotonic_clock_ns: Long)

Set the real desired presentation time targeted for this frame. 

Unlike setPresentationTime(), which configures hardware headroom, this is the exact target presentation time and is used for FrameInfo frame history reporting.

This must be called before endFrame().

#### Parameters

main

| | |
|---|---|
| monotonic_clock_ns | the desired presentation timestamp in nanoseconds on the steady clock. |
