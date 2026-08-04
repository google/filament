//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChainFlags](index.md)

# SwapChainFlags

class [SwapChainFlags](index.md)

Flags that a `SwapChain` can be created with to control behavior.

#### See also

| |
|---|
| [Engine](../-engine/create-swap-chain-from-native-surface.md) |

## Constructors

| | |
|---|---|
| [SwapChainFlags](-swap-chain-flags.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [CONFIG_DEFAULT](-c-o-n-f-i-g_-d-e-f-a-u-l-t.md) | [main]<br>val [CONFIG_DEFAULT](-c-o-n-f-i-g_-d-e-f-a-u-l-t.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 0 |
| [CONFIG_ENABLE_XCB](-c-o-n-f-i-g_-e-n-a-b-l-e_-x-c-b.md) | [main]<br>val [CONFIG_ENABLE_XCB](-c-o-n-f-i-g_-e-n-a-b-l-e_-x-c-b.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 4<br>Indicates that the native X11 window is an XCB window rather than an XLIB window. |
| [CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md) | [main]<br>val [CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 32<br>Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer. |
| [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md) | [main]<br>val [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 128<br>Indicates that the SwapChain is configured to use Multi-Sample Anti-Aliasing (MSAA) with the given sample points within each pixel. |
| [CONFIG_PROTECTED_CONTENT](-c-o-n-f-i-g_-p-r-o-t-e-c-t-e-d_-c-o-n-t-e-n-t.md) | [main]<br>val [CONFIG_PROTECTED_CONTENT](-c-o-n-f-i-g_-p-r-o-t-e-c-t-e-d_-c-o-n-t-e-n-t.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 64<br>The SwapChain contains protected content. |
| [CONFIG_READABLE](-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md) | [main]<br>val [CONFIG_READABLE](-c-o-n-f-i-g_-r-e-a-d-a-b-l-e.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 2<br>This flag indicates that the `SwapChain` may be used as a source surface for reading back render results. |
| [CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md) | [main]<br>val [CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 16<br>Indicates that the SwapChain must automatically perform linear to sRGB encoding. |
| [CONFIG_TRANSPARENT](-c-o-n-f-i-g_-t-r-a-n-s-p-a-r-e-n-t.md) | [main]<br>val [CONFIG_TRANSPARENT](-c-o-n-f-i-g_-t-r-a-n-s-p-a-r-e-n-t.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 1<br>This flag indicates that the `SwapChain` must be allocated with an alpha-channel. |
