//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use `Builder` to construct a `Skybox` object instance.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor()<br>Use `Builder` to construct a `Skybox` object instance. |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Skybox](../index.md)<br>Creates a `Skybox` object |
| [color](color.md) | [main]<br>open fun [color](color.md)(color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Skybox.Builder](index.md)<br>open fun [color](color.md)(r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), a: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)<br>Sets the `Skybox` to a constant color. |
| [environment](environment.md) | [main]<br>open fun [environment](environment.md)(cubemap: [Texture](../../-texture/index.md)): [Skybox.Builder](index.md)<br>Set the environment map (i.e. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)<br>Sets the `Skybox` intensity when no [IndirectLight](../../-indirect-light/index.md) is set on the [Scene](../../-scene/index.md). |
| [priority](priority.md) | [main]<br>open fun [priority](priority.md)(priority: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Skybox.Builder](index.md)<br>Set the rendering priority of the Skybox. |
| [showSun](show-sun.md) | [main]<br>open fun [showSun](show-sun.md)(show: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [Skybox.Builder](index.md)<br>Indicates whether the sun should be rendered. |
