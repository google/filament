//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[shadowBulbRadius](shadow-bulb-radius.md)

# shadowBulbRadius

[main]\
open var [shadowBulbRadius](shadow-bulb-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

Light bulb radius used for soft shadows. This is only used PCSS. A negative value is used to use a default value for each light type. For Spot and point-lights, this is the radius of the light bulb in meters. For Directional lights, this is tan(angularRadius), or just angularRadius [Rad] for small angles. SUN: getSunAngularRadius() * getSunHaloSize() DIRECTIONAL: 1.0 (1m area light) POINT / SPOT: 0.06 (A19 bulb)
