//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[toSRGB](to-s-r-g-b.md)

# toSRGB

[main]\
open fun [toSRGB](to-s-r-g-b.md)(color: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGB color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGB color in Rec.709-sRGB-D65 (sRGB) space.

[main]\
open fun [toSRGB](to-s-r-g-b.md)(colorx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colory: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGB color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGB color in Rec.709-sRGB-D65 (sRGB) space.

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |

[main]\
open fun [toSRGB](to-s-r-g-b.md)(colorx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colory: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), colorw: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts an RGBA color in Rec.709-Linear-D65 (&quot;linear sRGB&quot;) space to an RGBA color in Rec.709-sRGB-D65 (sRGB) space the alpha component is left unmodified.

#### Parameters

main

| | |
|---|---|
| colorx | (x component) |
| colory | (y component) |
| colorz | (z component) |
| colorw | (w component) |
