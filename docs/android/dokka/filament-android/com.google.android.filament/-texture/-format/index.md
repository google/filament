//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Format](index.md)

# Format

[main]\
enum [Format](index.md)

Pixel Data Format

## Entries

| | |
|---|---|
| [R](-r/index.md) | [main]<br>[R](-r/index.md)<br>One Red channel, float |
| [R_INTEGER](-r_-i-n-t-e-g-e-r/index.md) | [main]<br>[R_INTEGER](-r_-i-n-t-e-g-e-r/index.md)<br>One Red channel, integer |
| [RG](-r-g/index.md) | [main]<br>[RG](-r-g/index.md)<br>Two Red and Green channels, float |
| [RG_INTEGER](-r-g_-i-n-t-e-g-e-r/index.md) | [main]<br>[RG_INTEGER](-r-g_-i-n-t-e-g-e-r/index.md)<br>Two Red and Green channels, integer |
| [RGB](-r-g-b/index.md) | [main]<br>[RGB](-r-g-b/index.md)<br>Three Red, Green and Blue channels, float |
| [RGB_INTEGER](-r-g-b_-i-n-t-e-g-e-r/index.md) | [main]<br>[RGB_INTEGER](-r-g-b_-i-n-t-e-g-e-r/index.md)<br>Three Red, Green and Blue channels, integer |
| [RGBA](-r-g-b-a/index.md) | [main]<br>[RGBA](-r-g-b-a/index.md)<br>Four Red, Green, Blue and Alpha channels, float |
| [RGBA_INTEGER](-r-g-b-a_-i-n-t-e-g-e-r/index.md) | [main]<br>[RGBA_INTEGER](-r-g-b-a_-i-n-t-e-g-e-r/index.md)<br>Four Red, Green, Blue and Alpha channels, integer |
| [UNUSED](-u-n-u-s-e-d/index.md) | [main]<br>[UNUSED](-u-n-u-s-e-d/index.md) |
| [DEPTH_COMPONENT](-d-e-p-t-h_-c-o-m-p-o-n-e-n-t/index.md) | [main]<br>[DEPTH_COMPONENT](-d-e-p-t-h_-c-o-m-p-o-n-e-n-t/index.md)<br>Depth, 16-bit or 24-bits usually |
| [DEPTH_STENCIL](-d-e-p-t-h_-s-t-e-n-c-i-l/index.md) | [main]<br>[DEPTH_STENCIL](-d-e-p-t-h_-s-t-e-n-c-i-l/index.md)<br>Two Depth (24-bits) + Stencil (8-bits) channels |
| [ALPHA](-a-l-p-h-a/index.md) | [main]<br>[ALPHA](-a-l-p-h-a/index.md) |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Texture.Format](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[Texture.Format](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
