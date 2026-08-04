//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[format](format.md)

# format

[main]\
open fun [format](format.md)(format: [ColorGrading.LutFormat](../-lut-format/index.md)): [ColorGrading.Builder](index.md)

When color grading is implemented using a 3D LUT, this sets the texture format of of the LUT. This overrides the value set by quality(). This setting has no effect if generating a 1D LUT. The default is INTEGER

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| format | The desired format of the 3D LUT. |
