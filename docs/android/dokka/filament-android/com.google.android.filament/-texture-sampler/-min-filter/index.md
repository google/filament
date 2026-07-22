//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[TextureSampler](../index.md)/[MinFilter](index.md)

# MinFilter

[main]\
enum [MinFilter](index.md)

## Entries

| | |
|---|---|
| [NEAREST](-n-e-a-r-e-s-t/index.md) | [main]<br>[NEAREST](-n-e-a-r-e-s-t/index.md)<br>No filtering. Nearest neighbor is used. |
| [LINEAR](-l-i-n-e-a-r/index.md) | [main]<br>[LINEAR](-l-i-n-e-a-r/index.md)<br>Box filtering. Weighted average of 4 neighbors is used. |
| [NEAREST_MIPMAP_NEAREST](-n-e-a-r-e-s-t_-m-i-p-m-a-p_-n-e-a-r-e-s-t/index.md) | [main]<br>[NEAREST_MIPMAP_NEAREST](-n-e-a-r-e-s-t_-m-i-p-m-a-p_-n-e-a-r-e-s-t/index.md)<br>Mip-mapping is activated. But no filtering occurs. |
| [LINEAR_MIPMAP_NEAREST](-l-i-n-e-a-r_-m-i-p-m-a-p_-n-e-a-r-e-s-t/index.md) | [main]<br>[LINEAR_MIPMAP_NEAREST](-l-i-n-e-a-r_-m-i-p-m-a-p_-n-e-a-r-e-s-t/index.md)<br>Box filtering within a mip-map level. |
| [NEAREST_MIPMAP_LINEAR](-n-e-a-r-e-s-t_-m-i-p-m-a-p_-l-i-n-e-a-r/index.md) | [main]<br>[NEAREST_MIPMAP_LINEAR](-n-e-a-r-e-s-t_-m-i-p-m-a-p_-l-i-n-e-a-r/index.md)<br>Mip-map levels are interpolated, but no other filtering occurs. |
| [LINEAR_MIPMAP_LINEAR](-l-i-n-e-a-r_-m-i-p-m-a-p_-l-i-n-e-a-r/index.md) | [main]<br>[LINEAR_MIPMAP_LINEAR](-l-i-n-e-a-r_-m-i-p-m-a-p_-l-i-n-e-a-r/index.md)<br>Both interpolated Mip-mapping and linear filtering are used. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [TextureSampler.MinFilter](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[TextureSampler.MinFilter](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
