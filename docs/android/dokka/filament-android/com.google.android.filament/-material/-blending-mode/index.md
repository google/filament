//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Material](../index.md)/[BlendingMode](index.md)

# BlendingMode

enum [BlendingMode](index.md)

Supported blending modes

#### See also

| | |
|---|---|
| &lt;a href=&quot;https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:blending&quot;&gt; | Blending and transparency: blending |

## Entries

| | |
|---|---|
| [OPAQUE](-o-p-a-q-u-e/index.md) | [main]<br>[OPAQUE](-o-p-a-q-u-e/index.md)<br>Material is opaque. |
| [TRANSPARENT](-t-r-a-n-s-p-a-r-e-n-t/index.md) | [main]<br>[TRANSPARENT](-t-r-a-n-s-p-a-r-e-n-t/index.md)<br>Material is transparent and color is alpha-pre-multiplied. Affects diffuse lighting only. |
| [ADD](-a-d-d/index.md) | [main]<br>[ADD](-a-d-d/index.md)<br>Material is additive (e.g.: hologram). |
| [MASKED](-m-a-s-k-e-d/index.md) | [main]<br>[MASKED](-m-a-s-k-e-d/index.md)<br>Material is masked (i.e. alpha tested). |
| [FADE](-f-a-d-e/index.md) | [main]<br>[FADE](-f-a-d-e/index.md)<br>Material is transparent and color is alpha-pre-multiplied. Affects specular lighting. |
| [MULTIPLY](-m-u-l-t-i-p-l-y/index.md) | [main]<br>[MULTIPLY](-m-u-l-t-i-p-l-y/index.md)<br>Material darkens what's behind it. |
| [SCREEN](-s-c-r-e-e-n/index.md) | [main]<br>[SCREEN](-s-c-r-e-e-n/index.md)<br>Material brightens what's behind it. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Material.BlendingMode](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Material.BlendingMode](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
