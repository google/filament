//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[polygonOffsetSlope](polygon-offset-slope.md)

# polygonOffsetSlope

[main]\
open var [polygonOffsetSlope](polygon-offset-slope.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Bias based on the change in depth in depth-resolution units by which shadows are moved away from the light. The default value of 2.0 works well with SHADOW_SAMPLING_PCF_LOW. Generally this value is between 0.5 and the size in texel of the PCF filter. Setting this value correctly is essential for LISPSM shadow-maps. This is ignored when the View's ShadowType is set to VSM.
