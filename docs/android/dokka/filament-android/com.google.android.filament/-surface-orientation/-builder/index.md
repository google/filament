//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[SurfaceOrientation](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Constructs an immutable surface orientation helper. At a minimum, clients must supply a vertex count. They can supply data in any of the following combinations: 

1. normals only (not recommended)
2. normals + tangents (sign of W determines bitangent orientation)
3. normals + uvs + positions + indices
4. positions + indices

 Additionally, the client-side data has the following type constraints: 

1. Normals must be float3
2. Tangents must be float4
3. UVs must be float2
4. Positions must be float3
5. Triangles must be uint3 or ushort3

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(): [SurfaceOrientation](../index.md)<br>Consumes the input data, produces quaternions, and destroys the native builder. |
| [normals](normals.md) | [main]<br>open fun [normals](normals.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [positions](positions.md) | [main]<br>open fun [positions](positions.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [tangents](tangents.md) | [main]<br>open fun [tangents](tangents.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [triangleCount](triangle-count.md) | [main]<br>open fun [triangleCount](triangle-count.md)(triangleCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [SurfaceOrientation.Builder](index.md) |
| [triangles_uint16](triangles_uint16.md) | [main]<br>open fun [triangles_uint16](triangles_uint16.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [triangles_uint32](triangles_uint32.md) | [main]<br>open fun [triangles_uint32](triangles_uint32.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [uvs](uvs.md) | [main]<br>open fun [uvs](uvs.md)(buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html)): [SurfaceOrientation.Builder](index.md) |
| [vertexCount](vertex-count.md) | [main]<br>open fun [vertexCount](vertex-count.md)(vertexCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [SurfaceOrientation.Builder](index.md) |
