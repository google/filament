//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[GT7ToneMapper](index.md)

# GT7ToneMapper

open class [GT7ToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

Gran Turismo 7 tone mapping operator. 

This tone mapper was designed to preserve the appearance of materials across lighting conditions while avoiding artifacts in the highlights in high dynamic range conditions. This tone mapper targets an SDR paper white value of 250 nits, with a reference luminance of 100 cd/m^2 (a value of 1.0 in the HDR framebuffer).

#### Inheritors

| |
|---|
| [GT7](../-tone-mapper/-g-t7/index.md) |

## Constructors

| | |
|---|---|
| [GT7ToneMapper](-g-t7-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): Long |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): Boolean<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): Boolean<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: Long): [ToneMapper](../-tone-mapper/index.md) |
