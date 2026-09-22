//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[ACESToneMapper](index.md)

# ACESToneMapper

open class [ACESToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

ACES tone mapping operator. 

This operator is an implementation of the ACES Reference Rendering Transform (RRT) combined with the Output Device Transform (ODT) for sRGB monitors (dim surround, 100 nits).

#### Inheritors

| |
|---|
| [ACES](../-tone-mapper/-a-c-e-s/index.md) |

## Constructors

| | |
|---|---|
| [ACESToneMapper](-a-c-e-s-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
