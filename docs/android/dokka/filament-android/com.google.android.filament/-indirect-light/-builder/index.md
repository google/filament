//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndirectLight](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct an IndirectLight object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndirectLight](../index.md)<br>Creates the IndirectLight object and returns a pointer to it. |
| [intensity](intensity.md) | [main]<br>open fun [intensity](intensity.md)(envIntensity: Float): [IndirectLight.Builder](index.md)<br>(optional) Environment intensity. |
| [irradiance](irradiance.md) | [main]<br>open fun [irradiance](irradiance.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)<br>Sets the irradiance as a cubemap.<br>[main]<br>open fun [irradiance](irradiance.md)(bands: Int, sh: Array&lt;Float&gt;): [IndirectLight.Builder](index.md)<br>Sets the irradiance as Spherical Harmonics. |
| [radiance](radiance.md) | [main]<br>open fun [radiance](radiance.md)(bands: Int, sh: Array&lt;Float&gt;): [IndirectLight.Builder](index.md)<br>Sets the irradiance from the radiance expressed as Spherical Harmonics. |
| [reflections](reflections.md) | [main]<br>open fun [reflections](reflections.md)(cubemap: [Texture](../../-texture/index.md)): [IndirectLight.Builder](index.md)<br>Set the reflections cubemap mipmap chain. |
| [rotation](rotation.md) | [main]<br>open fun [rotation](rotation.md)(rotation: Array&lt;Float&gt;): [IndirectLight.Builder](index.md)<br>Specifies the rigid-body transformation to apply to the IBL. |
