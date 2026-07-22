//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[morphing](morphing.md)

# morphing

[main]\
open fun [morphing](morphing.md)(targetCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Controls if the renderable has legacy vertex morphing targets, zero by default. For legacy morphing, the attached [VertexBuffer](../../-vertex-buffer/index.md) must provide data in the appropriate [VertexBuffer.VertexAttribute](../../-vertex-buffer/-vertex-attribute/index.md) slots (`MORPH_POSITION_0` etc). Legacy morphing only supports up to 4 morph targets and will be deprecated in the future. Legacy morphing must be enabled on the material definition: either via the `legacyMorphing` material attribute or by calling ::useLegacyMorphing. 

See also [setMorphWeights](../set-morph-weights.md), which can be called on a per-frame basis to advance the animation.

[main]\
open fun [morphing](morphing.md)(morphTargetBuffer: [MorphTargetBuffer](../../-morph-target-buffer/index.md)): [RenderableManager.Builder](index.md)

Controls if the renderable has vertex morphing targets, zero by default. 

For standard morphing, A [MorphTargetBuffer](../../-morph-target-buffer/index.md) must be provided. Standard morphing supports up to `CONFIG_MAX_MORPH_TARGET_COUNT` morph targets.

See also [setMorphWeights](../set-morph-weights.md), which can be called on a per-frame basis to advance the animation.

[main]\
open fun [morphing](morphing.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the morph target buffer for a primitive. The morph target buffer must have an associated renderable and geometry. Two conditions must be met: 1. The number of morph targets in the buffer must equal the renderable's morph target count. 2. The vertex count of each morph target must equal the geometry's vertex count.

#### Parameters

main

| | |
|---|---|
| level | the level of detail (lod), only 0 can be specified |
| primitiveIndex | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| offset | specifies where in the morph target buffer to start reading (expressed as a number of vertices) |
