//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[customLut](custom-lut.md)

# customLut

[main]\
open fun [customLut](custom-lut.md)(data: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), dimension: Int): [ColorGrading.Builder](index.md)

Specifies a custom 3D color grading LUT to map the final sRGB color. 

The LUT is applied after post-processing and in LDR (sRGB space). The data must be a 3D array of float3 (RGB) values. The dimension does not need to be a power of two, but must be non-zero. The values are always interpolated (trilinear) because the input color from previous steps is continuous. The dimension doesn't need to match dimensions(). If the dimension is 0 or the data is empty, the custom LUT is skipped (ignored).

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| data | FixedCapacityVector containing the custom LUT data (3D array of float3). |
| dimension | Dimension of the custom LUT. |
