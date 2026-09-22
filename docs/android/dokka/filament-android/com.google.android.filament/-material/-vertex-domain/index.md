//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[VertexDomain](index.md)

# VertexDomain

[main]\
enum [VertexDomain](index.md)

Supported types of vertex domains.

## Entries

| | |
|---|---|
| [OBJECT](-o-b-j-e-c-t/index.md) | [main]<br>[OBJECT](-o-b-j-e-c-t/index.md)<br>vertices are in object space, default |
| [WORLD](-w-o-r-l-d/index.md) | [main]<br>[WORLD](-w-o-r-l-d/index.md)<br>vertices are in world space |
| [VIEW](-v-i-e-w/index.md) | [main]<br>[VIEW](-v-i-e-w/index.md)<br>vertices are in view space |
| [DEVICE](-d-e-v-i-c-e/index.md) | [main]<br>[DEVICE](-d-e-v-i-c-e/index.md)<br>vertices are in normalized device space |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.VertexDomain](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[Material.VertexDomain](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
