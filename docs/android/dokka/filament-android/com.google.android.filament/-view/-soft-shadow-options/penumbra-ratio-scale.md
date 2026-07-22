//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[SoftShadowOptions](index.md)/[penumbraRatioScale](penumbra-ratio-scale.md)

# penumbraRatioScale

[main]\
open var [penumbraRatioScale](penumbra-ratio-scale.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Sets a global scale factor applied to the PCSS geometric ratio before failsafe clamping. This parameter controls the &quot;contact shadow contrast&quot; or the rate at which shadows transition from sharp to soft. By scaling the geometric ratio, you can create highly dramatic, cinematic shadows that blur rapidly as they move away from the contact point, completely independently of the light's overall physical size. - Values >1.0 cause the shadow to accelerate toward its maximum softness faster. - Values <1.0 cause the shadow to stay sharper for a longer distance. The final ratio scale applied is calculated by modulating this global value with the light's individual penumbraRatioScale (global * local). The global penumbra ratio scale multiplier. Default is 1.0.

#### See also

| | |
|---|---|
| [LightManager](../../-light-manager/index.md) | ::ShadowOptions::penumbraRatioScale |
