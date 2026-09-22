//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setPresentationTime](set-presentation-time.md)

# setPresentationTime

[main]\
open fun [setPresentationTime](set-presentation-time.md)(monotonic_clock_ns: Long)

Set the time at which the frame must be presented to the display hardware. 

This value is used to configure the hardware and must typically be strictly smaller than the desired presentation time (i.e. it must include some headroom but not too much). For instance, on Android, it is typically set to desired_presentation_time - vsync_period / 2. This behavior can vary on other platforms.

This must be called before endFrame().

#### Parameters

main

| | |
|---|---|
| monotonic_clock_ns | the presentation configuration timestamp in nanoseconds on the steady clock. |
