/*!
\brief Contains definitions for methods of the Volume class.
\file PVRAssets/Volume.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Volume.h"
#include "PVRAssets/Helper.h"

#include "PVRCore/Log.h"
using std::pair;
using std::map;

namespace pvr {
Volume::~Volume()
{
	delete[] _volumeMesh.vertices;
	delete[] _volumeMesh.edges;
	delete[] _volumeMesh.triangles;
	delete[] _volumeMesh.vertexData;
}

uint32_t Volume::findOrCreateVertex(const glm::vec3& vertex, bool& existed)
{
	// First check whether we already have a vertex here
	for (uint32_t i = 0; i < _volumeMesh.numVertices; ++i)
	{
		if (_volumeMesh.vertices[i].x == vertex.x && _volumeMesh.vertices[i].y == vertex.y && _volumeMesh.vertices[i].z == vertex.z)
		{
			// Don't do anything more if the vertex already exists
			existed = true;
			return i;
		}
	}

	if (_volumeMesh.numVertices == 0) { _volumeMesh.minimum = _volumeMesh.maximum = vertex; }
	else
	{
		if (vertex.x < _volumeMesh.minimum.x) { _volumeMesh.minimum.x = vertex.x; }

		if (vertex.y < _volumeMesh.minimum.y) { _volumeMesh.minimum.y = vertex.y; }

		if (vertex.z < _volumeMesh.minimum.z) { _volumeMesh.minimum.z = vertex.z; }

		if (vertex.x > _volumeMesh.maximum.x) { _volumeMesh.maximum.x = vertex.x; }

		if (vertex.y > _volumeMesh.maximum.y) { _volumeMesh.maximum.y = vertex.y; }

		if (vertex.z > _volumeMesh.maximum.z) { _volumeMesh.maximum.z = vertex.z; }
	}

	// Add the vertex
	memcpy(&_volumeMesh.vertices[_volumeMesh.numVertices], &vertex, sizeof(vertex));
	existed = false;
	return _volumeMesh.numVertices++;
}

uint32_t Volume::findOrCreateEdge(const glm::vec3& v0, const glm::vec3& v1, bool& existed)
{
	uint32_t vertexIndices[2];
	bool alreadyExisted[2];
	vertexIndices[0] = findOrCreateVertex(v0, alreadyExisted[0]);
	vertexIndices[1] = findOrCreateVertex(v1, alreadyExisted[1]);

	if (alreadyExisted[0] && alreadyExisted[1])
	{
		// Check whether we already have an edge here
		for (uint32_t i = 0; i < _volumeMesh.numEdges; ++i)
		{
			if ((_volumeMesh.edges[i].vertexIndices[0] == vertexIndices[0] && _volumeMesh.edges[i].vertexIndices[1] == vertexIndices[1]) ||
				(_volumeMesh.edges[i].vertexIndices[0] == vertexIndices[1] && _volumeMesh.edges[i].vertexIndices[1] == vertexIndices[0]))
			{
				// Don't do anything more if the edge already exists
				existed = true;
				return i;
			}
		}
	}

	// Add the edge
	_volumeMesh.edges[_volumeMesh.numEdges].vertexIndices[0] = vertexIndices[0];
	_volumeMesh.edges[_volumeMesh.numEdges].vertexIndices[1] = vertexIndices[1];
	existed = false;
	return _volumeMesh.numEdges++;
}

void Volume::findOrCreateTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
	VolumeEdge *edge0, *edge1, *edge2;
	uint32_t edgeIndex0, edgeIndex1, edgeIndex2;
	bool alreadyExisted[3];

	edgeIndex0 = findOrCreateEdge(v0, v1, alreadyExisted[0]);
	edgeIndex1 = findOrCreateEdge(v1, v2, alreadyExisted[1]);
	edgeIndex2 = findOrCreateEdge(v2, v0, alreadyExisted[2]);

	if (edgeIndex0 == edgeIndex1 || edgeIndex1 == edgeIndex2 || edgeIndex2 == edgeIndex0)
	{
		// Degenerate triangle
		return;
	}

	// First check whether we already have a triangle here
	if (alreadyExisted[0] && alreadyExisted[1] && alreadyExisted[2])
	{
		for (uint32_t i = 0; i < _volumeMesh.numTriangles; ++i)
		{
			if ((_volumeMesh.triangles[i].edgeIndices[0] == edgeIndex0 || _volumeMesh.triangles[i].edgeIndices[0] == edgeIndex1 || _volumeMesh.triangles[i].edgeIndices[0] == edgeIndex2) &&
				(_volumeMesh.triangles[i].edgeIndices[1] == edgeIndex0 || _volumeMesh.triangles[i].edgeIndices[1] == edgeIndex1 || _volumeMesh.triangles[i].edgeIndices[1] == edgeIndex2) &&
				(_volumeMesh.triangles[i].edgeIndices[2] == edgeIndex0 || _volumeMesh.triangles[i].edgeIndices[2] == edgeIndex1 || _volumeMesh.triangles[i].edgeIndices[2] == edgeIndex2))
			{
				// Don't do anything more if the triangle already exists
				return;
			}
		}
	}

	// Add the triangle then
	_volumeMesh.triangles[_volumeMesh.numTriangles].edgeIndices[0] = edgeIndex0;
	_volumeMesh.triangles[_volumeMesh.numTriangles].edgeIndices[1] = edgeIndex1;
	_volumeMesh.triangles[_volumeMesh.numTriangles].edgeIndices[2] = edgeIndex2;

	// Store the triangle indices; these are indices into the mesh, not the source model indices
	edge0 = &_volumeMesh.edges[edgeIndex0];
	edge1 = &_volumeMesh.edges[edgeIndex1];
	edge2 = &_volumeMesh.edges[edgeIndex2];

	if (edge0->vertexIndices[0] == edge1->vertexIndices[0] || edge0->vertexIndices[0] == edge1->vertexIndices[1])
	{ _volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[0] = edge0->vertexIndices[1]; } else
	{
		_volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[0] = edge0->vertexIndices[0];
	}

	if (edge1->vertexIndices[0] == edge2->vertexIndices[0] || edge1->vertexIndices[0] == edge2->vertexIndices[1])
	{ _volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[1] = edge1->vertexIndices[1]; } else
	{
		_volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[1] = edge1->vertexIndices[0];
	}

	if (edge2->vertexIndices[0] == edge0->vertexIndices[0] || edge2->vertexIndices[0] == edge0->vertexIndices[1])
	{ _volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[2] = edge2->vertexIndices[1]; } else
	{
		_volumeMesh.triangles[_volumeMesh.numTriangles].vertexIndices[2] = edge2->vertexIndices[0];
	}

	// Calculate the triangle normal
	glm::vec3 n0(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
	glm::vec3 n1(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

	_volumeMesh.triangles[_volumeMesh.numTriangles].normal.x = n0.y * n1.z - n0.z * n1.y;
	_volumeMesh.triangles[_volumeMesh.numTriangles].normal.y = n0.z * n1.x - n0.x * n1.z;
	_volumeMesh.triangles[_volumeMesh.numTriangles].normal.z = n0.x * n1.y - n0.y * n1.x;

	// Check which edges have the correct winding order for this triangle
	_volumeMesh.triangles[_volumeMesh.numTriangles].winding = 0;

	if (memcmp(&_volumeMesh.vertices[edge0->vertexIndices[0]], &v0, sizeof(v0)) == 0) { _volumeMesh.triangles[_volumeMesh.numTriangles].winding |= 0x01; }

	if (memcmp(&_volumeMesh.vertices[edge1->vertexIndices[0]], &v1, sizeof(v1)) == 0) { _volumeMesh.triangles[_volumeMesh.numTriangles].winding |= 0x02; }

	if (memcmp(&_volumeMesh.vertices[edge2->vertexIndices[0]], &v2, sizeof(v2)) == 0) { _volumeMesh.triangles[_volumeMesh.numTriangles].winding |= 0x04; }

	++_volumeMesh.numTriangles;
	return;
}

bool Volume::init(const assets::Mesh& mesh)
{
	const assets::Mesh::VertexAttributeData* positions = mesh.getVertexAttributeByName("POSITION");

	if (positions == NULL) { return false; }

	uint32_t posIdx = positions->getDataIndex();
	if (posIdx) { return false; }

	const assets::Mesh::FaceData& faceData = mesh.getFaces();

	return init(static_cast<const uint8_t*>(mesh.getData(posIdx)), mesh.getNumVertices(), mesh.getStride(posIdx), positions->getVertexLayout().dataType, faceData.getData(),
		mesh.getNumFaces(), faceData.getDataType());
}

bool Volume::init(const uint8_t* const data, uint32_t numVertices, uint32_t verticesStride, DataType vertexType, const uint8_t* const faceData, uint32_t numFaces, IndexType indexType)
{
	delete[] _volumeMesh.vertices;
	_volumeMesh.numVertices = 0;

	delete[] _volumeMesh.edges;
	_volumeMesh.numEdges = 0;

	delete[] _volumeMesh.triangles;
	_volumeMesh.numTriangles = 0;

	_volumeMesh.vertices = new glm::vec3[numVertices];

	if (faceData)
	{
		_volumeMesh.edges = new VolumeEdge[3 * numFaces];
		_volumeMesh.triangles = new VolumeTriangle[3 * numFaces];

		uint32_t indexStride = indexTypeSizeInBytes(indexType);

		const uint8_t* facePtr = static_cast<const uint8_t*>(faceData);

		for (uint32_t i = 0; i < numFaces; ++i)
		{
			uint32_t indices[3];
			assets::helper::VertexIndexRead(facePtr, indexType, &indices[0]);
			facePtr += indexStride;
			assets::helper::VertexIndexRead(facePtr, indexType, &indices[1]);
			facePtr += indexStride;
			assets::helper::VertexIndexRead(facePtr, indexType, &indices[2]);
			facePtr += indexStride;

			glm::vec3 vertex0, vertex1, vertex2;
			assets::helper::VertexRead(data + (verticesStride * indices[0]), vertexType, 3, &vertex0.x);
			assets::helper::VertexRead(data + (verticesStride * indices[1]), vertexType, 3, &vertex1.x);
			assets::helper::VertexRead(data + (verticesStride * indices[2]), vertexType, 3, &vertex2.x);

			findOrCreateTriangle(vertex0, vertex1, vertex2);
		}
	}
	else // Non-index
	{
		_volumeMesh.edges = new VolumeEdge[numVertices / 3];
		_volumeMesh.triangles = new VolumeTriangle[numVertices / 3];

		for (uint32_t i = 0; i < numVertices; i += 3)
		{
			glm::vec3 vertex0, vertex1, vertex2;
			assets::helper::VertexRead(data + (verticesStride * (i + 0)), vertexType, 3, &vertex0.x);
			assets::helper::VertexRead(data + (verticesStride * (i + 1)), vertexType, 3, &vertex1.x);
			assets::helper::VertexRead(data + (verticesStride * (i + 2)), vertexType, 3, &vertex2.x);

			findOrCreateTriangle(vertex0, vertex1, vertex2);
		}
	}

#ifdef DEBUG
	// Check the data is valid
	for (uint32_t edge = 0; edge < _volumeMesh.numEdges; ++edge)
	{
		uint32_t count = 0;
		for (uint32_t triangle = 0; triangle < _volumeMesh.numTriangles; ++triangle)
		{
			if (_volumeMesh.triangles[triangle].edgeIndices[0] == edge) { ++count; }

			if (_volumeMesh.triangles[triangle].edgeIndices[1] == edge) { ++count; }

			if (_volumeMesh.triangles[triangle].edgeIndices[2] == edge) { ++count; }
		}

		/*
			Every edge should be referenced exactly twice.
			If they aren't then the mesh isn't closed which will cause problems when rendering.
		*/
		if (count != 2) { _isClosed = false; }
	}

