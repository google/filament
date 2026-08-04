//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[getSelectedFrameRate](get-selected-frame-rate.md)

# getSelectedFrameRate

[main]\
open fun [getSelectedFrameRate](get-selected-frame-rate.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Returns the actual pacing frame rate selected during the active frame pacing cycle. 

If the requested target frame rate is fuzzy-matched to an active display hardware cadence (such as 60.0 FPS requested on a 59.94Hz broadcast screen), this returns the actual hardware rate (59.94 FPS). 

Additionally, if the requested rate exceeds the maximum refresh capability of the active physical display panel (such as 90 FPS requested on a 60Hz screen), this returns the clamped maximum display refresh rate (60 FPS).

#### Return

The active pacing frame rate in frames per second.
