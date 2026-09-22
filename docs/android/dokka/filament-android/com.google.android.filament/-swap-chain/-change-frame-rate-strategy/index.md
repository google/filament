//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[SwapChain](../index.md)/[ChangeFrameRateStrategy](index.md)

# ChangeFrameRateStrategy

[main]\
enum [ChangeFrameRateStrategy](index.md)

Frame rate change strategy for setFrameRate().

## Entries

| | |
|---|---|
| [ONLY_IF_SEAMLESS](-o-n-l-y_-i-f_-s-e-a-m-l-e-s-s/index.md) | [main]<br>[ONLY_IF_SEAMLESS](-o-n-l-y_-i-f_-s-e-a-m-l-e-s-s/index.md)<br>The frame rate transition is applied only if the display controller can perform it seamlessly without visual glitches or disruptive display mode switch blackouts. |
| [ALWAYS](-a-l-w-a-y-s/index.md) | [main]<br>[ALWAYS](-a-l-w-a-y-s/index.md)<br>The transition is applied immediately, even if it requires a non-seamless display mode switch that introduces brief screen interruptions or visual artifacts. |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [SwapChain.ChangeFrameRateStrategy](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[SwapChain.ChangeFrameRateStrategy](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
