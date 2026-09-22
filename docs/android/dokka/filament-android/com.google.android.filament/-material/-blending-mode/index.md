//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[BlendingMode](index.md)

# BlendingMode

[main]\
enum [BlendingMode](index.md)

Supported blending modes

## Entries

| | |
|---|---|
| [OPAQUE](-o-p-a-q-u-e/index.md) | [main]<br>[OPAQUE](-o-p-a-q-u-e/index.md)<br>material is opaque |
| [TRANSPARENT](-t-r-a-n-s-p-a-r-e-n-t/index.md) | [main]<br>[TRANSPARENT](-t-r-a-n-s-p-a-r-e-n-t/index.md)<br>material is transparent and color is alpha-pre-multiplied, affects diffuse lighting only |
| [ADD](-a-d-d/index.md) | [main]<br>[ADD](-a-d-d/index.md)<br>material is additive (e.g.: hologram) |
| [MASKED](-m-a-s-k-e-d/index.md) | [main]<br>[MASKED](-m-a-s-k-e-d/index.md)<br>material is masked (i.e. alpha tested) |
| [FADE](-f-a-d-e/index.md) | [main]<br>[FADE](-f-a-d-e/index.md)<br>material is transparent and color is alpha-pre-multiplied, affects specular lighting when adding more entries, change the size of FRenderer::CommandKey::blending |
| [MULTIPLY](-m-u-l-t-i-p-l-y/index.md) | [main]<br>[MULTIPLY](-m-u-l-t-i-p-l-y/index.md)<br>material darkens what's behind it |
| [SCREEN](-s-c-r-e-e-n/index.md) | [main]<br>[SCREEN](-s-c-r-e-e-n/index.md)<br>material brightens what's behind it |
| [CUSTOM](-c-u-s-t-o-m/index.md) | [main]<br>[CUSTOM](-c-u-s-t-o-m/index.md)<br>custom blending function |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.BlendingMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[Material.BlendingMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
