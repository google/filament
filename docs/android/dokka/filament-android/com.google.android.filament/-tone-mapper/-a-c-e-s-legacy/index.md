//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ToneMapper](../index.md)/[ACESLegacy](index.md)

# ACESLegacy

[main]\
open class [ACESLegacy](index.md) : [ToneMapper](../index.md)

ACES tone mapping operator, modified to match the perceived brightness of FilmicToneMapper. This operator is the same as ACESToneMapper but applies a brightness multiplier of ~1.6 to the input color value to target brighter viewing environments.

## Constructors

| | |
|---|---|
| [ACESLegacy](-a-c-e-s-legacy.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../get-native-object.md) | [main]<br>open fun [getNativeObject](../get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
