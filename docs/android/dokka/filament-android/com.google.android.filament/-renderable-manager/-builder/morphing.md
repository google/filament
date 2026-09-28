//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[morphing](morphing.md)

# morphing

[main]\
open fun [morphing](morphing.md)(targetCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Controls if the renderable has legacy vertex morphing targets, zero by default. 

This is required to enable GPU morphing.

For legacy morphing, the attached VertexBuffer must provide data in the appropriate VertexAttribute slots (\c MORPH_POSITION_0 etc). Legacy morphing only supports up to 4 morph targets and will be deprecated in the future. Legacy morphing must be enabled on the material definition: either via the legacyMorphing material attribute or by calling filamat::MaterialBuilder::useLegacyMorphing().

See also RenderableManager::setMorphWeights(), which can be called on a per-frame basis to advance the animation.

[main]\
open fun [morphing](morphing.md)(morphTargetBuffer: [MorphTargetBuffer](../../-morph-target-buffer/index.md)): [RenderableManager.Builder](index.md)

Controls if the renderable has vertex morphing targets, zero by default. 

This is required to enable GPU morphing.

Filament supports two morphing modes: standard (default) and legacy.

For standard morphing, A MorphTargetBuffer must be provided. Standard morphing supports up to \c CONFIG_MAX_MORPH_TARGET_COUNT morph targets.

See also RenderableManager::setMorphWeights(), which can be called on a per-frame basis to advance the animation.

[main]\
open fun [morphing](morphing.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the the range of the MorphTargetBuffer to use with this primitive.

#### Parameters

main

| | |
|---|---|
| level | the level of detail (lod), only 0 can be specified |
| primitiveIndex | zero-based index of the primitive, must be less than the count passed to Builder constructor |
| offset | specifies where in the morph target buffer to start reading (expressed as a number of vertices) |
