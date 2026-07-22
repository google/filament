//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[boundingBox](bounding-box.md)

# boundingBox

[main]\
open fun [boundingBox](bounding-box.md)(aabb: [Box](../../-box/index.md)): [RenderableManager.Builder](index.md)

The axis-aligned bounding box of the renderable. 

This is an object-space AABB used for frustum culling. For skinning and morphing, this should encompass all possible vertex positions. It is mandatory unless culling is disabled for the renderable.
