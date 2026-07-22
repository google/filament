//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Stream](../index.md)/[StreamType](index.md)

# StreamType

[main]\
enum [StreamType](index.md)

Represents the immutable stream type.

## Entries

| | |
|---|---|
| [NATIVE](-n-a-t-i-v-e/index.md) | [main]<br>[NATIVE](-n-a-t-i-v-e/index.md)<br>Not synchronized but copy-free. Good for video. |
| [ACQUIRED](-a-c-q-u-i-r-e-d/index.md) | [main]<br>[ACQUIRED](-a-c-q-u-i-r-e-d/index.md)<br>Synchronized, copy-free, and take a release callback. Good for AR but requires API 26+. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Stream.StreamType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Stream.StreamType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
