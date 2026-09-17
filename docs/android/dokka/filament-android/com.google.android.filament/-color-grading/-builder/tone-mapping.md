//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[toneMapping](tone-mapping.md)

# toneMapping

[main]\
open fun [toneMapping](tone-mapping.md)(toneMapping: [ColorGrading.ToneMapping](../-tone-mapping/index.md)): [ColorGrading.Builder](index.md)

Selects the tone mapping operator to apply to the HDR color buffer as the last operation of the color grading post-processing step. 

The default tone mapping operator is ACES_LEGACY.

#### Return

This Builder, for chaining calls

#### Deprecated

Use toneMapper(ToneMapper*) instead

#### Parameters

main

| | |
|---|---|
| toneMapping | The tone mapping operator to apply to the HDR color buffer |
