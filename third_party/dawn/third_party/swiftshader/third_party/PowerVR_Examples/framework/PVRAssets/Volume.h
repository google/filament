/*!
\brief Contains an implementation of Volume generation.
\file PVRAssets/Volume.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/model/Mesh.h"

namespace pvr {

/// <summary>Represents data for handling volumes of a single Mesh.</summary>
class Volume
{
public:
	/// <summary>Represents an edge.</summary>
	struct VolumeEdge
	{
		uint32_t vertexIndices[2]; ///< The indexes of the two vertices of the edge
		uint32_t visibilityFlags; ///< Flags
	};

	/// <summary>Represents an face (triangle).</summary>
	struct VolumeTriangle
	{
		uint32_t vertexIndices[3]; ///< The indexes of the three vertices of the triangle
		uint32_t edgeIndices[3]; ///< The indexes of the three vertices of the three edges of the triangle
		glm::vec3 normal; ///< The normal of the triangle
		int32_t winding; ///< The winding of the triangle (clockwise / counterclockwise)
	};

	/// <summary>Preprocessed data needed to create volumes out of a mesh</summary>
	struct VolumeMesh
	{
		/// <summary>A pointer to a list of vertices making up the volume</summary>
		glm::vec3* vertices;
		/// <summary>A pointer to a list of edges between vertices</summary>
		VolumeEdge* edges;
		/// <summary>A pointer to a list of triangles making up the volume</summary>
		VolumeTriangle* triangles;
		/// <summary>The minimum vertex</summary>
		glm::vec3 minimum;
		/// <summary>The maximum vertex</summary>
		glm::vec3 maximum;
		/// <summary>The number of vertices making up the volume</summary>
		uint32_t numVertices;
		/// <summary>The number of edges making up the volume</summary>
		uint32_t numEdges;
		/// <summary>The number of triangles making up the volume</summary>
		uint32_t numTriangles;

		/// <summary>Vertex data</summary>
		uint8_t* vertexData;

		/// <summary>Specifies whether 32 bit indicies are required</summary>
		bool needs32BitIndices;

		/// <summary>Default constructor for a volume mesh</summary>
		VolumeMesh() : vertices(nullptr), edges(nullptr), triangles(nullptr), numVertices(0), numEdges(0), numTriangles(0), vertexData(nullptr), needs32BitIndices(false) {}
	};

	/// <summary>dtor, releases all resources held by the Volume.</summary>
	virtual ~Volume();

	/// <summary>Initialize a volume from the data of a Mesh.</summary>
	/// <param name="mesh">A mesh whose vertex data is used to initialize this Volume instance. The POSITION
	/// semantic must be present in the mesh.</param>
	/// <returns>True if successfully initialized, otherwise false</returns>
	/// <remarks>This method will pre-process the data in the mesh, to calculate all vertices, edges and faces of the
	/// mesh as required. In effect it will extract the POSITION semantic data and the face data and use it to create
	/// a "light" and cleaned up version of the mesh that will be then used to calculate extruded volumes as required.</remarks>
	bool init(const assets::Mesh& mesh);

	/// <summary>Initialize a volume from raw data.</summary>
	/// <param name="data">Pointer to the first POSITION attribute of vertex data (so buffer_start + offset)</param>
	/// <param name="numVertices">Number of vertices in (data)</param>
	/// <param name="verticesStride">Stride between each vertex attribute</param>
	/// <param name="vertexType">The DataType of each position coordinate</param>
	/// <param name="faceData">Pointer to index data</param>
	/// <param name="numFaces">Number of Faces contained in (faceData)</param>
	/// <param name="indexType">Type of indices in faceData (16/32 bit)</param>
	/// <remarks>This method will pre-process the data in the mesh, to calculate all vertices, edges and faces of the
	/// mesh as required</remarks>
	/// <returns>True if successfully initialized, otherwise false</returns>
	bool init(const uint8_t* const data, uint32_t numVertices, uint32_t verticesStride, DataType vertexType, const uint8_t* const faceData, uint32_t numFaces, IndexType indexType);

	/// <summary>Get the size of the vertex attributes in bytes. Is 2 * numVertices * stride.</summary>
	/// <returns>The size of the vertex data</summary>
	uint32_t getVertexDataSize();

	/// <summary>Return the stride of the vertex attributes, in bytes. Is 3 * 4 + 4 = 16 .</summary>
	/// <returns>The stride of the vertex attributes</summary>
	uint32_t getVertexDataStride();

	/// <summary>Return the offset of the Position vertex attribute in bytes. Is 0.</summary>
	/// <returns>The offset of the Position attribute</summary>
	uint32_t getVertexDataPositionOffset();

	/// <summary>Return the extrude offset. Is 3.</summary>
	/// <returns>The size of each vertex attribute</summary>
	uint32_t getVertexDataExtrudeOffset();

	/// <summary>Get a pointer to the raw vertex data. Use to bind vertex buffer.</summary>
	/// <returns> The vertex data</returns>
	uint8_t* getVertexData();

	/// <summary>Get the size of the Index data, in bytes.</summary>
	/// <returns> The index data size</returns>
	uint32_t getIndexDataSize();

	/// <summary>Get the stride of the Index data, in bytes. Is sizeof(IndexType).</summary>
	/// <returns> The index data stride</returns>
	uint32_t getIndexDataStride();

	/// <summary>Get the number of triangles in the volume.</summary>
	/// <returns> The number of triangles</returns>
	uint32_t getTriangleCount();

	/// <summary>Get the triangle data at a particular index in the volume.</summary>
	/// <param name="triangleIndex">The index of the triangle to retrieve</param>
	/// <returns> The triangle data</returns>
	VolumeTriangle getTriangleData(uint32_t triangleIndex);

	/// <summary>Get the vertices of a particular triangle</summary>
	/// <param name="triangle">The triangle from which to return the vertices</param>
	/// <param name="vertex0">Output: The first vertex</param>
	/// <param name="vertex1">Output: The second vertex</param>
	/// <param name="vertex2">Output: The third vertex</param>
	void getVerticesForTriangle(const VolumeTriangle& triangle, glm::vec3& vertex0, glm::vec3& vertex1, glm::vec3& vertex2);

	/// <summary>Get if a volume is closed</summary>
	/// <returns>True if voluem is closed, otherwise false</returns>
	bool isVolumeClosed();

protected:
	/// <summary>Retrieve the index of a vertex by coordinates. If it does not exist, create a new one.</summary>
	/// <param name="vertex">The coordinates of a vertex</param>
	/// <param name="existed">Output: Is set to true if the vertex already existed, otherwise will be set to false</param>
	/// <returns>The index of the vertex (existing or new)</returns>
	uint32_t findOrCreateVertex(const glm::vec3& vertex, bool& existed);

	/// <summary>Retrieve the index of an edge by vertex coordinates. If it does not exist, create a new one.</summary>
	/// <param name="v0">The coordinates of the first vertex of the edge</param>
	/// <param name="v1">The coordinates of the second vertex of the edge</param>
	/// <param name="existed">Output: Is set to true if the edge already existed, otherwise will be set to false</param>
	/// <returns>The index of the edge (existing or new)</returns>
	uint32_t findOrCreateEdge(const glm::vec3& v0, const glm::vec3& v1, bool& existed);

	/// <summary>Create a triangle of given coordinates if one does not exist.</summary>
	/// <param name="v0">The coordinates of the first vertex of the edge</param>
	/// <param name="v1">The coordinates of the second vertex of the edge</param>
	/// <param name="v2">The coordinates of the third vertex of the edge</param>
	void findOrCreateTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

	VolumeMesh _volumeMesh; ///< The internal data of the mesh

	bool _isClosed; ///< Is the mesh closed
};
} // namespace pvr
