//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[ShaderModel](index.md)

# ShaderModel

[main]\
enum [ShaderModel](index.md)

Shader model. 

These enumerants are used across all backends and refer to a level of functionality and quality.

For example, the OpenGL backend returns `MOBILE` if it supports OpenGL ES, or `DESKTOP` if it supports Desktop OpenGL, this is later used to select the proper shader.

Shader quality vs. performance is also affected by ShaderModel.

## Entries

| | |
|---|---|
| [MOBILE](-m-o-b-i-l-e/index.md) | [main]<br>[MOBILE](-m-o-b-i-l-e/index.md)<br>Mobile level functionality |
| [DESKTOP](-d-e-s-k-t-o-p/index.md) | [main]<br>[DESKTOP](-d-e-s-k-t-o-p/index.md)<br>Desktop level functionality |

## Functions

| Name | Summary |
|---|---|
| [from](from.md) | [main]<br>open fun [from](from.md)(value: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Material.ShaderModel](index.md) |
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.ShaderModel](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.ShaderModel](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
