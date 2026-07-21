//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ToneMapper](../index.md)/[GT7ToneMapper](index.md)

# GT7ToneMapper

[main]\
open class [GT7ToneMapper](index.md) : [ToneMapper](../index.md)

Gran Turismo 7 tone mapping operator. This tone mapper was designed to preserve the appearance of materials across lighting conditions while avoiding artifacts in the highlights in high dynamic range conditions. This tone mapper targets an SDR paper white value of 250 nits, with a reference luminance of 100 cd/m^2 (a value of 1.0 in the HDR framebuffer).

## Constructors

| | |
|---|---|
| [GT7ToneMapper](-g-t7-tone-mapper.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](../get-native-object.md) | [main]<br>open fun [getNativeObject](../get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
