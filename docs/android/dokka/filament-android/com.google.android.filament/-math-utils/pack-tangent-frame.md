//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MathUtils](index.md)/[packTangentFrame](pack-tangent-frame.md)

# packTangentFrame

[main]\
open fun [packTangentFrame](pack-tangent-frame.md)(tangentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tangentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tangentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), quaternion: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Packs the tangent frame represented by the specified tangent, bitangent, and normal into a quaternion. 

 Reflection is preserved by encoding it as the sign of the w component in the resulting quaternion. Since -0 cannot always be represented on the GPU, this function computes a bias to ensure values are always either positive or negative, never 0. The bias is computed based on a per-element storage size of 2 bytes, making the resulting quaternion suitable for storage into an SNORM16 vector. 

#### Parameters

main

| | |
|---|---|
| tangentX | the X component of the tangent |
| tangentY | the Y component of the tangent |
| tangentZ | the Z component of the tangent |
| bitangentX | the X component of the bitangent |
| bitangentY | the Y component of the bitangent |
| bitangentZ | the Z component of the bitangent |
| normalX | the X component of the normal |
| normalY | the Y component of the normal |
| normalZ | the Z component of the normal |
| quaternion | a float array of at least size 4 for the quaternion result to be stored |

[main]\
open fun [packTangentFrame](pack-tangent-frame.md)(tangentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tangentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tangentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), bitangentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), normalZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), quaternion: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Packs the tangent frame represented by the specified tangent, bitangent, and normal into a quaternion. 

 Reflection is preserved by encoding it as the sign of the w component in the resulting quaternion. Since -0 cannot always be represented on the GPU, this function computes a bias to ensure values are always either positive or negative, never 0. The bias is computed based on a per-element storage size of 2 bytes, making the resulting quaternion suitable for storage into an SNORM16 vector. 

#### Parameters

main

| | |
|---|---|
| tangentX | the X component of the tangent |
| tangentY | the Y component of the tangent |
| tangentZ | the Z component of the tangent |
| bitangentX | the X component of the bitangent |
| bitangentY | the Y component of the bitangent |
| bitangentZ | the Z component of the bitangent |
| normalX | the X component of the normal |
| normalY | the Y component of the normal |
| normalZ | the Z component of the normal |
| quaternion | a float array of at least size 4 for the quaternion result to be stored |
| offset | offset, in elements, into the quaternion array to store the results |
