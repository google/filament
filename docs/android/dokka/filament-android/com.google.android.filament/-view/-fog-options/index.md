//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)

# FogOptions

[main]\
open class [FogOptions](index.md)

Options to control large-scale fog in the scene. Materials can enable the `linearFog` property, which uses a simplified, linear equation for fog calculation; in this mode, the heightFalloff is ignored as well as the mipmap selection in IBL or skyColor mode.

## Constructors

| | |
|---|---|
| [FogOptions](-fog-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [color](color.md) | [main]<br>open var [color](color.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Fog's color is used for ambient light in-scattering, a good value is to use the average of the ambient light, possibly tinted towards blue for outdoors environments. |
| [cutOffDistance](cut-off-distance.md) | [main]<br>open var [cutOffDistance](cut-off-distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Distance in world units [m] after which the fog calculation is disabled. |
| [density](density.md) | [main]<br>open var [density](density.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Extinction factor in [1/m] at an altitude 'height'. |
| [distance](distance.md) | [main]<br>open var [distance](distance.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Distance in world units [m] from the camera to where the fog starts ( >= 0. |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Enable or disable large-scale fog |
| [fogColorFromIbl](fog-color-from-ibl.md) | [main]<br>open var [fogColorFromIbl](fog-color-from-ibl.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>The fog color will be sampled from the IBL in the view direction and tinted by `color`. |
| [height](height.md) | [main]<br>open var [height](height.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Fog's floor in world units [m]. |
| [heightFalloff](height-falloff.md) | [main]<br>open var [heightFalloff](height-falloff.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>How fast the fog dissipates with the altitude. |
| [inScatteringSize](in-scattering-size.md) | [main]<br>open var [inScatteringSize](in-scattering-size.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Very inaccurately simulates the Sun's in-scattering. |
| [inScatteringStart](in-scattering-start.md) | [main]<br>open var [inScatteringStart](in-scattering-start.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Distance in world units [m] from the camera where the Sun in-scattering starts. |
| [maximumOpacity](maximum-opacity.md) | [main]<br>open var [maximumOpacity](maximum-opacity.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>fog's maximum opacity between 0 and 1. |
| [skyColor](sky-color.md) | [main]<br>open var [skyColor](sky-color.md): [Texture](../../-texture/index.md)<br>skyTexture must be a mipmapped cubemap. |
