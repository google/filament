//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[BloomOptions](index.md)

# BloomOptions

[main]\
open class [BloomOptions](index.md)

Options to control the bloom effect 

- enabled: Enable or disable the bloom post-processing effect. Disabled by default.
- levels: Number of successive blurs to achieve the blur effect, the minimum is 3 and the maximum is 12. This value together with resolution influences the spread of the blur effect. This value can be silently reduced to accommodate the original image size.
- resolution: Resolution of bloom's minor axis. The minimum value is 2^levels and the the maximum is lower of the original resolution and 4096. This parameter is silently clamped to the minimum and maximum. It is highly recommended that this value be smaller than the target resolution after dynamic resolution is applied (horizontally and vertically).
- strength: how much of the bloom is added to the original image. Between 0 and 1.
- blendMode: Whether the bloom effect is purely additive (false) or mixed with the original image (true).
- threshold: When enabled, a threshold at 1.0 is applied on the source image, this is useful for artistic reasons and is usually needed when a dirt texture is used.
- dirt: A dirt/scratch/smudges texture (that can be RGB), which gets added to the bloom effect. Smudges are visible where bloom occurs. Threshold must be enabled for the dirt effect to work properly.
- dirtStrength: Strength of the dirt texture.

## Constructors

| | |
|---|---|
| [BloomOptions](-bloom-options.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [BlendMode](-blend-mode/index.md) | [main]<br>enum [BlendMode](-blend-mode/index.md) |

## Properties

| Name | Summary |
|---|---|
| [blendMode](blend-mode.md) | [main]<br>open var [blendMode](blend-mode.md): [View.BloomOptions.BlendMode](-blend-mode/index.md)<br>how the bloom effect is applied |
| [chromaticAberration](chromatic-aberration.md) | [main]<br>open var [chromaticAberration](chromatic-aberration.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>amount of chromatic aberration |
| [dirt](dirt.md) | [main]<br>open var [dirt](dirt.md): [Texture](../../-texture/index.md)<br>user provided dirt texture |
| [dirtStrength](dirt-strength.md) | [main]<br>open var [dirtStrength](dirt-strength.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>strength of the dirt texture |
| [enabled](enabled.md) | [main]<br>open var [enabled](enabled.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enable or disable bloom |
| [ghostCount](ghost-count.md) | [main]<br>open var [ghostCount](ghost-count.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>number of flare &quot;ghosts&quot; |
| [ghostSpacing](ghost-spacing.md) | [main]<br>open var [ghostSpacing](ghost-spacing.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>spacing of the ghost in screen units [0, 1[ |
| [ghostThreshold](ghost-threshold.md) | [main]<br>open var [ghostThreshold](ghost-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>hdr threshold for the ghosts |
| [haloRadius](halo-radius.md) | [main]<br>open var [haloRadius](halo-radius.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>radius of halo in vertical screen units [0, 0. |
| [haloThickness](halo-thickness.md) | [main]<br>open var [haloThickness](halo-thickness.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>thickness of halo in vertical screen units, 0 to disable |
| [haloThreshold](halo-threshold.md) | [main]<br>open var [haloThreshold](halo-threshold.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>hdr threshold for the halo |
| [highlight](highlight.md) | [main]<br>open var [highlight](highlight.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>limit highlights to this value before bloom [10, +inf] |
| [lensFlare](lens-flare.md) | [main]<br>open var [lensFlare](lens-flare.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enable screen-space lens flare |
| [levels](levels.md) | [main]<br>open var [levels](levels.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>number of blur levels (1 to 11) |
| [quality](quality.md) | [main]<br>open var [quality](quality.md): [View.QualityLevel](../-quality-level/index.md)<br>Bloom quality level. |
| [resolution](resolution.md) | [main]<br>open var [resolution](resolution.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>resolution of vertical axis (2^levels to 2048) |
| [starburst](starburst.md) | [main]<br>open var [starburst](starburst.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>enable starburst effect on lens flare |
| [strength](strength.md) | [main]<br>open var [strength](strength.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>bloom's strength between 0.0 and 1. |
| [threshold](threshold.md) | [main]<br>open var [threshold](threshold.md): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>whether to threshold the source |
