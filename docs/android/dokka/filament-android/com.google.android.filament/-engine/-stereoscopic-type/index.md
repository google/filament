//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[StereoscopicType](index.md)

# StereoscopicType

[main]\
enum [StereoscopicType](index.md)

The type of technique for stereoscopic rendering. (Note that the materials used will need to be compatible with the chosen technique.)

## Entries

| | |
|---|---|
| [NONE](-n-o-n-e/index.md) | [main]<br>[NONE](-n-o-n-e/index.md)<br>No stereoscopic rendering. |
| [INSTANCED](-i-n-s-t-a-n-c-e-d/index.md) | [main]<br>[INSTANCED](-i-n-s-t-a-n-c-e-d/index.md)<br>Stereoscopic rendering is performed using instanced rendering technique. |
| [MULTIVIEW](-m-u-l-t-i-v-i-e-w/index.md) | [main]<br>[MULTIVIEW](-m-u-l-t-i-v-i-e-w/index.md)<br>Stereoscopic rendering is performed using the multiview feature from the graphics backend. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Engine.StereoscopicType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Engine.StereoscopicType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
