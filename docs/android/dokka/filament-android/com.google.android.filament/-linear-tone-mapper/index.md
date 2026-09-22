//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LinearToneMapper](index.md)

# LinearToneMapper

open class [LinearToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

Linear tone mapping operator that returns the input color but clamped to the 0..1 range. 

This operator is mostly useful for debugging.

#### Inheritors

| |
|---|
| [Linear](../-tone-mapper/-linear/index.md) |

## Constructors

| | |
|---|---|
| [LinearToneMapper](-linear-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
