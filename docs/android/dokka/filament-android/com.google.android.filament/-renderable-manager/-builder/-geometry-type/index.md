//[filament-android](../../../../../index.md)/[com.google.android.filament](../../../index.md)/[RenderableManager](../../index.md)/[Builder](../index.md)/[GeometryType](index.md)

# GeometryType

[main]\
enum [GeometryType](index.md)

Type of geometry for a Renderable

## Entries

| | |
|---|---|
| [DYNAMIC](-d-y-n-a-m-i-c/index.md) | [main]<br>[DYNAMIC](-d-y-n-a-m-i-c/index.md)<br>dynamic gemoetry has no restriction |
| [STATIC_BOUNDS](-s-t-a-t-i-c_-b-o-u-n-d-s/index.md) | [main]<br>[STATIC_BOUNDS](-s-t-a-t-i-c_-b-o-u-n-d-s/index.md)<br>bounds and world space transform are immutable |
| [STATIC](-s-t-a-t-i-c/index.md) | [main]<br>[STATIC](-s-t-a-t-i-c/index.md)<br>skinning/morphing not allowed and Vertex/IndexBuffer immutables |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [RenderableManager.Builder.GeometryType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[RenderableManager.Builder.GeometryType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
