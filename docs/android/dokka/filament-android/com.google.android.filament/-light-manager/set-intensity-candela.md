//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setIntensityCandela](set-intensity-candela.md)

# setIntensityCandela

[main]\
open fun [setIntensityCandela](set-intensity-candela.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's intensity in candela. The intensity can be negative. This method is equivalent to calling setIntensity for directional lights (Type.DIRECTIONAL or Type.SUN).

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| intensity | Luminous intensity in *candela*. |

#### See also

| |
|---|
| [LightManager.Builder](-builder/intensity-candela.md) |
