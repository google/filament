//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md)

# CONFIG_MSAA_4_SAMPLES

[main]\
val [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md): Long = 128

Indicates that the SwapChain is configured to use Multi-Sample Anti-Aliasing (MSAA) with the given sample points within each pixel. 

Only supported when isMSAASwapChainSupported(4) is true.

This is supported by EGL(Android) and Metal. Other GL platforms (GLX, WGL, etc) don't support it because the swapchain MSAA settings must be configured before window creation.

With Metal, this flag should only be used when rendering a single View into a SwapChain. This flag is not supported when rendering multiple Filament Views into this SwapChain.

#### See also

| |
|---|
| [isMSAASwapChainSupported](is-m-s-a-a-swap-chain-supported.md) |
