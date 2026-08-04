//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[luminanceScaling](luminance-scaling.md)

# luminanceScaling

[main]\
open fun [luminanceScaling](luminance-scaling.md)(luminanceScaling: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [ColorGrading.Builder](index.md)

Enables or disables the luminance scaling component (LICH) from the exposure value invariant luminance system (EVILS). When this setting is enabled, pixels with high chromatic values will roll-off to white to offer a more natural rendering. This step also helps avoid undesirable hue skews caused by out of gamut colors clipped to the destination color gamut. When luminance scaling is enabled, tone mapping is performed on the luminance of each pixel instead of per-channel.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| luminanceScaling | Enables or disables EVILS post-tone mapping |
