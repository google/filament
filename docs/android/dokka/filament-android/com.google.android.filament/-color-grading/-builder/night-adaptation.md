//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[nightAdaptation](night-adaptation.md)

# nightAdaptation

[main]\
open fun [nightAdaptation](night-adaptation.md)(adaptation: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Controls the amount of night adaptation to replicate a more natural representation of low-light conditions as perceived by the human vision system. In low-light conditions, peak luminance sensitivity of the eye shifts toward the blue end of the color spectrum: darker tones appear brighter, reducing contrast, and colors are blue shifted (the darker the more intense the effect).

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| adaptation | Amount of adaptation, between 0 (no adaptation) and 1 (full adaptation). |
