//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Usage](index.md)

# Usage

[main]\
open class [Usage](index.md)

A bitmask to specify how the texture will be used.

## Constructors

| | |
|---|---|
| [Usage](-usage.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [BLIT_DST](-b-l-i-t_-d-s-t.md) | [main]<br>val [BLIT_DST](-b-l-i-t_-d-s-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 128<br>Texture can be used the destination of a blit() |
| [BLIT_SRC](-b-l-i-t_-s-r-c.md) | [main]<br>val [BLIT_SRC](-b-l-i-t_-s-r-c.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 64<br>Texture can be used the source of a blit() |
| [COLOR_ATTACHMENT](-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [COLOR_ATTACHMENT](-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 1<br>The texture will be used as a color attachment |
| [DEFAULT](-d-e-f-a-u-l-t.md) | [main]<br>val [DEFAULT](-d-e-f-a-u-l-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 24<br>by default textures are `UPLOADABLE` and `SAMPLEABLE` |
| [DEPTH_ATTACHMENT](-d-e-p-t-h_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [DEPTH_ATTACHMENT](-d-e-p-t-h_-a-t-t-a-c-h-m-e-n-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 2<br>The texture will be used as a depth attachment |
| [GEN_MIPMAPPABLE](-g-e-n_-m-i-p-m-a-p-p-a-b-l-e.md) | [main]<br>val [GEN_MIPMAPPABLE](-g-e-n_-m-i-p-m-a-p-p-a-b-l-e.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 512<br>Texture can be used with generateMipmaps() |
| [PROTECTED](-p-r-o-t-e-c-t-e-d.md) | [main]<br>val [PROTECTED](-p-r-o-t-e-c-t-e-d.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 256<br>Texture can be used for protected content |
| [SAMPLEABLE](-s-a-m-p-l-e-a-b-l-e.md) | [main]<br>val [SAMPLEABLE](-s-a-m-p-l-e-a-b-l-e.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 16<br>The texture can be read from a shader or blitted from |
| [STENCIL_ATTACHMENT](-s-t-e-n-c-i-l_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [STENCIL_ATTACHMENT](-s-t-e-n-c-i-l_-a-t-t-a-c-h-m-e-n-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 4<br>The texture will be used as a stencil attachment |
| [SUBPASS_INPUT](-s-u-b-p-a-s-s_-i-n-p-u-t.md) | [main]<br>val [SUBPASS_INPUT](-s-u-b-p-a-s-s_-i-n-p-u-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 32<br>Texture can be used as a subpass input |
| [UPLOADABLE](-u-p-l-o-a-d-a-b-l-e.md) | [main]<br>val [UPLOADABLE](-u-p-l-o-a-d-a-b-l-e.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 8<br>The texture content can be set with setImage |
