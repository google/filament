//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setIntensity](set-intensity.md)

# setIntensity

[main]\
open fun [setIntensity](set-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's intensity. The intensity can be negative.

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| intensity | This parameter depends on the [Type](-type/index.md), for directional lights, it specifies the illuminance in *lux* (or *lumen/m^2*). For point lights and spot lights, it specifies the luminous power in *lumen*. For example, the sun's illuminance is about 100,000 lux. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |

[main]\
open fun [setIntensity](set-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), watts: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), efficiency: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's intensity. The intensity can be negative. 

```kotlin
 Lightbulb type  | Efficiency
-----------------+------------
    Incandescent |  2.2%
        Halogen  |  7.0%
            LED  |  8.7%
    Fluorescent  | 10.7%

```
 This call is equivalent to: ```kotlin
Builder.intensity(efficiency * 683 * watts);

```

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| watts | Energy consumed by a lightbulb. It is related to the energy produced and ultimately the brightness by the efficiency parameter. This value is often available on the packaging of commercial lightbulbs. |
| efficiency | Efficiency in percent. This depends on the type of lightbulb used. |
