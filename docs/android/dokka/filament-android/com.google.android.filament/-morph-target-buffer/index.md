//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MorphTargetBuffer](index.md)

# MorphTargetBuffer

open class [MorphTargetBuffer](index.md)

A container for vertex morphing data that supports both automatic and manual morphing. 

MorphTargetBuffer operates in a hybrid model depending on the attribute being morphed:

1. Automatic for Built-ins (positions/tangents): Enable via `withPositions(true)` or `withTangents(true)`. The MorphTargetBuffer will allocate internal storage and hold the data for these attributes, which you upload via `setPositionsAt()` or `setTangentsAt()`. The framework automatically applies the morphing logic in the vertex shader.
2. Manual for Custom Data (e.g., UVs, colors): The MorphTargetBuffer does NOT hold data for custom targets. The user is responsible for the full data pipeline:

- Create and manage a separate `Texture` to hold the morph target data (offsets).
- In the material, declare a `sampler2d_array` parameter.
- Bind the `Texture` to the material instance.
- In the vertex shader, manually call `morphData2`, `morphData3`, or `morphData4` with the custom sampler to apply the morphing. A MorphTargetBuffer object must be associated with a Renderable via `RenderableManager::Builder::morphing()` to enable the morphing pipeline for all cases.

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/index.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getCount](get-count.md) | [main]<br>open fun [getCount](get-count.md)(): Int<br>Returns the target count of this MorphTargetBuffer. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getVertexCount](get-vertex-count.md) | [main]<br>open fun [getVertexCount](get-vertex-count.md)(): Int<br>Returns the vertex count of this MorphTargetBuffer. |
| [hasPositions](has-positions.md) | [main]<br>open fun [hasPositions](has-positions.md)(): Boolean<br>Returns true if this MorphTargetBuffer has a position buffer. |
| [hasTangents](has-tangents.md) | [main]<br>open fun [hasTangents](has-tangents.md)(): Boolean<br>Returns true if this MorphTargetBuffer has a tangent buffer. |
| [isCustomMorphingEnabled](is-custom-morphing-enabled.md) | [main]<br>open fun [isCustomMorphingEnabled](is-custom-morphing-enabled.md)(): Boolean<br>Returns true if custom morphing is enabled |
| [setPositionsAt](set-positions-at.md) | [main]<br>open fun [setPositionsAt](set-positions-at.md)(engine: [Engine](../-engine/index.md), targetIndex: Int, positions: Array&lt;Float&gt;, count: Int)<br>open fun [setPositionsAt](set-positions-at.md)(engine: [Engine](../-engine/index.md), targetIndex: Int, positions: Array&lt;Float&gt;, count: Int, offset: Int)<br>Updates positions for the given morph target. |
| [setTangentsAt](set-tangents-at.md) | [main]<br>open fun [setTangentsAt](set-tangents-at.md)(engine: [Engine](../-engine/index.md), targetIndex: Int, tangents: Array&lt;Short&gt;, count: Int)<br>open fun [setTangentsAt](set-tangents-at.md)(engine: [Engine](../-engine/index.md), targetIndex: Int, tangents: Array&lt;Short&gt;, count: Int, offset: Int)<br>Updates tangents for the given morph target. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [MorphTargetBuffer](index.md) |
