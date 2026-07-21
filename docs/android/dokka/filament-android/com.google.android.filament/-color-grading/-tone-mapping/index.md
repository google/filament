//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[ToneMapping](index.md)

# ToneMapping

[main]\
enum [~~ToneMapping~~](index.md)---

### Deprecated

---

List of available tone-mapping operators.

#### Deprecated

Use [toneMapper](../-builder/tone-mapper.md)

## Entries

| | |
|---|---|
| [LINEAR](-l-i-n-e-a-r/index.md) | [main]<br>[~~LINEAR~~](-l-i-n-e-a-r/index.md)<br>Linear tone mapping (i.e. no tone mapping). |
| [ACES_LEGACY](-a-c-e-s_-l-e-g-a-c-y/index.md) | [main]<br>[~~ACES_LEGACY~~](-a-c-e-s_-l-e-g-a-c-y/index.md)<br>ACES tone mapping, with a brightness modifier to match Filament's legacy tone mapper. |
| [ACES](-a-c-e-s/index.md) | [main]<br>[~~ACES~~](-a-c-e-s/index.md)<br>ACES tone mapping. |
| [FILMIC](-f-i-l-m-i-c/index.md) | [main]<br>[~~FILMIC~~](-f-i-l-m-i-c/index.md)<br>Filmic tone mapping, modelled after ACES but applied in sRGB space. |
| [DISPLAY_RANGE](-d-i-s-p-l-a-y_-r-a-n-g-e/index.md) | [main]<br>[~~DISPLAY_RANGE~~](-d-i-s-p-l-a-y_-r-a-n-g-e/index.md)<br>Tone mapping used to validate/debug scene exposure. |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [ColorGrading.ToneMapping](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[ColorGrading.ToneMapping](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
