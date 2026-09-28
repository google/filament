//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Colors](../index.md)/[ColorConversion](index.md)

# ColorConversion

[main]\
enum [ColorConversion](index.md)

type of color conversion to use when converting to/from sRGB and linear spaces

## Entries

| | |
|---|---|
| [ACCURATE](-a-c-c-u-r-a-t-e/index.md) | [main]<br>[ACCURATE](-a-c-c-u-r-a-t-e/index.md)<br>accurate conversion using the sRGB standard |
| [FAST](-f-a-s-t/index.md) | [main]<br>[FAST](-f-a-s-t/index.md)<br>fast conversion using a simple gamma 2.2 curve |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Colors.ColorConversion](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Colors.ColorConversion](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
