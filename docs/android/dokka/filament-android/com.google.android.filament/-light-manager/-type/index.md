//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Type](index.md)

# Type

[main]\
enum [Type](index.md)

Denotes the type of the light being created.

## Entries

| | |
|---|---|
| [SUN](-s-u-n/index.md) | [main]<br>[SUN](-s-u-n/index.md)<br>Directional light that also draws a sun's disk in the sky. |
| [DIRECTIONAL](-d-i-r-e-c-t-i-o-n-a-l/index.md) | [main]<br>[DIRECTIONAL](-d-i-r-e-c-t-i-o-n-a-l/index.md)<br>Directional light, emits light in a given direction. |
| [POINT](-p-o-i-n-t/index.md) | [main]<br>[POINT](-p-o-i-n-t/index.md)<br>Point light, emits light from a position, in all directions. |
| [FOCUSED_SPOT](-f-o-c-u-s-e-d_-s-p-o-t/index.md) | [main]<br>[FOCUSED_SPOT](-f-o-c-u-s-e-d_-s-p-o-t/index.md)<br>Physically correct spot light. |
| [SPOT](-s-p-o-t/index.md) | [main]<br>[SPOT](-s-p-o-t/index.md)<br>Spot light with coupling of outer cone and illumination disabled. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [LightManager.Type](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[LightManager.Type](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
