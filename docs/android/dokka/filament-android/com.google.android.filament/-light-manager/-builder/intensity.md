//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[intensity](intensity.md)

# intensity

[main]\
open fun [intensity](intensity.md)(intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial intensity of a light. This method overrides any prior calls to #intensity or #intensityCandela.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| intensity | This parameter depends on the [Type](../-type/index.md), for directional lights, it specifies the illuminance in *lux* (or *lumen/m^2*). For point lights and spot lights, it specifies the luminous power in *lumen*. For example, the sun's illuminance is about 100,000 lux. |

#### See also

| |
|---|
| setIntensity |

[main]\
open fun [intensity](intensity.md)(watts: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), efficiency: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial intensity of a light in watts. 

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
 This method overrides any prior calls to #intensity or #intensityCandela.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| watts | Energy consumed by a lightbulb. It is related to the energy produced and ultimately the brightness by the efficiency parameter. This value is often available on the packaging of commercial lightbulbs. |
| efficiency | Efficiency in percent. This depends on the type of lightbulb used. |
