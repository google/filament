//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[ToneMapper](index.md)

# ToneMapper

open class [ToneMapper](index.md)

Interface for tone mapping operators. A tone mapping operator, or tone mapper, 

is responsible for compressing the dynamic range of the rendered scene to a dynamic range suitable for display.

In Filament, tone mapping is a color grading step. ToneMapper instances are created and passed to the ColorGrading::Builder to produce a 3D LUT that will be used during post-processing to prepare the final color buffer for display.

Filament provides several default tone mapping operators that fall into three categories:

- Configurable tone mapping operators
- GenericToneMapper
- AgXToneMapper
- Fixed-aesthetic tone mapping operators
- ACESToneMapper
- ACESLegacyToneMapper
- FilmicToneMapper
- PBRNeutralToneMapper
- GT7ToneMapper
- Debug/validation tone mapping operators
- LinearToneMapper
- DisplayRangeToneMapper You can create custom tone mapping operators by subclassing ToneMapper.

#### Inheritors

| |
|---|
| [LinearToneMapper](../-linear-tone-mapper/index.md) |
| [ACESToneMapper](../-a-c-e-s-tone-mapper/index.md) |
| [GenericToneMapper](../-generic-tone-mapper/index.md) |
| [GT7ToneMapper](../-g-t7-tone-mapper/index.md) |
| [DisplayRangeToneMapper](../-display-range-tone-mapper/index.md) |
| [FilmicToneMapper](../-filmic-tone-mapper/index.md) |
| [PBRNeutralToneMapper](../-p-b-r-neutral-tone-mapper/index.md) |
| [AgxToneMapper](../-agx-tone-mapper/index.md) |
| [ACESLegacyToneMapper](../-a-c-e-s-legacy-tone-mapper/index.md) |

## Types

| Name | Summary |
|---|---|
| [ACES](-a-c-e-s/index.md) | [main]<br>open class [ACES](-a-c-e-s/index.md) : [ACESToneMapper](../-a-c-e-s-tone-mapper/index.md) |
| [ACESLegacy](-a-c-e-s-legacy/index.md) | [main]<br>open class [ACESLegacy](-a-c-e-s-legacy/index.md) : [ACESLegacyToneMapper](../-a-c-e-s-legacy-tone-mapper/index.md) |
| [Agx](-agx/index.md) | [main]<br>open class [Agx](-agx/index.md) : [AgxToneMapper](../-agx-tone-mapper/index.md) |
| [Filmic](-filmic/index.md) | [main]<br>open class [Filmic](-filmic/index.md) : [FilmicToneMapper](../-filmic-tone-mapper/index.md) |
| [Generic](-generic/index.md) | [main]<br>open class [Generic](-generic/index.md) : [GenericToneMapper](../-generic-tone-mapper/index.md) |
| [GT7](-g-t7/index.md) | [main]<br>open class [GT7](-g-t7/index.md) : [GT7ToneMapper](../-g-t7-tone-mapper/index.md) |
| [Linear](-linear/index.md) | [main]<br>open class [Linear](-linear/index.md) : [LinearToneMapper](../-linear-tone-mapper/index.md) |
| [PBRNeutral](-p-b-r-neutral/index.md) | [main]<br>open class [PBRNeutral](-p-b-r-neutral/index.md) : [PBRNeutralToneMapper](../-p-b-r-neutral-tone-mapper/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [ToneMapper](index.md) |
