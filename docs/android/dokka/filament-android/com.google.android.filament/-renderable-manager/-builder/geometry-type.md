//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[geometryType](geometry-type.md)

# geometryType

[main]\
open fun [geometryType](geometry-type.md)(type: [RenderableManager.Builder.GeometryType](-geometry-type/index.md)): [RenderableManager.Builder](index.md)

Specify the type of geometry for this renderable. 

DYNAMIC geometry has no restriction, STATIC_BOUNDS geometry means that both the bounds and the world-space transform of the renderable are immutable. STATIC geometry has the same restrictions as STATIC_BOUNDS, but in addition disallows skinning, morphing and changing the VertexBuffer or IndexBuffer in any way.

#### Parameters

main

| | |
|---|---|
| type | type of geometry. |
