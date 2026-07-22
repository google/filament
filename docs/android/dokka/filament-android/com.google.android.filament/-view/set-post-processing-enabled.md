//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setPostProcessingEnabled](set-post-processing-enabled.md)

# setPostProcessingEnabled

[main]\
open fun [setPostProcessingEnabled](set-post-processing-enabled.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables or disables post processing. Enabled by default. 

Post-processing includes:

- Depth-of-field
- Bloom
- Vignetting
- Temporal Anti-aliasing (TAA)
- Color grading &gamma encoding
- Dithering
- FXAA
- Dynamic scaling

 Disabling post-processing forgoes color correctness as well as some anti-aliasing techniques and should only be used for debugging, UI overlays or when using custom render targets (see RenderTarget). 

#### Parameters

main

| | |
|---|---|
| enabled | true enables post processing, false disables it |

#### See also

| |
|---|
| [setBloomOptions](set-bloom-options.md) |
| [setColorGrading](set-color-grading.md) |
| [setAntiAliasing](set-anti-aliasing.md) |
| [setDithering](set-dithering.md) |
| [setSampleCount](set-sample-count.md) |
