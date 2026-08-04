//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[isExactFrameRateAchieved](is-exact-frame-rate-achieved.md)

# isExactFrameRateAchieved

[main]\
open fun [isExactFrameRateAchieved](is-exact-frame-rate-achieved.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Returns whether the selected pacing frame rate is achieved exactly by the display hardware.

#### Return

true if the selected rate is an exact integer fraction of the host display platform's refresh rate, false if non-integer ratio pacing is active (such as 45 FPS on 60Hz).
