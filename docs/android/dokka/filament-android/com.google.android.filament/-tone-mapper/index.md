//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[ToneMapper](index.md)

# ToneMapper

open class [ToneMapper](index.md)

Interface for tone mapping operators. A tone mapping operator, or tone mapper, is responsible for compressing the dynamic range of the rendered scene to a dynamic range suitable for display. In Filament, tone mapping is a color grading step. ToneMapper instances are created and passed to the ColorGrading::Builder to produce a 3D LUT that will be used during post-processing to prepare the final color buffer for display. Filament provides several default tone mapping operators that fall into three categories: 

- Configurable tone mapping operators
- - GenericToneMapper
   - AgXToneMapper
- Fixed-aesthetic tone mapping operators
- - ACESToneMapper
   - ACESLegacyToneMapper
   - FilmicToneMapper
   - PBRNeutralToneMapper
   - GT7ToneMapper
- Debug/validation tone mapping operators
- - LinearToneMapper
   - DisplayRangeToneMapper

#### Inheritors

| |
|---|
| [Linear](-linear/index.md) |
| [ACES](-a-c-e-s/index.md) |
| [ACESLegacy](-a-c-e-s-legacy/index.md) |
| [Filmic](-filmic/index.md) |
| [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper/index.md) |
| [GT7ToneMapper](-g-t7-tone-mapper/index.md) |
| [Agx](-agx/index.md) |
| [Generic](-generic/index.md) |

## Types

| Name | Summary |
|---|---|
| [ACES](-a-c-e-s/index.md) | [main]<br>open class [ACES](-a-c-e-s/index.md) : [ToneMapper](index.md)<br>ACES tone mapping operator. |
| [ACESLegacy](-a-c-e-s-legacy/index.md) | [main]<br>open class [ACESLegacy](-a-c-e-s-legacy/index.md) : [ToneMapper](index.md)<br>ACES tone mapping operator, modified to match the perceived brightness of FilmicToneMapper. |
| [Agx](-agx/index.md) | [main]<br>open class [Agx](-agx/index.md) : [ToneMapper](index.md)<br>AgX tone mapping operator. |
| [Filmic](-filmic/index.md) | [main]<br>open class [Filmic](-filmic/index.md) : [ToneMapper](index.md)<br>&quot;Filmic&quot; tone mapping operator. |
| [Generic](-generic/index.md) | [main]<br>open class [Generic](-generic/index.md) : [ToneMapper](index.md)<br>Generic tone mapping operator that gives control over the tone mapping curve. |
| [GT7ToneMapper](-g-t7-tone-mapper/index.md) | [main]<br>open class [GT7ToneMapper](-g-t7-tone-mapper/index.md) : [ToneMapper](index.md)<br>Gran Turismo 7 tone mapping operator. |
| [Linear](-linear/index.md) | [main]<br>open class [Linear](-linear/index.md) : [ToneMapper](index.md)<br>Linear tone mapping operator that returns the input color but clamped to the 0..1 range. |
| [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper/index.md) | [main]<br>open class [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper/index.md) : [ToneMapper](index.md)<br>Khronos PBR Neutral tone mapping operator. |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
