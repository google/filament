//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[lispsm](lispsm.md)

# lispsm

[main]\
open var [lispsm](lispsm.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

LiSPSM, or light-space perspective shadow-mapping is a technique allowing to better optimize the use of the shadow-map texture. When enabled the effective resolution of shadows is greatly improved and yields result similar to using cascades without the extra cost. LiSPSM comes with some drawbacks however, in particular it is incompatible with blurring because it effectively affects the blur kernel size. Blurring is only an issue when using ShadowType.VSM with a large blur or with ShadowType.PCSS however. If these blurring artifacts become problematic, this flag can be used to disable LiSPSM.
