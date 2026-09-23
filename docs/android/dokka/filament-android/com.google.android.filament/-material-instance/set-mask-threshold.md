//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setMaskThreshold](set-mask-threshold.md)

# setMaskThreshold

[main]\
open fun [setMaskThreshold](set-mask-threshold.md)(threshold: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Overrides the minimum alpha value a fragment must have to not be discarded when the blend mode is MASKED. 

Defaults to 0.4 if it has not been set in the parent Material. The specified value should be between 0 and 1 and will be clamped if necessary.
