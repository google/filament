//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setSkinningBuffer](set-skinning-buffer.md)

# setSkinningBuffer

[main]\
open fun [setSkinningBuffer](set-skinning-buffer.md)(instance: Int, skinningBuffer: [SkinningBuffer](../-skinning-buffer/index.md), count: Int, offset: Int)

Associates a region of a SkinningBuffer to a renderable instance 

Note: due to hardware limitations offset + 256 must be smaller or equal to skinningBuffer->getBoneCount()

#### Parameters

main

| | |
|---|---|
| instance | Instance of the component obtained from getInstance(). |
| skinningBuffer | skinning buffer to associate to the instance |
| count | Size of the region in bones, must be smaller or equal to 256. |
| offset | Start offset of the region in bones |
