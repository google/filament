//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Type](index.md)

# Type

[main]\
enum [Type](index.md)

Pixel data type

## Entries

| | |
|---|---|
| [UBYTE](-u-b-y-t-e/index.md) | [main]<br>[UBYTE](-u-b-y-t-e/index.md)<br>unsigned byte, 8-bit |
| [BYTE](-b-y-t-e/index.md) | [main]<br>[BYTE](-b-y-t-e/index.md)<br>signed byte, 8-bit |
| [USHORT](-u-s-h-o-r-t/index.md) | [main]<br>[USHORT](-u-s-h-o-r-t/index.md)<br>unsigned short, 16-bits |
| [SHORT](-s-h-o-r-t/index.md) | [main]<br>[SHORT](-s-h-o-r-t/index.md)<br>signed short, 16-bit |
| [UINT](-u-i-n-t/index.md) | [main]<br>[UINT](-u-i-n-t/index.md)<br>unsigned int, 32-bit |
| [INT](-i-n-t/index.md) | [main]<br>[INT](-i-n-t/index.md)<br>signed int, 32-bit |
| [HALF](-h-a-l-f/index.md) | [main]<br>[HALF](-h-a-l-f/index.md)<br>half-float, 16-bit float with 10 bits mantissa |
| [FLOAT](-f-l-o-a-t/index.md) | [main]<br>[FLOAT](-f-l-o-a-t/index.md)<br>float, 32-bit float, with 24 bits mantissa |
| [COMPRESSED](-c-o-m-p-r-e-s-s-e-d/index.md) | [main]<br>[COMPRESSED](-c-o-m-p-r-e-s-s-e-d/index.md)<br>a compressed type |
| [UINT_10F_11F_11F_REV](-u-i-n-t_10-f_11-f_11-f_-r-e-v/index.md) | [main]<br>[UINT_10F_11F_11F_REV](-u-i-n-t_10-f_11-f_11-f_-r-e-v/index.md)<br>unsigned 5.6 (5.5 for blue) float packed in a 32-bit integer. |
| [USHORT_565](-u-s-h-o-r-t_565/index.md) | [main]<br>[USHORT_565](-u-s-h-o-r-t_565/index.md)<br>unsigned 5/6 bit integers packed in a 16-bit short. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Texture.Type](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Texture.Type](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
