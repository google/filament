//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[customLut](custom-lut.md)

# customLut

[main]\
open fun [customLut](custom-lut.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), dimension: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [ColorGrading.Builder](index.md)

Specifies a custom 3D color grading LUT to map the final sRGB color. The LUT is applied after post-processing and in LDR (sRGB space). The data must be a 3D array of float3 (RGB) values. The data must remain valid until build() is called.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| buffer | Direct ByteBuffer containing the custom LUT data (3D array of float3). |
| dimension | Dimension of the custom LUT (e.g., 16, 32, 64). |
