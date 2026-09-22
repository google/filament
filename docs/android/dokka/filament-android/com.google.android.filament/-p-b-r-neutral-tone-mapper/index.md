//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[PBRNeutralToneMapper](index.md)

# PBRNeutralToneMapper

open class [PBRNeutralToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

Khronos PBR Neutral tone mapping operator. 

This tone mapper was designed to preserve the appearance of materials across lighting conditions while avoiding artifacts in the highlights in high dynamic range conditions.

#### Inheritors

| |
|---|
| [PBRNeutral](../-tone-mapper/-p-b-r-neutral/index.md) |

## Constructors

| | |
|---|---|
| [PBRNeutralToneMapper](-p-b-r-neutral-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [ToneMapper](../-tone-mapper/index.md) |
