//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SwapChain](index.md)/[CONFIG_APPLE_CVPIXELBUFFER](-c-o-n-f-i-g_-a-p-p-l-e_-c-v-p-i-x-e-l-b-u-f-f-e-r.md)

# CONFIG_APPLE_CVPIXELBUFFER

[main]\
val [CONFIG_APPLE_CVPIXELBUFFER](-c-o-n-f-i-g_-a-p-p-l-e_-c-v-p-i-x-e-l-b-u-f-f-e-r.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = 8

Indicates that the native window is a CVPixelBufferRef. 

This is only supported by the Metal backend. The CVPixelBuffer must be in the kCVPixelFormatType_32BGRA format.

It is not necessary to add an additional retain call before passing the pixel buffer to Filament. Filament will call CVPixelBufferRetain during Engine::createSwapChain, and CVPixelBufferRelease when the swap chain is destroyed.
