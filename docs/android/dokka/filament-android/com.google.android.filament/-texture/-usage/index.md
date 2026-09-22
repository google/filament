//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Usage](index.md)

# Usage

[main]\
open class [Usage](index.md)

Bitmask describing the intended Texture Usage

## Properties

| Name | Summary |
|---|---|
| [ALL_ATTACHMENTS](-a-l-l_-a-t-t-a-c-h-m-e-n-t-s.md) | [main]<br>val [ALL_ATTACHMENTS](-a-l-l_-a-t-t-a-c-h-m-e-n-t-s.md): Int = 39<br>Mask of all attachments |
| [BLIT_DST](-b-l-i-t_-d-s-t.md) | [main]<br>val [BLIT_DST](-b-l-i-t_-d-s-t.md): Int = 128<br>Texture can be used the destination of a blit() |
| [BLIT_SRC](-b-l-i-t_-s-r-c.md) | [main]<br>val [BLIT_SRC](-b-l-i-t_-s-r-c.md): Int = 64<br>Texture can be used the source of a blit() |
| [COLOR_ATTACHMENT](-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [COLOR_ATTACHMENT](-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t.md): Int = 1<br>Texture can be used as a color attachment |
| [DEFAULT](-d-e-f-a-u-l-t.md) | [main]<br>val [DEFAULT](-d-e-f-a-u-l-t.md): Int = 24<br>Default texture usage |
| [DEPTH_ATTACHMENT](-d-e-p-t-h_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [DEPTH_ATTACHMENT](-d-e-p-t-h_-a-t-t-a-c-h-m-e-n-t.md): Int = 2<br>Texture can be used as a depth attachment |
| [GEN_MIPMAPPABLE](-g-e-n_-m-i-p-m-a-p-p-a-b-l-e.md) | [main]<br>val [GEN_MIPMAPPABLE](-g-e-n_-m-i-p-m-a-p-p-a-b-l-e.md): Int = 512<br>Texture can be used with generateMipmaps() |
| [NONE](-n-o-n-e.md) | [main]<br>val [NONE](-n-o-n-e.md): Int = 0 |
| [PROTECTED](-p-r-o-t-e-c-t-e-d.md) | [main]<br>val [PROTECTED](-p-r-o-t-e-c-t-e-d.md): Int = 256<br>Texture can be used for protected content |
| [SAMPLEABLE](-s-a-m-p-l-e-a-b-l-e.md) | [main]<br>val [SAMPLEABLE](-s-a-m-p-l-e-a-b-l-e.md): Int = 16<br>Texture can be sampled (default) |
| [STENCIL_ATTACHMENT](-s-t-e-n-c-i-l_-a-t-t-a-c-h-m-e-n-t.md) | [main]<br>val [STENCIL_ATTACHMENT](-s-t-e-n-c-i-l_-a-t-t-a-c-h-m-e-n-t.md): Int = 4<br>Texture can be used as a stencil attachment |
| [SUBPASS_INPUT](-s-u-b-p-a-s-s_-i-n-p-u-t.md) | [main]<br>val [SUBPASS_INPUT](-s-u-b-p-a-s-s_-i-n-p-u-t.md): Int = 32<br>Texture can be used as a subpass input |
| [UPLOADABLE](-u-p-l-o-a-d-a-b-l-e.md) | [main]<br>val [UPLOADABLE](-u-p-l-o-a-d-a-b-l-e.md): Int = 8<br>Data can be uploaded into this texture (default) |
