//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[TransparencyMode](index.md)

# TransparencyMode

enum [TransparencyMode](index.md)

How transparent objects are handled

#### See also

| | |
|---|---|
| &lt;a href=&quot;https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:transparencymode&quot;&gt; | Blending and transparency: transparencyMode |

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md)<br>The transparent object is drawn honoring the raster state. |
| [TWO_PASSES_ONE_SIDE](-t-w-o_-p-a-s-s-e-s_-o-n-e_-s-i-d-e/index.md) | [main]<br>[TWO_PASSES_ONE_SIDE](-t-w-o_-p-a-s-s-e-s_-o-n-e_-s-i-d-e/index.md)<br>The transparent object is first drawn in the depth buffer, then in the color buffer, honoring the culling mode, but ignoring the depth test function. |
| [TWO_PASSES_TWO_SIDES](-t-w-o_-p-a-s-s-e-s_-t-w-o_-s-i-d-e-s/index.md) | [main]<br>[TWO_PASSES_TWO_SIDES](-t-w-o_-p-a-s-s-e-s_-t-w-o_-s-i-d-e-s/index.md)<br>The transparent object is drawn twice in the color buffer, first with back faces only, then with front faces; the culling mode is ignored. Can be combined with two-sided lighting. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.TransparencyMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.TransparencyMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
