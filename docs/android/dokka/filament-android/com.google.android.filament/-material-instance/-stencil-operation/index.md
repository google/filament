//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MaterialInstance](../index.md)/[StencilOperation](index.md)

# StencilOperation

[main]\
enum [StencilOperation](index.md)

Operations that control how the stencil buffer is updated.

## Entries

| | |
|---|---|
| [KEEP](-k-e-e-p/index.md) | [main]<br>[KEEP](-k-e-e-p/index.md)<br>Keeps the current value. |
| [ZERO](-z-e-r-o/index.md) | [main]<br>[ZERO](-z-e-r-o/index.md)<br>Sets the value to 0. |
| [REPLACE](-r-e-p-l-a-c-e/index.md) | [main]<br>[REPLACE](-r-e-p-l-a-c-e/index.md)<br>Sets the value to the stencil reference value. |
| [INCR_CLAMP](-i-n-c-r_-c-l-a-m-p/index.md) | [main]<br>[INCR_CLAMP](-i-n-c-r_-c-l-a-m-p/index.md)<br>Increments the current value. Clamps to the maximum representable unsigned value. |
| [INCR_WRAP](-i-n-c-r_-w-r-a-p/index.md) | [main]<br>[INCR_WRAP](-i-n-c-r_-w-r-a-p/index.md)<br>Increments the current value. Wraps value to zero when incrementing the maximum representable unsigned value. |
| [DECR_CLAMP](-d-e-c-r_-c-l-a-m-p/index.md) | [main]<br>[DECR_CLAMP](-d-e-c-r_-c-l-a-m-p/index.md)<br>Decrements the current value. Clamps to 0. |
| [DECR_WRAP](-d-e-c-r_-w-r-a-p/index.md) | [main]<br>[DECR_WRAP](-d-e-c-r_-w-r-a-p/index.md)<br>Decrements the current value. Wraps value to the maximum representable unsigned value when decrementing a value of zero. |
| [INVERT](-i-n-v-e-r-t/index.md) | [main]<br>[INVERT](-i-n-v-e-r-t/index.md)<br>Bitwise inverts the current value. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [MaterialInstance.StencilOperation](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[MaterialInstance.StencilOperation](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
