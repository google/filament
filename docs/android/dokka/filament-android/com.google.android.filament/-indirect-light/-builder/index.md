//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use `Builder` to construct an `IndirectLight` object instance.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor()<br>Use `Builder` to construct an `IndirectLight` object instance. |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndirectLight](../index.md)<br>Creates the IndirectLight object and returns a pointer to it. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [IndirectLight.Builder](index.md)<br>Environment intensity (optional). |
| [irradiance](irradiance.md) | [main]<br>open fun [irradiance](irradiance.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)<br>Sets the irradiance as a cubemap.<br>[main]<br>open fun [irradiance](irradiance.md)(bands: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [IndirectLight.Builder](index.md)<br>Sets the irradiance as Spherical Harmonics. |
| [radiance](radiance.md) | [main]<br>open fun [radiance](radiance.md)(bands: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [IndirectLight.Builder](index.md)<br>Sets the irradiance from the radiance expressed as Spherical Harmonics. |
| [reflections](reflections.md) | [main]<br>open fun [reflections](reflections.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)<br>Set the reflections cubemap mipmap chain. |
| [rotation](rotation.md) | [main]<br>open fun [rotation](rotation.md)(rotation: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [IndirectLight.Builder](index.md)<br>Specifies the rigid-body transformation to apply to the IBL. |
