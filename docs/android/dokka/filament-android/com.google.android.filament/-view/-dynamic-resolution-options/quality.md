//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[DynamicResolutionOptions](index.md)/[quality](quality.md)

# quality

[main]\
open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)

Upscaling quality 

- LOW: bilinear filtered blit. Fastest, poor quality
- MEDIUM: Qualcomm Snapdragon Game Super Resolution (SGSR) 1.0
- HIGH: AMD FidelityFX FSR1 w/ mobile optimizations
- ULTRA: AMD FidelityFX FSR1

 FSR1 and SGSR require a well anti-aliased (MSAA or TAA), noise free scene. Avoid FXAA and dithering. 

The default upscaling quality is set to LOW.

caveat: currently, `quality` is always set to LOW if the View is TRANSLUCENT.
