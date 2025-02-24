/*!
\brief Functionality to extract and work with the Bounding Boxes of PVRAssets (meshes etc.)
\file PVRAssets/BoundingBox.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <PVRAssets/Model.h>
#include <PVRCore/math/AxisAlignedBox.h>
namespace pvr {
namespace assets {
/// <summary>Contains utilities and helpers</summary>
namespace utils {

/// <summary>Return bounding box from vertex data.</summary>
/// <param name="data">Vertex data</param>
/// <param name="stride_bytes">Vertex stride in bytes</param>
/// <param name="offset_bytes">Offset to the vertex data</param>
/// <param name="size_bytes">Data size</param>
/// <returns>The Axis-aligned bounding box of the data</returns>
inline math::AxisAlignedBox getBoundingBox(const char* data, size_t stride_bytes, size_t offset_bytes, size_t size_bytes)
{
	math::AxisAlignedBox aabb;
	assertion(static_cast<bool>(data));
	assertion(stride_bytes >= 12 || !stride_bytes);
	assertion(size_bytes >= stride_bytes);
	if (size_bytes && data)
	{
		data = data + offset_bytes;
		glm::vec3 minvec;
		glm::vec3 maxvec;
		memcpy(&minvec, data, 12);
		memcpy(&maxvec, data, 12);

		for (size_t i = stride_bytes; i < size_bytes; i += stride_bytes)
		{
			glm::vec3 vec;
			memcpy(&vec, data + i, 12);
			minvec = glm::min(vec, minvec);
			maxvec = glm::max(vec, maxvec);
		}
		aabb.setMinMax(minvec, maxvec);
	}
	else
	{
		aabb.setMinMax(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
	}
	return aabb;
}

/// <summary>Return bounding box of a mesh.</summary>
/// <param name="mesh">A mesh from which to get the bounding box of</param>
/// <param name="positionSemanticName">Position attribute semantic name</param>
/// <returns>Axis-aligned bounding box</returns>
/// <remarks>It will be assumed that Vertex Position is a vec3.</remarks>
inline math::AxisAlignedBox getBoundingBox(const Mesh& mesh, const char* positionSemanticName)
{
	const Mesh::VertexAttributeData* vbo = mesh.getVertexAttributeByName(positionSemanticName);
	if (vbo)
	{
		return getBoundingBox(static_cast<const char*>(mesh.getData(vbo->getDataIndex())), mesh.getStride(vbo->getDataIndex()), vbo->getOffset(), mesh.getDataSize(vbo->getDataIndex()));
	}
	return math::AxisAlignedBox();
}

/// <summary>Return bounding box of a mesh.</summary>
/// <param name="mesh">A mesh from which to get the bounding box of</param>
/// <returns>Axis-aligned bounding box</returns>
/// <remarks>It will be assumed that Vertex Position is a vec3 and has the semantic "POSITION".</remarks>
inline math::AxisAlignedBox getBoundingBox(const Mesh& mesh) { return getBoundingBox(mesh, "POSITION"); }

/// <summary>Return bounding box of a model.</summary>
/// <param name="model">A model from which to get the bounding box of. All meshes will be considered.</param>
/// <returns>Axis-aligned bounding box</returns>
/// <remarks>It will be assumed that Vertex Position is a vec3 and has the semantic "POSITION".</remarks>
inline math::AxisAlignedBox getBoundingBox(const Model& model)
{
	if (model.getNumMeshes())
	{
		math::AxisAlignedBox retval(getBoundingBox(model.getMesh(0)));
		for (uint32_t i = 1; i < model.getNumMeshes(); ++i) { retval.mergeBox(getBoundingBox(model.getMesh(i))); }
		return retval;
	}
	else
	{
		return math::AxisAlignedBox();
	}
}

} // namespace utils
} // namespace assets
} // namespace pvr
