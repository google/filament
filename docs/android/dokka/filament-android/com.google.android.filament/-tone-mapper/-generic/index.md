//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ToneMapper](../index.md)/[Generic](index.md)

# Generic

[main]\
open class [Generic](index.md) : [ToneMapper](../index.md)

Generic tone mapping operator that gives control over the tone mapping curve. This operator can be used to control the aesthetics of the final image. This operator also allows to control the dynamic range of the scene referred values. The tone mapping curve is defined by 5 parameters: 

- contrast: controls the contrast of the curve
- referred values map to output white
- midGrayIn: sets the input middle gray
- midGrayOut: sets the output middle gray
- hdrMax: defines the maximum input value that will be mapped to output white

## Constructors

| | |
|---|---|
| [Generic](-generic.md) | [main]<br>constructor()<br>Builds a new generic tone mapper parameterized to closely approximate the [ACESLegacy](../-a-c-e-s-legacy/index.md) tone mapper.<br>constructor(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayIn: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayOut: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), hdrMax: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Builds a new generic tone mapper. |

## Functions

| Name | Summary |
|---|---|
| [getContrast](get-contrast.md) | [main]<br>open fun [getContrast](get-contrast.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the contrast of the curve as a strictly positive value. |
| [getHdrMax](get-hdr-max.md) | [main]<br>open fun [getHdrMax](get-hdr-max.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the maximum input value that will map to output white, as a value >= 1.0. |
| [getMidGrayIn](get-mid-gray-in.md) | [main]<br>open fun [getMidGrayIn](get-mid-gray-in.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the middle gray point for input values as a value between 0.0 and 1.0. |
| [getMidGrayOut](get-mid-gray-out.md) | [main]<br>open fun [getMidGrayOut](get-mid-gray-out.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns the middle gray point for output values as a value between 0.0 and 1.0. |
| [getNativeObject](../get-native-object.md) | [main]<br>open fun [getNativeObject](../get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setContrast](set-contrast.md) | [main]<br>open fun [setContrast](set-contrast.md)(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| [setHdrMax](set-hdr-max.md) | [main]<br>open fun [setHdrMax](set-hdr-max.md)(hdrMax: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Defines the maximum input value that will be mapped to output white. |
| [setMidGrayIn](set-mid-gray-in.md) | [main]<br>open fun [setMidGrayIn](set-mid-gray-in.md)(midGrayIn: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the input middle gray, between 0.0 and 1.0. |
| [setMidGrayOut](set-mid-gray-out.md) | [main]<br>open fun [setMidGrayOut](set-mid-gray-out.md)(midGrayOut: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the output middle gray, between 0.0 and 1.0. |
