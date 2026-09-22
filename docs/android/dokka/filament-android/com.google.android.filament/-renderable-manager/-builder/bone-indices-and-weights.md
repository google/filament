//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[boneIndicesAndWeights](bone-indices-and-weights.md)

# boneIndicesAndWeights

[main]\
open fun [boneIndicesAndWeights](bone-indices-and-weights.md)(primitiveIndex: Int, indicesAndWeights: Array&lt;Float&gt;, count: Int, bonesPerVertex: Int): [RenderableManager.Builder](index.md)

Define bone indices and weights &quot;pairs&quot; for vertex skinning as a float2. 

The unsigned int(pair.x) defines index of the bone and pair.y is the bone weight. The pairs substitute \c BONE_INDICES and the \c BONE_WEIGHTS defined in the VertexBuffer. Both ways of indices and weights definition must not be combined in one primitive. Number of pairs per vertex bonesPerVertex is not limited to 4 bones. Vertex buffer used for \c primitiveIndex must be set for advance skinning. All bone weights of one vertex should sum to one. Otherwise they will be normalized. Data must be rectangular and number of bone pairs must be same for all vertices of this primitive. The data is arranged sequentially, all bone pairs for the first vertex, then for the second vertex, and so on.

#### Return

Builder reference for chaining calls.

#### Parameters

main

| | |
|---|---|
| primitiveIndex | zero-based index of the primitive, must be less than the primitive count passed to Builder constructor |
| indicesAndWeights | pairs of bone index and bone weight for all vertices sequentially |
| count | number of all pairs, must be a multiple of vertexCount of the primitive count = vertexCount * bonesPerVertex |
| bonesPerVertex | number of bone pairs, same for all vertices of the primitive |

#### See also

| |
|---|
| [VertexBuffer.Builder](../../-vertex-buffer/-builder/advanced-skinning.md) |
