//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[Shading](index.md)

# Shading

[main]\
enum [Shading](index.md)

Supported shading models

## Entries

| | |
|---|---|
| [UNLIT](-u-n-l-i-t/index.md) | [main]<br>[UNLIT](-u-n-l-i-t/index.md)<br>no lighting applied, emissive possible |
| [LIT](-l-i-t/index.md) | [main]<br>[LIT](-l-i-t/index.md)<br>default, standard lighting |
| [SUBSURFACE](-s-u-b-s-u-r-f-a-c-e/index.md) | [main]<br>[SUBSURFACE](-s-u-b-s-u-r-f-a-c-e/index.md)<br>subsurface lighting model |
| [CLOTH](-c-l-o-t-h/index.md) | [main]<br>[CLOTH](-c-l-o-t-h/index.md)<br>cloth lighting model |
| [SPECULAR_GLOSSINESS](-s-p-e-c-u-l-a-r_-g-l-o-s-s-i-n-e-s-s/index.md) | [main]<br>[SPECULAR_GLOSSINESS](-s-p-e-c-u-l-a-r_-g-l-o-s-s-i-n-e-s-s/index.md)<br>legacy lighting model |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.Shading](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[Material.Shading](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
