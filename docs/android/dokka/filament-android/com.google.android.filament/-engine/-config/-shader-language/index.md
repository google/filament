//[filament-android](../../../../../index.md)/[com.google.android.filament](../../../index.md)/[Engine](../../index.md)/[Config](../index.md)/[ShaderLanguage](index.md)

# ShaderLanguage

[main]\
enum [ShaderLanguage](index.md)

Sets a preferred shader language for Filament to use. The Metal backend supports two shader languages: MSL (Metal Shading Language) and METAL_LIBRARY (precompiled .metallib). This option controls which shader language is used when materials contain both. By default, when preferredShaderLanguage is unset, Filament will prefer METAL_LIBRARY shaders if present within a material, falling back to MSL. Setting preferredShaderLanguage to ShaderLanguage::MSL will instead instruct Filament to check for the presence of MSL in a material first, falling back to METAL_LIBRARY if MSL is not present. When using a non-Metal backend, setting this has no effect.

## Entries

| | |
|---|---|
| [DEFAULT](-d-e-f-a-u-l-t/index.md) | [main]<br>[DEFAULT](-d-e-f-a-u-l-t/index.md) |
| [MSL](-m-s-l/index.md) | [main]<br>[MSL](-m-s-l/index.md) |
| [METAL_LIBRARY](-m-e-t-a-l_-l-i-b-r-a-r-y/index.md) | [main]<br>[METAL_LIBRARY](-m-e-t-a-l_-l-i-b-r-a-r-y/index.md) |

## Functions

| Name | Summary |
|---|---|
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [Engine.Config.ShaderLanguage](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Engine.Config.ShaderLanguage](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
