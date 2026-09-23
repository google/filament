//[filament-android](../../../../../index.md)/[com.google.android.filament](../../../index.md)/[RenderableManager](../../index.md)/[Builder](../index.md)/[MorphType](index.md)

# MorphType

[main]\
enum [MorphType](index.md)

Type of morphing for a Renderable. 

This usually acts as a bitmask of multiple types.

## Entries

| | |
|---|---|
| [NONE](-n-o-n-e/index.md) | [main]<br>[NONE](-n-o-n-e/index.md) |
| [POSITION](-p-o-s-i-t-i-o-n/index.md) | [main]<br>[POSITION](-p-o-s-i-t-i-o-n/index.md) |
| [TANGENT](-t-a-n-g-e-n-t/index.md) | [main]<br>[TANGENT](-t-a-n-g-e-n-t/index.md) |
| [CUSTOM](-c-u-s-t-o-m/index.md) | [main]<br>[CUSTOM](-c-u-s-t-o-m/index.md) |

## Functions

| Name | Summary |
|---|---|
| [from](from.md) | [main]<br>open fun [from](from.md)(value: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder.MorphType](index.md) |
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [RenderableManager.Builder.MorphType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[RenderableManager.Builder.MorphType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
