//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[UboBatchingMode](index.md)

# UboBatchingMode

[main]\
enum [UboBatchingMode](index.md)

Defines whether a material instance should use UBO batching or not.

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md)<br>For default, it follows the engine settings. If UBO batching is enabled on the engine and the material domain is SURFACE, it turns on the UBO batching. Otherwise, it turns off the UBO batching. |
| [DISABLED](-d-i-s-a-b-l-e-d/index.md) | [main]<br>[DISABLED](-d-i-s-a-b-l-e-d/index.md)<br>Disable the Ubo Batching for this material |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.UboBatchingMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.UboBatchingMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
