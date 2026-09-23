//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChainFlags](index.md)/[CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md)

# CONFIG_HAS_STENCIL_BUFFER

[main]\
val [CONFIG_HAS_STENCIL_BUFFER](-c-o-n-f-i-g_-h-a-s_-s-t-e-n-c-i-l_-b-u-f-f-e-r.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 32

Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer. This flag is necessary when using View::setStencilBufferEnabled and rendering directly into the SwapChain (when post-processing is disabled). The specific format of the stencil buffer depends on platform support. The following pixel formats are tried, in order of preference: Depth only (without CONFIG_HAS_STENCIL_BUFFER): - DEPTH32F - DEPTH24 Depth + stencil (with CONFIG_HAS_STENCIL_BUFFER): - DEPTH32F_STENCIL8 - DEPTH24F_STENCIL8 Note that enabling the stencil buffer may hinder depth precision and should only be used if necessary.

#### See also

| |
|---|
| [View](../-view/set-post-processing-enabled.md) |
