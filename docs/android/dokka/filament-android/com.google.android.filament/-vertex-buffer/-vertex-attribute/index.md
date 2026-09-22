//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[VertexAttribute](index.md)

# VertexAttribute

[main]\
enum [VertexAttribute](index.md)

Vertex attribute types

## Entries

| | |
|---|---|
| [POSITION](-p-o-s-i-t-i-o-n/index.md) | [main]<br>[POSITION](-p-o-s-i-t-i-o-n/index.md)<br>XYZ position (float3) |
| [TANGENTS](-t-a-n-g-e-n-t-s/index.md) | [main]<br>[TANGENTS](-t-a-n-g-e-n-t-s/index.md)<br>tangent, bitangent and normal, encoded as a quaternion (float4) |
| [COLOR](-c-o-l-o-r/index.md) | [main]<br>[COLOR](-c-o-l-o-r/index.md)<br>vertex color (float4) |
| [UV0](-u-v0/index.md) | [main]<br>[UV0](-u-v0/index.md)<br>texture coordinates (float2) |
| [UV1](-u-v1/index.md) | [main]<br>[UV1](-u-v1/index.md)<br>texture coordinates (float2) |
| [BONE_INDICES](-b-o-n-e_-i-n-d-i-c-e-s/index.md) | [main]<br>[BONE_INDICES](-b-o-n-e_-i-n-d-i-c-e-s/index.md)<br>indices of 4 bones, as unsigned integers (uvec4) |
| [BONE_WEIGHTS](-b-o-n-e_-w-e-i-g-h-t-s/index.md) | [main]<br>[BONE_WEIGHTS](-b-o-n-e_-w-e-i-g-h-t-s/index.md)<br>weights of the 4 bones (normalized float4) |
| [CUSTOM0](-c-u-s-t-o-m0/index.md) | [main]<br>[CUSTOM0](-c-u-s-t-o-m0/index.md) |
| [CUSTOM1](-c-u-s-t-o-m1/index.md) | [main]<br>[CUSTOM1](-c-u-s-t-o-m1/index.md) |
| [CUSTOM2](-c-u-s-t-o-m2/index.md) | [main]<br>[CUSTOM2](-c-u-s-t-o-m2/index.md) |
| [CUSTOM3](-c-u-s-t-o-m3/index.md) | [main]<br>[CUSTOM3](-c-u-s-t-o-m3/index.md) |
| [CUSTOM4](-c-u-s-t-o-m4/index.md) | [main]<br>[CUSTOM4](-c-u-s-t-o-m4/index.md) |
| [CUSTOM5](-c-u-s-t-o-m5/index.md) | [main]<br>[CUSTOM5](-c-u-s-t-o-m5/index.md) |
| [CUSTOM6](-c-u-s-t-o-m6/index.md) | [main]<br>[CUSTOM6](-c-u-s-t-o-m6/index.md) |
| [CUSTOM7](-c-u-s-t-o-m7/index.md) | [main]<br>[CUSTOM7](-c-u-s-t-o-m7/index.md) |
| [MORPH_POSITION_0](-m-o-r-p-h_-p-o-s-i-t-i-o-n_0/index.md) | [main]<br>[MORPH_POSITION_0](-m-o-r-p-h_-p-o-s-i-t-i-o-n_0/index.md) |
| [MORPH_POSITION_1](-m-o-r-p-h_-p-o-s-i-t-i-o-n_1/index.md) | [main]<br>[MORPH_POSITION_1](-m-o-r-p-h_-p-o-s-i-t-i-o-n_1/index.md) |
| [MORPH_POSITION_2](-m-o-r-p-h_-p-o-s-i-t-i-o-n_2/index.md) | [main]<br>[MORPH_POSITION_2](-m-o-r-p-h_-p-o-s-i-t-i-o-n_2/index.md) |
| [MORPH_POSITION_3](-m-o-r-p-h_-p-o-s-i-t-i-o-n_3/index.md) | [main]<br>[MORPH_POSITION_3](-m-o-r-p-h_-p-o-s-i-t-i-o-n_3/index.md) |
| [MORPH_TANGENTS_0](-m-o-r-p-h_-t-a-n-g-e-n-t-s_0/index.md) | [main]<br>[MORPH_TANGENTS_0](-m-o-r-p-h_-t-a-n-g-e-n-t-s_0/index.md) |
| [MORPH_TANGENTS_1](-m-o-r-p-h_-t-a-n-g-e-n-t-s_1/index.md) | [main]<br>[MORPH_TANGENTS_1](-m-o-r-p-h_-t-a-n-g-e-n-t-s_1/index.md) |
| [MORPH_TANGENTS_2](-m-o-r-p-h_-t-a-n-g-e-n-t-s_2/index.md) | [main]<br>[MORPH_TANGENTS_2](-m-o-r-p-h_-t-a-n-g-e-n-t-s_2/index.md) |
| [MORPH_TANGENTS_3](-m-o-r-p-h_-t-a-n-g-e-n-t-s_3/index.md) | [main]<br>[MORPH_TANGENTS_3](-m-o-r-p-h_-t-a-n-g-e-n-t-s_3/index.md) |

## Functions

| Name | Summary |
|---|---|
| [from](from.md) | [main]<br>open fun [from](from.md)(value: Int): [VertexBuffer.VertexAttribute](index.md) |
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [VertexBuffer.VertexAttribute](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[VertexBuffer.VertexAttribute](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
