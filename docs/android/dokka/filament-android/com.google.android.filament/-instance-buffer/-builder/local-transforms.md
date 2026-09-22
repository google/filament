//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[InstanceBuffer](../index.md)/[Builder](index.md)/[localTransforms](local-transforms.md)

# localTransforms

[main]\
open fun [localTransforms](local-transforms.md)(localTransforms: Array&lt;Float&gt;): [InstanceBuffer.Builder](index.md)

Provide an initial local transform for each instance. Each local transform is relative to 

the transform of the associated renderable. This forms a parent-child relationship between the renderable and its instances, so adjusting the renderable's transform will

- * affect all instances. The array of math::mat4f must have length instanceCount, provided when constructing this Builder.

#### Parameters

main

| | |
|---|---|
| localTransforms | an array of math::mat4f with length instanceCount, must remain valid until after build() is called |
