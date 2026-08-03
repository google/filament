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
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [MorphTargetBuffer](../index.md)<br>Creates and returns the `MorphTargetBuffer` object. |
| [count](count.md) | [main]<br>open fun [count](count.md)(count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Size of the morph targets in targets. |
| [enableCustomMorphing](enable-custom-morphing.md) | [main]<br>open fun [enableCustomMorphing](enable-custom-morphing.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Use this method to enable or disable custom morphing. |
| [vertexCount](vertex-count.md) | [main]<br>open fun [vertexCount](vertex-count.md)(vertexCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Size of the morph targets in vertex counts. |
| [withPositions](with-positions.md) | [main]<br>open fun [withPositions](with-positions.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Use this method to enable or disable the built-in position morphing buffer. |
| [withTangents](with-tangents.md) | [main]<br>open fun [withTangents](with-tangents.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [MorphTargetBuffer.Builder](index.md)<br>Use this method to enable or disable the built-in tangent morphing buffer. |
