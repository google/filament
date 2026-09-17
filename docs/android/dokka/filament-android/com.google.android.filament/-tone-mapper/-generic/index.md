//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ToneMapper](../index.md)/[Generic](index.md)

# Generic

[main]\
open class [Generic](index.md) : [GenericToneMapper](../../-generic-tone-mapper/index.md)

## Constructors

| | |
|---|---|
| [Generic](-generic.md) | [main]<br>constructor()constructor(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayIn: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayOut: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), hdrMax: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)) |

## Functions

| Name | Summary |
|---|---|
| [getContrast](../../-generic-tone-mapper/get-contrast.md) | [main]<br>open fun [getContrast](../../-generic-tone-mapper/get-contrast.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the contrast of the curve as a strictly positive value. |
| [getHdrMax](../../-generic-tone-mapper/get-hdr-max.md) | [main]<br>open fun [getHdrMax](../../-generic-tone-mapper/get-hdr-max.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the maximum input value that will map to output white, as a value >= 1.0. |
| [getMidGrayIn](../../-generic-tone-mapper/get-mid-gray-in.md) | [main]<br>open fun [getMidGrayIn](../../-generic-tone-mapper/get-mid-gray-in.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the middle gray point for input values as a value between 0.0 and 1.0. |
| [getMidGrayOut](../../-generic-tone-mapper/get-mid-gray-out.md) | [main]<br>open fun [getMidGrayOut](../../-generic-tone-mapper/get-mid-gray-out.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the middle gray point for output values as a value between 0.0 and 1.0. |
| [getNativeObject](../get-native-object.md) | [main]<br>open fun [getNativeObject](../get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [isLDR](../../-generic-tone-mapper/is-l-d-r.md) | [main]<br>open fun [isLDR](../../-generic-tone-mapper/is-l-d-r.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>True if this tonemapper only works in low-dynamic-range. |
| [isOneDimensional](../../-generic-tone-mapper/is-one-dimensional.md) | [main]<br>open fun [isOneDimensional](../../-generic-tone-mapper/is-one-dimensional.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>If true, then this function holds that f(x) = vec3(f(x.r), f(x.g), f(x. |
| [setContrast](../../-generic-tone-mapper/set-contrast.md) | [main]<br>open fun [setContrast](../../-generic-tone-mapper/set-contrast.md)(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| [setHdrMax](../../-generic-tone-mapper/set-hdr-max.md) | [main]<br>open fun [setHdrMax](../../-generic-tone-mapper/set-hdr-max.md)(hdrMax: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Defines the maximum input value that will be mapped to output white. |
| [setMidGrayIn](../../-generic-tone-mapper/set-mid-gray-in.md) | [main]<br>open fun [setMidGrayIn](../../-generic-tone-mapper/set-mid-gray-in.md)(midGrayIn: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the input middle gray, between 0.0 and 1.0. |
| [setMidGrayOut](../../-generic-tone-mapper/set-mid-gray-out.md) | [main]<br>open fun [setMidGrayOut](../../-generic-tone-mapper/set-mid-gray-out.md)(midGrayOut: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the output middle gray, between 0.0 and 1.0. |
| [wrap](../wrap.md) | [main]<br>open fun [wrap](../wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [ToneMapper](../index.md) |
