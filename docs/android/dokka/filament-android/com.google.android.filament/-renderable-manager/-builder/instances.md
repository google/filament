//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[instances](instances.md)

# instances

[main]\
open fun [instances](instances.md)(instanceCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Specifies the number of draw instance of this renderable. The default is 1 instance and the maximum number of instances allowed is 32767. 0 is invalid. All instances are culled using the same bounding box, so care must be taken to make sure all instances render inside the specified bounding box. The material can use getInstanceIndex() in the vertex shader to get the instance index and possibly adjust the position or transform.

#### Parameters

main

| | |
|---|---|
| instanceCount | the number of instances silently clamped between 1 and 32767. |
