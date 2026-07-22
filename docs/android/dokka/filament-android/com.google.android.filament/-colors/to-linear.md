//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[toLinear](to-linear.md)

# toLinear

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGB color to linear space, the conversion depends on the specified type.

#### Return

an RGB float array of size 3 with the result of the conversion

#### Parameters

main

| | |
|---|---|
| type | the color space of the RGB color values provided |
| r | the red component |
| g | the green component |
| b | the blue component |

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbType](-rgb-type/index.md), rgb: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGB color to linear space, the conversion depends on the specified type.

#### Return

the passed-in `rgb` array, after applying the conversion

#### Parameters

main

| | |
|---|---|
| type | the color space of the RGB color values provided |
| rgb | an RGB float array of size 3, will be modified |

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), r: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), g: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), b: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), a: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGBA color to linear space, with pre-multiplied alpha.

#### Return

an RGBA float array of size 4 with the result of the conversion

#### Parameters

main

| | |
|---|---|
| type | the color space and type of RGBA color values provided |
| r | the red component |
| g | the green component |
| b | the blue component |
| a | the alpha component |

[main]\
open fun [toLinear](to-linear.md)(type: [Colors.RgbaType](-rgba-type/index.md), rgba: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGBA color to linear space, with pre-multiplied alpha.

#### Return

the passed-in `rgba` array, after applying the conversion

#### Parameters

main

| | |
|---|---|
| type | the color space of the RGBA color values provided |
| rgba | an RGBA float array of size 4, will be modified |

[main]\
open fun [toLinear](to-linear.md)(conversion: [Colors.Conversion](-conversion/index.md), rgb: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGB color in sRGB space to an RGB color in linear space.

#### Return

the passed-in `rgb` array, after applying the conversion. The alpha channel, if present, is left unmodified.

#### Parameters

main

| | |
|---|---|
| conversion | the conversion algorithm to use |
| rgb | an RGB float array of at least size 3, will be modified |