#endif

	// Create the real mesh
	{
		glm::vec3* tmp = new glm::vec3[_volumeMesh.numVertices];

		memcpy(tmp, _volumeMesh.vertices, _volumeMesh.numVertices * sizeof(*_volumeMesh.vertices));
		delete[] _volumeMesh.vertices;
		_volumeMesh.vertices = tmp;
	}

	{
		VolumeEdge* tmp = new VolumeEdge[_volumeMesh.numEdges];

		memcpy(tmp, _volumeMesh.edges, _volumeMesh.numEdges * sizeof(*_volumeMesh.edges));
		delete[] _volumeMesh.edges;
		_volumeMesh.edges = tmp;
	}

	{
		VolumeTriangle* tmp = new VolumeTriangle[_volumeMesh.numTriangles];

		memcpy(tmp, _volumeMesh.triangles, _volumeMesh.numTriangles * sizeof(*_volumeMesh.triangles));
		delete[] _volumeMesh.triangles;
		_volumeMesh.triangles = tmp;
	}

	_volumeMesh.needs32BitIndices = (_volumeMesh.numTriangles * 2 * 3) > 65535;

	return true;
}

uint8_t* Volume::getVertexData() { return _volumeMesh.vertexData; }

uint32_t Volume::getVertexDataPositionOffset() { return 0; }

