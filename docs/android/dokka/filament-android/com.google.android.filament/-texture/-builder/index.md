//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use `Builder` to construct a `Texture` object instance.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor()<br>Use `Builder` to construct a `Texture` object instance. |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Texture](../index.md)<br>Creates a new `Texture` instance. |
| [depth](depth.md) | [main]<br>open fun [depth](depth.md)(depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Specifies the texture's number of layers. |
| [external](external.md) | [main]<br>open fun [external](external.md)(): [Texture.Builder](index.md)<br>Creates an external texture. |
| [format](format.md) | [main]<br>open fun [format](format.md)(format: [Texture.InternalFormat](../-internal-format/index.md)): [Texture.Builder](index.md)<br>Specifies the texture's internal format. |
| [height](height.md) | [main]<br>open fun [height](height.md)(height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Specifies the height of the texture in texels. |
| [importTexture](import-texture.md) | [main]<br>open fun [importTexture](import-texture.md)(id: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Texture.Builder](index.md)<br>Specify a native texture to import as a Filament texture. |
| [levels](levels.md) | [main]<br>open fun [levels](levels.md)(levels: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Specifies the number of mipmap levels |
| [sampler](sampler.md) | [main]<br>open fun [sampler](sampler.md)(target: [Texture.Sampler](../-sampler/index.md)): [Texture.Builder](index.md)<br>Specifies the type of sampler to use. |
| [samples](samples.md) | [main]<br>open fun [samples](samples.md)(samples: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Specifies the number of samples for multisample anti-aliasing. |
| [swizzle](swizzle.md) | [main]<br>open fun [swizzle](swizzle.md)(r: [Texture.Swizzle](../-swizzle/index.md), g: [Texture.Swizzle](../-swizzle/index.md), b: [Texture.Swizzle](../-swizzle/index.md), a: [Texture.Swizzle](../-swizzle/index.md)): [Texture.Builder](index.md)<br>Specifies how a texture's channels map to color components |
| [usage](usage.md) | [main]<br>open fun [usage](usage.md)(flags: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Sets the usage flags, which is necessary when attaching to [RenderTarget](../../-render-target/index.md). |
| [width](width.md) | [main]<br>open fun [width](width.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Texture.Builder](index.md)<br>Specifies the width of the texture in texels. |
