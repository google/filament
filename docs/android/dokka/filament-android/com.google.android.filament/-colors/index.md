//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)

# Colors

[main]\
open class [Colors](index.md)

Utilities to manipulate and convert colors.

## Types

| Name | Summary |
|---|---|
| [Conversion](-conversion/index.md) | [main]<br>enum [Conversion](-conversion/index.md)<br>Type of color conversion to use when converting to/from sRGB and linear spaces. |
| [LinearColor](-linear-color/index.md) | [main]<br>@[Retention](https://developer.android.com/reference/kotlin/java/lang/annotation/Retention.html)(value = [SOURCE](https://developer.android.com/reference/kotlin/java/lang/annotation/RetentionPolicy.html#SOURCE))<br>@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [])<br>annotation class [LinearColor](-linear-color/index.md) |
| [RgbaType](-rgba-type/index.md) | [main]<br>enum [RgbaType](-rgba-type/index.md)<br>Types of RGBA colors. |
| [RgbType](-rgb-type/index.md) | [main]<br>enum [RgbType](-rgb-type/index.md)<br>Types of RGB colors. |

## Functions

| Name | Summary |
|---|---|
| [cct](cct.md) | [main]<br>open fun [cct](cct.md)(temperature: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Converts a correlated color temperature to a linear RGB color in sRGB space. |
| [illuminantD](illuminant-d.md) | [main]<br>open fun [illuminantD](illuminant-d.md)(temperature: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Converts a CIE standard illuminant series D to a linear RGB color in sRGB space. |
| [toLinear](to-linear.md) | [main]<br>open fun [toLinear](to-linear.md)(conversion: [Colors.Conversion](-conversion/index.md), rgb: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Converts an RGB color in sRGB space to an RGB color in linear space.<br>[main]<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), rgb: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Converts an RGB color to linear space, the conversion depends on the specified type.<br>[main]<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), rgba: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), a: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Converts an RGBA color to linear space, with pre-multiplied alpha. |
