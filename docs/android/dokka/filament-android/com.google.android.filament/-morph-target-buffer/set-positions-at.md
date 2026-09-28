//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MorphTargetBuffer](index.md)/[setPositionsAt](set-positions-at.md)

# setPositionsAt

[main]\
open fun [setPositionsAt](set-positions-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), positions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates positions for the given morph target. 

This method can only be called if the MorphTargetBuffer was built with `withPositions(true)`. This is equivalent to the float4 method, but uses 1.0 for the 4th component.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this MorphTargetBuffer. |
| targetIndex | the index of morph target to be updated. |
| positions | pointer to at least &quot;count&quot; positions |
| count | number of float3 vectors in positions |

[main]\
open fun [setPositionsAt](set-positions-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), positions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates positions for the given morph target. 

This method can only be called if the MorphTargetBuffer was built with `withPositions(true)`. This is equivalent to the float4 method, but uses 1.0 for the 4th component.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this MorphTargetBuffer. |
| targetIndex | the index of morph target to be updated. |
| positions | pointer to at least &quot;count&quot; positions |
| count | number of float3 vectors in positions |
| offset | offset into the target buffer, expressed as a number of float3 vectors |