uint32_t Volume::getVertexDataExtrudeOffset() { return sizeof(float) * 3; }

uint32_t Volume::getVertexDataSize() { return _volumeMesh.numVertices * 2 * getVertexDataStride(); }

uint32_t Volume::getVertexDataStride() { return 3 * sizeof(float) + sizeof(uint32_t); }

uint32_t Volume::getIndexDataSize() { return _volumeMesh.numTriangles * 2 * 3 * getIndexDataStride(); }

uint32_t Volume::getIndexDataStride() { return _volumeMesh.needs32BitIndices ? sizeof(uint32_t) : sizeof(uint16_t); }

uint32_t Volume::getTriangleCount() { return _volumeMesh.numTriangles; }

Volume::VolumeTriangle Volume::getTriangleData(uint32_t triangleIndex) { return _volumeMesh.triangles[triangleIndex]; }

void Volume::getVerticesForTriangle(const VolumeTriangle& triangle, glm::vec3& vertex0, glm::vec3& vertex1, glm::vec3& vertex2)
{
	vertex0 = _volumeMesh.vertices[triangle.vertexIndices[0]];
	vertex1 = _volumeMesh.vertices[triangle.vertexIndices[1]];
	vertex2 = _volumeMesh.vertices[triangle.vertexIndices[2]];
}

bool Volume::isVolumeClosed() { return _isClosed; }
} // namespace pvr
//!\endcond