//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[intensity](intensity.md)

# intensity

[main]\
open fun [intensity](intensity.md)(intensity: Float): [LightManager.Builder](index.md)

Sets the initial intensity of a light.

#### Return

This Builder, for chaining calls.

For example, the sun's illuminance is about 100,000 lux.

This method overrides any prior calls to intensity or intensityCandela.

#### Parameters

main

| | |
|---|---|
| intensity | This parameter depends on the Light.Type:<br>- For directional lights, it specifies the illuminance in *lux*(or *lumen/m^2*). - For point lights and spot lights, it specifies the luminous power in *lumen*. |

[main]\
open fun [intensity](intensity.md)(watts: Float, efficiency: Float): [LightManager.Builder](index.md)

Sets the initial intensity of a light in watts. 

This call is equivalent to `Builder::intensity(efficiency <i> 683 </i> watts);`

#### Return

This Builder, for chaining calls.

This method overrides any prior calls to intensity or intensityCandela.

#### Parameters

main

| | |
|---|---|
| watts | Energy consumed by a lightbulb. It is related to the energy produced and ultimately the brightness by the `efficiency` parameter. This value is often available on the packaging of commercial lightbulbs. |
| efficiency | Efficiency in percent. This depends on the type of lightbulb used.<br>| | | |---|---| |  |  | | Incandescent | 2.2% | | Halogen | 7.0% | | LED | 8.7% | | Fluorescent | 10.7% | |
