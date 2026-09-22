//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[ACESLegacyToneMapper](index.md)

# ACESLegacyToneMapper

open class [ACESLegacyToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

ACES tone mapping operator, modified to match the perceived brightness of FilmicToneMapper. 

This operator is the same as ACESToneMapper but applies a brightness multiplier of ~1.6 to the input color value to target brighter viewing environments.

#### Inheritors

| |
|---|
| [ACESLegacy](../-tone-mapper/-a-c-e-s-legacy/index.md) |

## Constructors

| | |
|---|---|
| [ACESLegacyToneMapper](-a-c-e-s-legacy-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
