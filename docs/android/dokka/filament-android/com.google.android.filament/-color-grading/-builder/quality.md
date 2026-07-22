//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[quality](quality.md)

# quality

[main]\
open fun [quality](quality.md)(qualityLevel: [ColorGrading.QualityLevel](../-quality-level/index.md)): [ColorGrading.Builder](index.md)

Sets the quality level of the color grading. When color grading is implemented using a 3D LUT, the quality level may impact the resolution and bit depth of the backing 3D texture. For instance, a low quality level will use a 16x16x16 10 bit LUT, a medium quality level will use a 32x32x32 10 bit LUT, a high quality will use a 32x32x32 16 bit LUT, and a ultra quality will use a 64x64x64 16 bit LUT. This setting has no effect if generating a 1D LUT. This overrides the values set by format() and dimensions(). The default quality is [MEDIUM](../-quality-level/-m-e-d-i-u-m/index.md).

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| qualityLevel | The desired quality of the color grading process |
