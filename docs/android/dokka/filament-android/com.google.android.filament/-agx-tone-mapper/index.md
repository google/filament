//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[AgxToneMapper](index.md)

# AgxToneMapper

open class [AgxToneMapper](index.md) : [ToneMapper](../-tone-mapper/index.md)

AgX tone mapping operator.

#### Inheritors

| |
|---|
| [Agx](../-tone-mapper/-agx/index.md) |

## Constructors

| | |
|---|---|
| [AgxToneMapper](-agx-tone-mapper.md) | [main]<br>constructor()<br>Builds a new AgX tone mapper.<br>constructor(look: [AgxToneMapper.AgxLook](-agx-look/index.md))<br>Builds a new AgX tone mapper. |

## Types

| Name | Summary |
|---|---|
| [AgxLook](-agx-look/index.md) | [main]<br>enum [AgxLook](-agx-look/index.md) |

## Properties

| Name | Summary |
|---|---|
| [look](look.md) | [main]<br>open var [look](look.md): [AgxToneMapper.AgxLook](-agx-look/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../-tone-mapper/get-native-object.md) | [main]<br>open fun [getNativeObject](../-tone-mapper/get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [isLDR](is-l-d-r.md) | [main]<br>open fun [isLDR](is-l-d-r.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](is-one-dimensional.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [wrap](../-tone-mapper/wrap.md) | [main]<br>open fun [wrap](../-tone-mapper/wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [ToneMapper](../-tone-mapper/index.md) |
