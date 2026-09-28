//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)/[attribute](attribute.md)

# attribute

[main]\
open fun [attribute](attribute.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), attributeType: [VertexBuffer.AttributeType](../-attribute-type/index.md)): [VertexBuffer.Builder](index.md)

Sets up an attribute for this vertex buffer set. 

Using `byteOffset` and `byteStride`, attributes can be interleaved in the same buffer.

VertexAttribute::TANGENTS must be specified as a quaternion and is how normals are specified.

Not all backends support 3-component attributes that are not floats. For help with conversion, see geometry::Transcoder.

#### Return

A reference to this Builder for chaining calls.

This is a no-op if the `attribute` is an invalid enum. This is a no-op if the `bufferIndex` is out of bounds.

#### Parameters

main

| | |
|---|---|
| attribute | The attribute to set up. |
| bufferIndex | The index of the buffer containing the data for this attribute. Must be between 0 and bufferCount() - 1. |
| attributeType | The type of the attribute data (e.g. byte, float3, etc...) |

#### See also

| |
|---|
| [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md) |

[main]\
open fun [attribute](attribute.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), attributeType: [VertexBuffer.AttributeType](../-attribute-type/index.md), byteOffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)

Sets up an attribute for this vertex buffer set. 

Using `byteOffset` and `byteStride`, attributes can be interleaved in the same buffer.

VertexAttribute::TANGENTS must be specified as a quaternion and is how normals are specified.

Not all backends support 3-component attributes that are not floats. For help with conversion, see geometry::Transcoder.

#### Return

A reference to this Builder for chaining calls.

This is a no-op if the `attribute` is an invalid enum. This is a no-op if the `bufferIndex` is out of bounds.

#### Parameters

main

| | |
|---|---|
| attribute | The attribute to set up. |
| bufferIndex | The index of the buffer containing the data for this attribute. Must be between 0 and bufferCount() - 1. |
| attributeType | The type of the attribute data (e.g. byte, float3, etc...) |
| byteOffset | Offset in *bytes* into the buffer `bufferIndex` |

#### See also

| |
|---|
| [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md) |

[main]\
open fun [attribute](attribute.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), attributeType: [VertexBuffer.AttributeType](../-attribute-type/index.md), byteOffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), byteStride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)

Sets up an attribute for this vertex buffer set. 

Using `byteOffset` and `byteStride`, attributes can be interleaved in the same buffer.

VertexAttribute::TANGENTS must be specified as a quaternion and is how normals are specified.

Not all backends support 3-component attributes that are not floats. For help with conversion, see geometry::Transcoder.

#### Return

A reference to this Builder for chaining calls.

This is a no-op if the `attribute` is an invalid enum. This is a no-op if the `bufferIndex` is out of bounds.

#### Parameters

main

| | |
|---|---|
| attribute | The attribute to set up. |
| bufferIndex | The index of the buffer containing the data for this attribute. Must be between 0 and bufferCount() - 1. |
| attributeType | The type of the attribute data (e.g. byte, float3, etc...) |
| byteOffset | Offset in *bytes* into the buffer `bufferIndex` |
| byteStride | Stride in *bytes* to the next element of this attribute. When set to zero the attribute size, as defined by `attributeType` is used. |

#### See also

| |
|---|
| [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md) |
