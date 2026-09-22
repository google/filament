//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct a Texture object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [Texture](../index.md)<br>Creates the Texture object and returns a pointer to it. |
| [depth](depth.md) | [main]<br>open fun [depth](depth.md)(depth: Int): [Texture.Builder](index.md)<br>Specifies the depth in texels of the texture. |
| [external](external.md) | [main]<br>open fun [external](external.md)(): [Texture.Builder](index.md)<br>Creates an external texture. |
| [format](format.md) | [main]<br>open fun [format](format.md)(format: [Texture.InternalFormat](../-internal-format/index.md)): [Texture.Builder](index.md)<br>Specifies the *internal* format of this texture. |
| [height](height.md) | [main]<br>open fun [height](height.md)(height: Int): [Texture.Builder](index.md)<br>Specifies the height in texels of the texture. |
| [importTexture](import-texture.md) | [main]<br>open fun [importTexture](import-texture.md)(id: Long): [Texture.Builder](index.md)<br>Specify a native texture to import as a Filament texture. |
| [levels](levels.md) | [main]<br>open fun [levels](levels.md)(levels: Int): [Texture.Builder](index.md)<br>Specifies the numbers of mip map levels. |
| [name](name.md) | [main]<br>open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Texture.Builder](index.md)<br>Associate an optional name with this Texture for debugging purposes. |
| [sampler](sampler.md) | [main]<br>open fun [sampler](sampler.md)(target: [Texture.Sampler](../-sampler/index.md)): [Texture.Builder](index.md)<br>Specifies the type of sampler to use. |
| [samples](samples.md) | [main]<br>open fun [samples](samples.md)(samples: Int): [Texture.Builder](index.md)<br>Specifies the numbers of samples used for MSAA (Multisample Anti-Aliasing). |
| [swizzle](swizzle.md) | [main]<br>open fun [swizzle](swizzle.md)(r: [Texture.Swizzle](../-swizzle/index.md), g: [Texture.Swizzle](../-swizzle/index.md), b: [Texture.Swizzle](../-swizzle/index.md), a: [Texture.Swizzle](../-swizzle/index.md)): [Texture.Builder](index.md)<br>Specifies how a texture's channels map to color components Texture Swizzle is only supported if isTextureSwizzleSupported() returns true. |
| [usage](usage.md) | [main]<br>open fun [usage](usage.md)(usage: Int): [Texture.Builder](index.md)<br>Specifies if the texture will be used as a render target attachment. |
| [width](width.md) | [main]<br>open fun [width](width.md)(width: Int): [Texture.Builder](index.md)<br>Specifies the width in texels of the texture. |
