//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[MorphTargetBuffer](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [MorphTargetBuffer](../index.md)<br>Creates the MorphTargetBuffer object and returns a pointer to it. |
| [count](count.md) | [main]<br>open fun [count](count.md)(count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Size of the morph targets in targets. |
| [enableCustomMorphing](enable-custom-morphing.md) | [main]<br>open fun [enableCustomMorphing](enable-custom-morphing.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Enables the custom morphing pipeline. |
| [name](name.md) | [main]<br>open fun [name](name.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [MorphTargetBuffer.Builder](index.md)<br>Associate an optional name with this MorphTargetBuffer for debugging purposes. |
| [vertexCount](vertex-count.md) | [main]<br>open fun [vertexCount](vertex-count.md)(vertexCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Size of the morph targets in vertex counts. |
| [withPositions](with-positions.md) | [main]<br>open fun [withPositions](with-positions.md)(): [MorphTargetBuffer.Builder](index.md)<br>open fun [withPositions](with-positions.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Enables and allocates the built-in buffer for position morphing. |
| [withTangents](with-tangents.md) | [main]<br>open fun [withTangents](with-tangents.md)(): [MorphTargetBuffer.Builder](index.md)<br>open fun [withTangents](with-tangents.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Enables and allocates the built-in buffer for tangent/normal morphing. |
