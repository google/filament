//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[polygonOffsetConstant](polygon-offset-constant.md)

# polygonOffsetConstant

[main]\
open var [polygonOffsetConstant](polygon-offset-constant.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Constant bias in depth-resolution units by which shadows are moved away from the light. The default value of 0.5 is used to round depth values up. Generally this value shouldn't be changed or at least be small and positive. This is ignored when the View's ShadowType is set to VSM.
