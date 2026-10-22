//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[BloomOptions](index.md)/[quality](quality.md)

# quality

[main]\
open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)

Bloom quality level. 

- LOW (default): use a more optimized down-sampling filter, however there can be artifacts with dynamic resolution, this can be alleviated by using the homogenous mode.
- MEDIUM: Good balance between quality and performance.
- HIGH: In this mode the bloom resolution is automatically increased to avoid artifacts. This mode can be significantly slower on mobile, especially at high resolution. This mode greatly improves the anamorphic bloom.
