//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)

# FogOptions

[main]\
open class [FogOptions](index.md)

Options to control large-scale fog in the scene. 

Materials can enable the `linearFog` property, which uses a simplified, linear equation for fog calculation; in this mode, the heightFalloff is ignored as well as the mipmap selection in IBL or skyColor mode.

## Constructors

| | |
|---|---|
| [FogOptions](-fog-options.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [color](color.md) | [main]<br>open var [color](color.md): Array&lt;Float&gt;<br>Fog's color is used for ambient light in-scattering, a good value is to use the average of the ambient light, possibly tinted towards blue for outdoors environments. |
| [cutOffDistance](cut-off-distance.md) | [main]<br>open var [cutOffDistance](cut-off-distance.md): Float<br>Distance in world units [m] after which the fog calculation is disabled. |
| [density](density.md) | [main]<br>open var [density](density.md): Float<br>Extinction factor in [1/m] at an altitude 'height'. |
| [distance](distance.md) | [main]<br>open var [distance](distance.md): Float<br>Distance in world units [m] from the camera to where the fog starts ( >= 0. |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): Boolean<br>Enable or disable large-scale fog |
| [fogColorFromIbl](fog-color-from-ibl.md) | [main]<br>open var [fogColorFromIbl](fog-color-from-ibl.md): Boolean<br>The fog color will be sampled from the IBL in the view direction and tinted by `color`. |
| [height](height.md) | [main]<br>open var [height](height.md): Float<br>Fog's floor in world units [m]. |
| [heightFalloff](height-falloff.md) | [main]<br>open var [heightFalloff](height-falloff.md): Float<br>How fast the fog dissipates with the altitude. |
| [inScatteringSize](in-scattering-size.md) | [main]<br>open var [inScatteringSize](in-scattering-size.md): Float<br>Very inaccurately simulates the Sun's in-scattering. |
| [inScatteringStart](in-scattering-start.md) | [main]<br>open var [inScatteringStart](in-scattering-start.md): Float<br>Distance in world units [m] from the camera where the Sun in-scattering starts. |
| [maximumOpacity](maximum-opacity.md) | [main]<br>open var [maximumOpacity](maximum-opacity.md): Float<br>fog's maximum opacity between 0 and 1. |
| [skyColor](sky-color.md) | [main]<br>open var [skyColor](sky-color.md): [Texture](../../-texture/index.md)<br>skyTexture must be a mipmapped cubemap. |
