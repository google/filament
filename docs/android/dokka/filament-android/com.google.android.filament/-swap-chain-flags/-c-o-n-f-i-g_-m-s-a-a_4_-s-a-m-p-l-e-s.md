//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChainFlags](index.md)/[CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md)

# CONFIG_MSAA_4_SAMPLES

[main]\
val [CONFIG_MSAA_4_SAMPLES](-c-o-n-f-i-g_-m-s-a-a_4_-s-a-m-p-l-e-s.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 128

Indicates that the SwapChain is configured to use Multi-Sample Anti-Aliasing (MSAA) with the given sample points within each pixel. Only supported when isMSAASwapChainSupported(4) is true. This is only supported by EGL(Android). Other GL platforms (GLX, WGL, etc) don't support it because the swapchain MSAA settings must be configured before window creation.
