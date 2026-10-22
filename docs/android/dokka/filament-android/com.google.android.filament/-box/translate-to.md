//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)/[translateTo](translate-to.md)

# translateTo

[main]\
open fun [translateTo](translate-to.md)(trx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), try_: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), trz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Box](index.md)

Translates the box *to* a given center position

#### Return

A box centered in `tr` with the same extent than *this

#### Parameters

main

| | |
|---|---|
| trx | (x component) position to translate the box to |
| try_ | (y component) position to translate the box to |
| trz | (z component) position to translate the box to |

[main]\
open fun [translateTo](translate-to.md)(tr: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Box](index.md)

Translates the box *to* a given center position

#### Return

A box centered in `tr` with the same extent than *this

#### Parameters

main

| | |
|---|---|
| tr | position to translate the box to |
