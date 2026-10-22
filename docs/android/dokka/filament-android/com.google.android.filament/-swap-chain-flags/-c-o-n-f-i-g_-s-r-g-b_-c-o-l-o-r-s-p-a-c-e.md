//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChainFlags](index.md)/[CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md)

# CONFIG_SRGB_COLORSPACE

[main]\
val [CONFIG_SRGB_COLORSPACE](-c-o-n-f-i-g_-s-r-g-b_-c-o-l-o-r-s-p-a-c-e.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 16

Indicates that the SwapChain must automatically perform linear to sRGB encoding. This flag is ignored if isSRGBSwapChainSupported() is false. When using this flag, post-processing should be disabled.

#### See also

| |
|---|
| [SwapChain](../-swap-chain/is-s-r-g-b-swap-chain-supported.md) |
| [View](../-view/set-post-processing-enabled.md) |
