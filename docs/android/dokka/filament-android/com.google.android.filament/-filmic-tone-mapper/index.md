//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FilmicToneMapper](index.md)

# FilmicToneMapper

open class [FilmicToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

&quot;Filmic&quot; tone mapping operator. 

This tone mapper was designed to approximate the aesthetics of the ACES RRT + ODT for Rec.709 and historically Filament's default tone mapping operator. It exists only for backward compatibility purposes and is not otherwise recommended.

#### Inheritors

| |
|---|
| [Filmic](../-tone-mapper/-filmic/index.md) |

## Constructors

| | |
|---|---|
| [FilmicToneMapper](-filmic-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
