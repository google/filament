//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setPresentationTime](set-presentation-time.md)

# setPresentationTime

[main]\
open fun [setPresentationTime](set-presentation-time.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Set the time at which the frame must be presented to the display hardware. This value is used to configure the hardware and must typically be strictly smaller than the desired presentation time (i.e. it must include some headroom but not too much). For instance, on Android, it is typically set to desired_presentation_time - vsync_period / 2. This behavior can vary on other platforms. This must be called before [endFrame](end-frame.md).

#### Parameters

main

| | |
|---|---|
| monotonicClockNanos | The presentation configuration timestamp in nanoseconds on the steady clock. |
