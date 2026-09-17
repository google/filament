//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[SwapChain](../index.md)/[FrameRateCompatibility](index.md)

# FrameRateCompatibility

[main]\
enum [FrameRateCompatibility](index.md)

Frame rate compatibility mode for setFrameRate().

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md)<br>The OS matches the frame rate when the surface is active, but may pick a different rate to better harmonize with concurrent windows or display power policies. |
| [FIXED_SOURCE](-f-i-x-e-d_-s-o-u-r-c-e/index.md) | [main]<br>[FIXED_SOURCE](-f-i-x-e-d_-s-o-u-r-c-e/index.md)<br>The surface represents a fixed-rate source (like video). |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [SwapChain.FrameRateCompatibility](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[SwapChain.FrameRateCompatibility](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
