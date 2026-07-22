//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowOptions](index.md)/[cascadeSplitPositions](cascade-split-positions.md)

# cascadeSplitPositions

[main]\
open var [cascadeSplitPositions](cascade-split-positions.md): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

The split positions for shadow cascades. 

 Cascaded shadow mapping (CSM) partitions the camera frustum into cascades. These values determine the planes along the camera's Z axis to split the frustum. The camera near plane is represented by 0.0f and the far plane represented by 1.0f. 

 For example, if using 4 cascades, these values would set a uniform split scheme: { 0.25f, 0.50f, 0.75f } 

 For N cascades, N - 1 split positions will be read from this array. 

 Filament provides utility methods inside [ShadowCascades](../-shadow-cascades/index.md) to help set these values. For example, to use a uniform split scheme: 

```kotlin
LightManager.ShadowCascades.computeUniformSplits(options.cascadeSplitPositions, 4);

```

#### See also

| |
|---|
| [LightManager.ShadowCascades](../-shadow-cascades/compute-practical-splits.md) |
