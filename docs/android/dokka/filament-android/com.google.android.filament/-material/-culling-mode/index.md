//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[CullingMode](index.md)

# CullingMode

enum [CullingMode](index.md)

Face culling Mode

#### See also

| | |
|---|---|
| &lt;a href=&quot;https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling&quot;&gt; | Rasterization: culling |

## Entries

| | |
|---|---|
| [NONE](-n-o-n-e/index.md) | [main]<br>[NONE](-n-o-n-e/index.md)<br>No culling. Front and back faces are visible. |
| [FRONT](-f-r-o-n-t/index.md) | [main]<br>[FRONT](-f-r-o-n-t/index.md)<br>Front face culling. Only back faces are visible. |
| [BACK](-b-a-c-k/index.md) | [main]<br>[BACK](-b-a-c-k/index.md)<br>Back face culling. Only front faces are visible. |
| [FRONT_AND_BACK](-f-r-o-n-t_-a-n-d_-b-a-c-k/index.md) | [main]<br>[FRONT_AND_BACK](-f-r-o-n-t_-a-n-d_-b-a-c-k/index.md)<br>Front and back culling. Geometry is not visible. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.CullingMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.CullingMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
