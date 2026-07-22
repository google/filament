//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[InternalFormat](index.md)

# InternalFormat

[main]\
enum [InternalFormat](index.md)

Internal texel formats 

These formats are used to specify a texture's internal storage format.

# Enumerants syntax format

`[components][size][type]``components` : List of stored components by this format `size` : Size in bit of each component `type` : Type this format is stored as 

# Special color formats

 There are a few special color formats that don't follow the convention above: 

# Compressed texture formats

 Many compressed texture formats are supported as well, which include (but are not limited to) the following list:

## Entries

| | |
|---|---|
| [R8](-r8/index.md) | [main]<br>[R8](-r8/index.md) |
| [R8_SNORM](-r8_-s-n-o-r-m/index.md) | [main]<br>[R8_SNORM](-r8_-s-n-o-r-m/index.md) |
| [R8UI](-r8-u-i/index.md) | [main]<br>[R8UI](-r8-u-i/index.md) |
| [R8I](-r8-i/index.md) | [main]<br>[R8I](-r8-i/index.md) |
| [STENCIL8](-s-t-e-n-c-i-l8/index.md) | [main]<br>[STENCIL8](-s-t-e-n-c-i-l8/index.md) |
| [R16F](-r16-f/index.md) | [main]<br>[R16F](-r16-f/index.md) |
| [R16UI](-r16-u-i/index.md) | [main]<br>[R16UI](-r16-u-i/index.md) |
| [R16I](-r16-i/index.md) | [main]<br>[R16I](-r16-i/index.md) |
| [RG8](-r-g8/index.md) | [main]<br>[RG8](-r-g8/index.md) |
| [RG8_SNORM](-r-g8_-s-n-o-r-m/index.md) | [main]<br>[RG8_SNORM](-r-g8_-s-n-o-r-m/index.md) |
| [RG8UI](-r-g8-u-i/index.md) | [main]<br>[RG8UI](-r-g8-u-i/index.md) |
| [RG8I](-r-g8-i/index.md) | [main]<br>[RG8I](-r-g8-i/index.md) |
| [RGB565](-r-g-b565/index.md) | [main]<br>[RGB565](-r-g-b565/index.md) |
| [RGB9_E5](-r-g-b9_-e5/index.md) | [main]<br>[RGB9_E5](-r-g-b9_-e5/index.md) |
| [RGB5_A1](-r-g-b5_-a1/index.md) | [main]<br>[RGB5_A1](-r-g-b5_-a1/index.md) |
| [RGBA4](-r-g-b-a4/index.md) | [main]<br>[RGBA4](-r-g-b-a4/index.md) |
| [DEPTH16](-d-e-p-t-h16/index.md) | [main]<br>[DEPTH16](-d-e-p-t-h16/index.md) |
| [RGB8](-r-g-b8/index.md) | [main]<br>[RGB8](-r-g-b8/index.md) |
| [SRGB8](-s-r-g-b8/index.md) | [main]<br>[SRGB8](-s-r-g-b8/index.md) |
| [RGB8_SNORM](-r-g-b8_-s-n-o-r-m/index.md) | [main]<br>[RGB8_SNORM](-r-g-b8_-s-n-o-r-m/index.md) |
| [RGB8UI](-r-g-b8-u-i/index.md) | [main]<br>[RGB8UI](-r-g-b8-u-i/index.md) |
| [RGB8I](-r-g-b8-i/index.md) | [main]<br>[RGB8I](-r-g-b8-i/index.md) |
| [DEPTH24](-d-e-p-t-h24/index.md) | [main]<br>[DEPTH24](-d-e-p-t-h24/index.md) |
| [R32F](-r32-f/index.md) | [main]<br>[R32F](-r32-f/index.md) |
| [R32UI](-r32-u-i/index.md) | [main]<br>[R32UI](-r32-u-i/index.md) |
| [R32I](-r32-i/index.md) | [main]<br>[R32I](-r32-i/index.md) |
| [RG16F](-r-g16-f/index.md) | [main]<br>[RG16F](-r-g16-f/index.md) |
| [RG16UI](-r-g16-u-i/index.md) | [main]<br>[RG16UI](-r-g16-u-i/index.md) |
| [RG16I](-r-g16-i/index.md) | [main]<br>[RG16I](-r-g16-i/index.md) |
| [R11F_G11F_B10F](-r11-f_-g11-f_-b10-f/index.md) | [main]<br>[R11F_G11F_B10F](-r11-f_-g11-f_-b10-f/index.md) |
| [RGBA8](-r-g-b-a8/index.md) | [main]<br>[RGBA8](-r-g-b-a8/index.md) |
| [SRGB8_A8](-s-r-g-b8_-a8/index.md) | [main]<br>[SRGB8_A8](-s-r-g-b8_-a8/index.md) |
| [RGBA8_SNORM](-r-g-b-a8_-s-n-o-r-m/index.md) | [main]<br>[RGBA8_SNORM](-r-g-b-a8_-s-n-o-r-m/index.md) |
| [UNUSED](-u-n-u-s-e-d/index.md) | [main]<br>[UNUSED](-u-n-u-s-e-d/index.md) |
| [RGB10_A2](-r-g-b10_-a2/index.md) | [main]<br>[RGB10_A2](-r-g-b10_-a2/index.md) |
| [RGBA8UI](-r-g-b-a8-u-i/index.md) | [main]<br>[RGBA8UI](-r-g-b-a8-u-i/index.md) |
| [RGBA8I](-r-g-b-a8-i/index.md) | [main]<br>[RGBA8I](-r-g-b-a8-i/index.md) |
| [DEPTH32F](-d-e-p-t-h32-f/index.md) | [main]<br>[DEPTH32F](-d-e-p-t-h32-f/index.md) |
| [DEPTH24_STENCIL8](-d-e-p-t-h24_-s-t-e-n-c-i-l8/index.md) | [main]<br>[DEPTH24_STENCIL8](-d-e-p-t-h24_-s-t-e-n-c-i-l8/index.md) |
| [DEPTH32F_STENCIL8](-d-e-p-t-h32-f_-s-t-e-n-c-i-l8/index.md) | [main]<br>[DEPTH32F_STENCIL8](-d-e-p-t-h32-f_-s-t-e-n-c-i-l8/index.md) |
| [RGB16F](-r-g-b16-f/index.md) | [main]<br>[RGB16F](-r-g-b16-f/index.md) |
| [RGB16UI](-r-g-b16-u-i/index.md) | [main]<br>[RGB16UI](-r-g-b16-u-i/index.md) |
| [RGB16I](-r-g-b16-i/index.md) | [main]<br>[RGB16I](-r-g-b16-i/index.md) |
| [RG32F](-r-g32-f/index.md) | [main]<br>[RG32F](-r-g32-f/index.md) |
| [RG32UI](-r-g32-u-i/index.md) | [main]<br>[RG32UI](-r-g32-u-i/index.md) |
| [RG32I](-r-g32-i/index.md) | [main]<br>[RG32I](-r-g32-i/index.md) |
| [RGBA16F](-r-g-b-a16-f/index.md) | [main]<br>[RGBA16F](-r-g-b-a16-f/index.md) |
| [RGBA16UI](-r-g-b-a16-u-i/index.md) | [main]<br>[RGBA16UI](-r-g-b-a16-u-i/index.md) |
| [RGBA16I](-r-g-b-a16-i/index.md) | [main]<br>[RGBA16I](-r-g-b-a16-i/index.md) |
| [RGB32F](-r-g-b32-f/index.md) | [main]<br>[RGB32F](-r-g-b32-f/index.md) |
| [RGB32UI](-r-g-b32-u-i/index.md) | [main]<br>[RGB32UI](-r-g-b32-u-i/index.md) |
| [RGB32I](-r-g-b32-i/index.md) | [main]<br>[RGB32I](-r-g-b32-i/index.md) |
| [RGBA32F](-r-g-b-a32-f/index.md) | [main]<br>[RGBA32F](-r-g-b-a32-f/index.md) |
| [RGBA32UI](-r-g-b-a32-u-i/index.md) | [main]<br>[RGBA32UI](-r-g-b-a32-u-i/index.md) |
| [RGBA32I](-r-g-b-a32-i/index.md) | [main]<br>[RGBA32I](-r-g-b-a32-i/index.md) |
| [EAC_R11](-e-a-c_-r11/index.md) | [main]<br>[EAC_R11](-e-a-c_-r11/index.md) |
| [EAC_R11_SIGNED](-e-a-c_-r11_-s-i-g-n-e-d/index.md) | [main]<br>[EAC_R11_SIGNED](-e-a-c_-r11_-s-i-g-n-e-d/index.md) |
| [EAC_RG11](-e-a-c_-r-g11/index.md) | [main]<br>[EAC_RG11](-e-a-c_-r-g11/index.md) |
| [EAC_RG11_SIGNED](-e-a-c_-r-g11_-s-i-g-n-e-d/index.md) | [main]<br>[EAC_RG11_SIGNED](-e-a-c_-r-g11_-s-i-g-n-e-d/index.md) |
| [ETC2_RGB8](-e-t-c2_-r-g-b8/index.md) | [main]<br>[ETC2_RGB8](-e-t-c2_-r-g-b8/index.md) |
| [ETC2_SRGB8](-e-t-c2_-s-r-g-b8/index.md) | [main]<br>[ETC2_SRGB8](-e-t-c2_-s-r-g-b8/index.md) |
| [ETC2_RGB8_A1](-e-t-c2_-r-g-b8_-a1/index.md) | [main]<br>[ETC2_RGB8_A1](-e-t-c2_-r-g-b8_-a1/index.md) |
| [ETC2_SRGB8_A1](-e-t-c2_-s-r-g-b8_-a1/index.md) | [main]<br>[ETC2_SRGB8_A1](-e-t-c2_-s-r-g-b8_-a1/index.md) |
| [ETC2_EAC_RGBA8](-e-t-c2_-e-a-c_-r-g-b-a8/index.md) | [main]<br>[ETC2_EAC_RGBA8](-e-t-c2_-e-a-c_-r-g-b-a8/index.md) |
| [ETC2_EAC_SRGBA8](-e-t-c2_-e-a-c_-s-r-g-b-a8/index.md) | [main]<br>[ETC2_EAC_SRGBA8](-e-t-c2_-e-a-c_-s-r-g-b-a8/index.md) |
| [DXT1_RGB](-d-x-t1_-r-g-b/index.md) | [main]<br>[DXT1_RGB](-d-x-t1_-r-g-b/index.md) |
| [DXT1_RGBA](-d-x-t1_-r-g-b-a/index.md) | [main]<br>[DXT1_RGBA](-d-x-t1_-r-g-b-a/index.md) |
| [DXT3_RGBA](-d-x-t3_-r-g-b-a/index.md) | [main]<br>[DXT3_RGBA](-d-x-t3_-r-g-b-a/index.md) |
| [DXT5_RGBA](-d-x-t5_-r-g-b-a/index.md) | [main]<br>[DXT5_RGBA](-d-x-t5_-r-g-b-a/index.md) |
| [DXT1_SRGB](-d-x-t1_-s-r-g-b/index.md) | [main]<br>[DXT1_SRGB](-d-x-t1_-s-r-g-b/index.md) |
| [DXT1_SRGBA](-d-x-t1_-s-r-g-b-a/index.md) | [main]<br>[DXT1_SRGBA](-d-x-t1_-s-r-g-b-a/index.md) |
| [DXT3_SRGBA](-d-x-t3_-s-r-g-b-a/index.md) | [main]<br>[DXT3_SRGBA](-d-x-t3_-s-r-g-b-a/index.md) |
| [DXT5_SRGBA](-d-x-t5_-s-r-g-b-a/index.md) | [main]<br>[DXT5_SRGBA](-d-x-t5_-s-r-g-b-a/index.md) |
| [RGBA_ASTC_4x4](-r-g-b-a_-a-s-t-c_4x4/index.md) | [main]<br>[RGBA_ASTC_4x4](-r-g-b-a_-a-s-t-c_4x4/index.md) |
| [RGBA_ASTC_5x4](-r-g-b-a_-a-s-t-c_5x4/index.md) | [main]<br>[RGBA_ASTC_5x4](-r-g-b-a_-a-s-t-c_5x4/index.md) |
| [RGBA_ASTC_5x5](-r-g-b-a_-a-s-t-c_5x5/index.md) | [main]<br>[RGBA_ASTC_5x5](-r-g-b-a_-a-s-t-c_5x5/index.md) |
| [RGBA_ASTC_6x5](-r-g-b-a_-a-s-t-c_6x5/index.md) | [main]<br>[RGBA_ASTC_6x5](-r-g-b-a_-a-s-t-c_6x5/index.md) |
| [RGBA_ASTC_6x6](-r-g-b-a_-a-s-t-c_6x6/index.md) | [main]<br>[RGBA_ASTC_6x6](-r-g-b-a_-a-s-t-c_6x6/index.md) |
| [RGBA_ASTC_8x5](-r-g-b-a_-a-s-t-c_8x5/index.md) | [main]<br>[RGBA_ASTC_8x5](-r-g-b-a_-a-s-t-c_8x5/index.md) |
| [RGBA_ASTC_8x6](-r-g-b-a_-a-s-t-c_8x6/index.md) | [main]<br>[RGBA_ASTC_8x6](-r-g-b-a_-a-s-t-c_8x6/index.md) |
| [RGBA_ASTC_8x8](-r-g-b-a_-a-s-t-c_8x8/index.md) | [main]<br>[RGBA_ASTC_8x8](-r-g-b-a_-a-s-t-c_8x8/index.md) |
| [RGBA_ASTC_10x5](-r-g-b-a_-a-s-t-c_10x5/index.md) | [main]<br>[RGBA_ASTC_10x5](-r-g-b-a_-a-s-t-c_10x5/index.md) |
| [RGBA_ASTC_10x6](-r-g-b-a_-a-s-t-c_10x6/index.md) | [main]<br>[RGBA_ASTC_10x6](-r-g-b-a_-a-s-t-c_10x6/index.md) |
| [RGBA_ASTC_10x8](-r-g-b-a_-a-s-t-c_10x8/index.md) | [main]<br>[RGBA_ASTC_10x8](-r-g-b-a_-a-s-t-c_10x8/index.md) |
| [RGBA_ASTC_10x10](-r-g-b-a_-a-s-t-c_10x10/index.md) | [main]<br>[RGBA_ASTC_10x10](-r-g-b-a_-a-s-t-c_10x10/index.md) |
| [RGBA_ASTC_12x10](-r-g-b-a_-a-s-t-c_12x10/index.md) | [main]<br>[RGBA_ASTC_12x10](-r-g-b-a_-a-s-t-c_12x10/index.md) |
| [RGBA_ASTC_12x12](-r-g-b-a_-a-s-t-c_12x12/index.md) | [main]<br>[RGBA_ASTC_12x12](-r-g-b-a_-a-s-t-c_12x12/index.md) |
| [SRGB8_ALPHA8_ASTC_4x4](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_4x4/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_4x4](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_4x4/index.md) |
| [SRGB8_ALPHA8_ASTC_5x4](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_5x4/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_5x4](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_5x4/index.md) |
| [SRGB8_ALPHA8_ASTC_5x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_5x5/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_5x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_5x5/index.md) |
| [SRGB8_ALPHA8_ASTC_6x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_6x5/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_6x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_6x5/index.md) |
| [SRGB8_ALPHA8_ASTC_6x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_6x6/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_6x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_6x6/index.md) |
| [SRGB8_ALPHA8_ASTC_8x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x5/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_8x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x5/index.md) |
| [SRGB8_ALPHA8_ASTC_8x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x6/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_8x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x6/index.md) |
| [SRGB8_ALPHA8_ASTC_8x8](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x8/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_8x8](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_8x8/index.md) |
| [SRGB8_ALPHA8_ASTC_10x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x5/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_10x5](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x5/index.md) |
| [SRGB8_ALPHA8_ASTC_10x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x6/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_10x6](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x6/index.md) |
| [SRGB8_ALPHA8_ASTC_10x8](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x8/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_10x8](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x8/index.md) |
| [SRGB8_ALPHA8_ASTC_10x10](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x10/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_10x10](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_10x10/index.md) |
| [SRGB8_ALPHA8_ASTC_12x10](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_12x10/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_12x10](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_12x10/index.md) |
| [SRGB8_ALPHA8_ASTC_12x12](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_12x12/index.md) | [main]<br>[SRGB8_ALPHA8_ASTC_12x12](-s-r-g-b8_-a-l-p-h-a8_-a-s-t-c_12x12/index.md) |
| [RED_RGTC1](-r-e-d_-r-g-t-c1/index.md) | [main]<br>[RED_RGTC1](-r-e-d_-r-g-t-c1/index.md) |
| [SIGNED_RED_RGTC1](-s-i-g-n-e-d_-r-e-d_-r-g-t-c1/index.md) | [main]<br>[SIGNED_RED_RGTC1](-s-i-g-n-e-d_-r-e-d_-r-g-t-c1/index.md) |
| [RED_GREEN_RGTC2](-r-e-d_-g-r-e-e-n_-r-g-t-c2/index.md) | [main]<br>[RED_GREEN_RGTC2](-r-e-d_-g-r-e-e-n_-r-g-t-c2/index.md) |
| [SIGNED_RED_GREEN_RGTC2](-s-i-g-n-e-d_-r-e-d_-g-r-e-e-n_-r-g-t-c2/index.md) | [main]<br>[SIGNED_RED_GREEN_RGTC2](-s-i-g-n-e-d_-r-e-d_-g-r-e-e-n_-r-g-t-c2/index.md) |
| [RGB_BPTC_SIGNED_FLOAT](-r-g-b_-b-p-t-c_-s-i-g-n-e-d_-f-l-o-a-t/index.md) | [main]<br>[RGB_BPTC_SIGNED_FLOAT](-r-g-b_-b-p-t-c_-s-i-g-n-e-d_-f-l-o-a-t/index.md) |
| [RGB_BPTC_UNSIGNED_FLOAT](-r-g-b_-b-p-t-c_-u-n-s-i-g-n-e-d_-f-l-o-a-t/index.md) | [main]<br>[RGB_BPTC_UNSIGNED_FLOAT](-r-g-b_-b-p-t-c_-u-n-s-i-g-n-e-d_-f-l-o-a-t/index.md) |
| [RGBA_BPTC_UNORM](-r-g-b-a_-b-p-t-c_-u-n-o-r-m/index.md) | [main]<br>[RGBA_BPTC_UNORM](-r-g-b-a_-b-p-t-c_-u-n-o-r-m/index.md) |
| [SRGB_ALPHA_BPTC_UNORM](-s-r-g-b_-a-l-p-h-a_-b-p-t-c_-u-n-o-r-m/index.md) | [main]<br>[SRGB_ALPHA_BPTC_UNORM](-s-r-g-b_-a-l-p-h-a_-b-p-t-c_-u-n-o-r-m/index.md) |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Texture.InternalFormat](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Texture.InternalFormat](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
