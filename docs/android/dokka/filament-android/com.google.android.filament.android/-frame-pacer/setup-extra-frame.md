//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[setupExtraFrame](setup-extra-frame.md)

# setupExtraFrame

[main]\
open fun [setupExtraFrame](setup-extra-frame.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Advances the internal pacing pipeline to target an extra presentation frame in the future, without advancing the ideal cadence clock (mExpectedBaseTime).

#### Return

true if the timestamp was safely advanced, false if refused to prevent over-stuffing.
