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
| [color](color.md) | [main]<br>open fun [color](color.md)(color: Array&lt;Float&gt;): [Skybox.Builder](index.md)<br>open fun [color](color.md)(colorx: Float, colory: Float, colorz: Float, colorw: Float): [Skybox.Builder](index.md)<br>Sets the skybox to a constant color. |
| [environment](environment.md) | [main]<br>open fun [environment](environment.md)(cubemap: [Texture](../../-texture/index.md)): [Skybox.Builder](index.md)<br>Set the environment map (i.e. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(envIntensity: Float): [Skybox.Builder](index.md)<br>Skybox intensity when no IndirectLight is set on the Scene. |
| [priority](priority.md) | [main]<br>open fun [priority](priority.md)(priority: Int): [Skybox.Builder](index.md)<br>Set the rendering priority of the Skybox. |
| [showSun](show-sun.md) | [main]<br>open fun [showSun](show-sun.md)(show: Boolean): [Skybox.Builder](index.md)<br>Indicates whether the sun should be rendered. |
