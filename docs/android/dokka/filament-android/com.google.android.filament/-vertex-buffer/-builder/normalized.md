//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[normalized](normalized.md)

# normalized

[main]\
open fun [normalized](normalized.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md)): [VertexBuffer.Builder](index.md)

Sets whether a given attribute should be normalized. By default attributes are not normalized. A normalized attribute is mapped between 0 and 1 in the shader. This applies only to integer types.

#### Return

this `Builder` object for chaining calls. This is a no-op if the `attribute` is an invalid enum.

#### Parameters

main

| | |
|---|---|
| attribute | enum of the attribute to set the normalization flag to |

[main]\
open fun [normalized](normalized.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [VertexBuffer.Builder](index.md)

Sets whether a given attribute should be normalized. By default attributes are not normalized. A normalized attribute is mapped between 0 and 1 in the shader. This applies only to integer types.

#### Return

this `Builder` object for chaining calls. This is a no-op if the `attribute` is an invalid enum.

#### Parameters

main

| | |
|---|---|
| attribute | enum of the attribute to set the normalization flag to |
| enabled | true to automatically normalize the given attribute |
