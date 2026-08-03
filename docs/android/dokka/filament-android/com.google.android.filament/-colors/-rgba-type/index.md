//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Colors](../index.md)/[RgbaType](index.md)

# RgbaType

[main]\
enum [RgbaType](index.md)

Types of RGBA colors.

## Entries

| | |
|---|---|
| [SRGB](-s-r-g-b/index.md) | [main]<br>[SRGB](-s-r-g-b/index.md)<br>The color is defined in sRGB space and the RGB values have not been premultiplied by the alpha (for instance, a 50% transparent red is <1,0,0,0.5>). |
| [LINEAR](-l-i-n-e-a-r/index.md) | [main]<br>[LINEAR](-l-i-n-e-a-r/index.md)<br>The color is defined in linear space and the RGB values have not been premultiplied by the alpha (for instance, a 50% transparent red is <1,0,0,0.5>). |
| [PREMULTIPLIED_SRGB](-p-r-e-m-u-l-t-i-p-l-i-e-d_-s-r-g-b/index.md) | [main]<br>[PREMULTIPLIED_SRGB](-p-r-e-m-u-l-t-i-p-l-i-e-d_-s-r-g-b/index.md)<br>The color is defined in sRGB space and the RGB values have been premultiplied by the alpha (for instance, a 50% transparent red is <0.5,0,0,0.5>). |
| [PREMULTIPLIED_LINEAR](-p-r-e-m-u-l-t-i-p-l-i-e-d_-l-i-n-e-a-r/index.md) | [main]<br>[PREMULTIPLIED_LINEAR](-p-r-e-m-u-l-t-i-p-l-i-e-d_-l-i-n-e-a-r/index.md)<br>The color is defined in linear space and the RGB values have been premultiplied by the alpha (for instance, a 50% transparent red is <0.5,0,0,0.5>). |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Colors.RgbaType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Colors.RgbaType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
