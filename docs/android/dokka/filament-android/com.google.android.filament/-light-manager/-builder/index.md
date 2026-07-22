//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct a Light object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor(type: [LightManager.Type](../-type/index.md))<br>Creates a light builder and set the light's [Type](../-type/index.md). |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Adds the Light component to an entity. |
| [castLight](cast-light.md) | [main]<br>open fun [castLight](cast-light.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [LightManager.Builder](index.md)<br>Whether this light casts light (enabled by default)  In some situations it can be useful to have a light in the scene that doesn't actually emit light, but does cast shadows. |
| [castShadows](cast-shadows.md) | [main]<br>open fun [castShadows](cast-shadows.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [LightManager.Builder](index.md)<br>Whether this Light casts shadows (disabled by default) |
| [color](color.md) | [main]<br>open fun [color](color.md)(linearR: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), linearG: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), linearB: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial color of a light. |
| [direction](direction.md) | [main]<br>open fun [direction](direction.md)(x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial direction of a light in world space. |
| [falloff](falloff.md) | [main]<br>open fun [falloff](falloff.md)(radius: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Set the falloff distance for point lights and spot lights. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial intensity of a light.<br>[main]<br>open fun [intensity](intensity.md)(watts: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), efficiency: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial intensity of a light in watts. |
| [intensityCandela](intensity-candela.md) | [main]<br>open fun [intensityCandela](intensity-candela.md)(intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial intensity of a spot or point light in candela. |
| [lightChannel](light-channel.md) | [main]<br>open fun [lightChannel](light-channel.md)(channel: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [LightManager.Builder](index.md)<br>Enables or disables a light channel. |
| [position](position.md) | [main]<br>open fun [position](position.md)(x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Sets the initial position of the light in world space. |
| [shadowOptions](shadow-options.md) | [main]<br>open fun [shadowOptions](shadow-options.md)(options: [LightManager.ShadowOptions](../-shadow-options/index.md)): [LightManager.Builder](index.md)<br>Sets the shadow map options for this light. |
| [spotLightCone](spot-light-cone.md) | [main]<br>open fun [spotLightCone](spot-light-cone.md)(inner: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), outer: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Defines a spot light's angular falloff attenuation. |
| [sunAngularRadius](sun-angular-radius.md) | [main]<br>open fun [sunAngularRadius](sun-angular-radius.md)(angularRadius: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Defines the angular radius of the sun, in degrees, between 0.25° and 20.0° The Sun as seen from Earth has an angular size of 0.526° to 0. |
| [sunHaloFalloff](sun-halo-falloff.md) | [main]<br>open fun [sunHaloFalloff](sun-halo-falloff.md)(haloFalloff: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Defines the halo falloff of the sun. |
| [sunHaloSize](sun-halo-size.md) | [main]<br>open fun [sunHaloSize](sun-halo-size.md)(haloSize: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)<br>Defines the halo radius of the sun. |
