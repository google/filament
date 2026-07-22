//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[geometryType](geometry-type.md)

# geometryType

[main]\
open fun [geometryType](geometry-type.md)(type: [RenderableManager.Builder.GeometryType](-geometry-type/index.md)): [RenderableManager.Builder](index.md)

Specify whether this renderable has static bounds. In this context his means that the renderable's bounding box cannot change and that the renderable's transform is assumed immutable. Changing the renderable's transform via the TransformManager can lead to corrupted graphics. Note that skinning and morphing are not forbidden. Disabled by default.

#### Parameters

main

| | |
|---|---|
| enable | whether this renderable has static bounds. false by default. |
