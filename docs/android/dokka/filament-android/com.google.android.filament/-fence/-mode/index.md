//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Fence](../index.md)/[Mode](index.md)

# Mode

[main]\
enum [Mode](index.md)

Mode controls the behavior of the command stream when calling wait() 

@attention It would be unwise to call `wait(..., Mode::DONT_FLUSH)` from the same thread the Fence was created, as it would most certainly create a dead-lock.

## Entries

| | |
|---|---|
| [FLUSH](-f-l-u-s-h/index.md) | [main]<br>[FLUSH](-f-l-u-s-h/index.md)<br>The command stream is flushed |
| [DONT_FLUSH](-d-o-n-t_-f-l-u-s-h/index.md) | [main]<br>[DONT_FLUSH](-d-o-n-t_-f-l-u-s-h/index.md)<br>The command stream is not flushed |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Fence.Mode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Fence.Mode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
