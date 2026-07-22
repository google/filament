//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[ShadowType](index.md)

# ShadowType

enum [ShadowType](index.md)

List of available shadow mapping techniques.

#### See also

| |
|---|
| [setShadowType](../set-shadow-type.md) |

## Entries

| | |
|---|---|
| [PCF](-p-c-f/index.md) | [main]<br>[PCF](-p-c-f/index.md)<br>percentage-closer filtered shadows (default) |
| [VSM](-v-s-m/index.md) | [main]<br>[VSM](-v-s-m/index.md)<br>exponential variance shadows (EVSM) |
| [DPCF](-d-p-c-f/index.md) | [main]<br>[DPCF](-d-p-c-f/index.md) |
| [PCSS](-p-c-s-s/index.md) | [main]<br>[PCSS](-p-c-s-s/index.md)<br>EVSM with soft shadows and contact hardening |
| [PCFd](-p-c-fd/index.md) | [main]<br>[PCFd](-p-c-fd/index.md) |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [View.ShadowType](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[View.ShadowType](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
