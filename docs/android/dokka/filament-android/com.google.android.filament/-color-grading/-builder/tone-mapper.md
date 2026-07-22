//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[toneMapper](tone-mapper.md)

# toneMapper

[main]\
open fun [toneMapper](tone-mapper.md)(toneMapper: [ToneMapper](../../-tone-mapper/index.md)): [ColorGrading.Builder](index.md)

Selects the tone mapping operator to apply to the HDR color buffer as the last operation of the color grading post-processing step. The default tone mapping operator is [ToneMapper.ACESLegacy](../../-tone-mapper/-a-c-e-s-legacy/index.md). The specified tone mapper must have a lifecycle that exceeds the lifetime of this builder. Since the build(Engine&) method is synchronous, it is safe to delete the tone mapper object after that finishes executing.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| toneMapper | The tone mapping operator to apply to the HDR color buffer |
