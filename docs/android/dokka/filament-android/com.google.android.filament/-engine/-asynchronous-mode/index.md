//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[AsynchronousMode](index.md)

# AsynchronousMode

[main]\
enum [AsynchronousMode](index.md)

## Entries

| | |
|---|---|
| [NONE](-n-o-n-e/index.md) | [main]<br>[NONE](-n-o-n-e/index.md)<br>Asynchronous operations are disabled. |
| [THREAD_PREFERRED](-t-h-r-e-a-d_-p-r-e-f-e-r-r-e-d/index.md) | [main]<br>[THREAD_PREFERRED](-t-h-r-e-a-d_-p-r-e-f-e-r-r-e-d/index.md)<br>Attempts to use a dedicated worker thread for asynchronous tasks. |
| [AMORTIZATION](-a-m-o-r-t-i-z-a-t-i-o-n/index.md) | [main]<br>[AMORTIZATION](-a-m-o-r-t-i-z-a-t-i-o-n/index.md)<br>Uses an amortization strategy, processing a small number of asynchronous tasks during each engine update cycle. |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Engine.AsynchronousMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Engine.AsynchronousMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
