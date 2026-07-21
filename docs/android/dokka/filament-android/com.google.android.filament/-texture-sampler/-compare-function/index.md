//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[TextureSampler](../index.md)/[CompareFunction](index.md)

# CompareFunction

[main]\
enum [CompareFunction](index.md)

Comparison functions for the depth sampler.

## Entries

| | |
|---|---|
| [LESS_EQUAL](-l-e-s-s_-e-q-u-a-l/index.md) | [main]<br>[LESS_EQUAL](-l-e-s-s_-e-q-u-a-l/index.md)<br>Less or equal |
| [GREATER_EQUAL](-g-r-e-a-t-e-r_-e-q-u-a-l/index.md) | [main]<br>[GREATER_EQUAL](-g-r-e-a-t-e-r_-e-q-u-a-l/index.md)<br>Greater or equal |
| [LESS](-l-e-s-s/index.md) | [main]<br>[LESS](-l-e-s-s/index.md)<br>Strictly less than |
| [GREATER](-g-r-e-a-t-e-r/index.md) | [main]<br>[GREATER](-g-r-e-a-t-e-r/index.md)<br>Strictly greater than |
| [EQUAL](-e-q-u-a-l/index.md) | [main]<br>[EQUAL](-e-q-u-a-l/index.md)<br>Equal |
| [NOT_EQUAL](-n-o-t_-e-q-u-a-l/index.md) | [main]<br>[NOT_EQUAL](-n-o-t_-e-q-u-a-l/index.md)<br>Not equal |
| [ALWAYS](-a-l-w-a-y-s/index.md) | [main]<br>[ALWAYS](-a-l-w-a-y-s/index.md)<br>Always. Depth testing is deactivated. |
| [NEVER](-n-e-v-e-r/index.md) | [main]<br>[NEVER](-n-e-v-e-r/index.md)<br>Never. The depth test always fails. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [TextureSampler.CompareFunction](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[TextureSampler.CompareFunction](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
