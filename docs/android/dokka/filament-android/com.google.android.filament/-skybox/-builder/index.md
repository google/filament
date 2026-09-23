//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct an Skybox object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Skybox](../index.md)<br>Creates the Skybox object and returns a pointer to it. |
| [color](color.md) | [main]<br>open fun [color](color.md)(color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Skybox.Builder](index.md)<br>open fun [color](color.md)(colorx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colory: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorw: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)<br>Sets the skybox to a constant color. |
| [environment](environment.md) | [main]<br>open fun [environment](environment.md)(cubemap: [Texture](../../-texture/index.md)): [Skybox.Builder](index.md)<br>Set the environment map (i.e. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)<br>Skybox intensity when no IndirectLight is set on the Scene. |
| [priority](priority.md) | [main]<br>open fun [priority](priority.md)(priority: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Skybox.Builder](index.md)<br>Set the rendering priority of the Skybox. |
| [showSun](show-sun.md) | [main]<br>open fun [showSun](show-sun.md)(show: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Skybox.Builder](index.md)<br>Indicates whether the sun should be rendered. |
