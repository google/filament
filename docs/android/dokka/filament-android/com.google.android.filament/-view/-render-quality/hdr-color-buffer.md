//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[RenderQuality](index.md)/[hdrColorBuffer](hdr-color-buffer.md)

# hdrColorBuffer

[main]\
open var [hdrColorBuffer](hdr-color-buffer.md): [View.QualityLevel](../-quality-level/index.md)

Sets the quality of the HDR color buffer. 

A quality of HIGH or ULTRA means using an RGB16F or RGBA16F color buffer. This means colors in the LDR range (0..1) have a 10 bit precision. A quality of LOW or MEDIUM means using an R11G11B10F opaque color buffer or an RGBA16F transparent color buffer. With R11G11B10F colors in the LDR range have a precision of either 6 bits (red and green channels) or 5 bits (blue channel).
