//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[gamutMapping](gamut-mapping.md)

# gamutMapping

[main]\
open fun [gamutMapping](gamut-mapping.md)(gamutMapping: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [ColorGrading.Builder](index.md)

Enables or disables gamut mapping to the destination color space's gamut. When gamut mapping is turned off, out-of-gamut colors are clipped to the destination's gamut, which may produce hue skews (blue skewing to purple, green to yellow, etc.). When gamut mapping is enabled, out-of-gamut colors are brought back in gamut by trying to preserve the perceived chroma and lightness of the original values.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| gamutMapping | Enables or disables gamut mapping |
