/*!
\brief Contains definitions for methods of the ShadowVolume class.
\file PVRAssets/ShadowVolume.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/ShadowVolume.h"
#include "PVRAssets/Helper.h"

#include "PVRCore/Log.h"
using std::pair;
using std::map;

const static unsigned short c_linesHyperCube[64] = {
	// Cube0
	0, 1, 2, 3, 0, 2, 1, 3, 4, 5, 6, 7, 4, 6, 5, 7, 0, 4, 1, 5, 2, 6, 3, 7,
	// Cube1
	8, 9, 10, 11, 8, 10, 9, 11, 12, 13, 14, 15, 12, 14, 13, 15, 8, 12, 9, 13, 10, 14, 11, 15,
	// Hyper cube jn
	0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15

};

const static glm::vec3 c_rect0(-1, -1, 1), c_rect1(-1, 1, 1), c_rect2(1, -1, 1), c_rect3(1, 1, 1);
namespace pvr {
ShadowVolume::~ShadowVolume()
{
	std::map<uint32_t, ShadowVolumeData>::iterator walk = _shadowVolumes.begin();
	for (; walk != _shadowVolumes.end(); ++walk)
	{
		const ShadowVolumeData& volume = walk->second;
		delete[] volume.indexData;
	}
}

void ShadowVolume::alllocateShadowVolume(uint32_t volumeID)
{
	ShadowVolumeData volume;
	volume.indexData = new char[getIndexDataSize()];
	_shadowVolumes.insert(pair<uint32_t, ShadowVolumeData>(volumeID, volume));
}

bool ShadowVolume::releaseVolume(uint32_t volumeID)
{
	std::map<uint32_t, ShadowVolumeData>::iterator found = _shadowVolumes.find(volumeID);
	assertion(found != _shadowVolumes.end());

	if (found == _shadowVolumes.end()) { return false; }
	{
		const ShadowVolumeData& volume = found->second;
		delete[] volume.indexData;
	}
	_shadowVolumes.erase(found);
	return true;
}

bool ShadowVolume::isIndexDataInternal(uint32_t volumeID)
{
	ShadowVolumeMapType::iterator found = _shadowVolumes.begin();
	std::advance(found, volumeID);
	return (found != _shadowVolumes.end() && found->second.indexData != NULL);
}

uint32_t ShadowVolume::getNumIndices(uint32_t volumeID)
{
	ShadowVolumeMapType::iterator found = _shadowVolumes.find(volumeID);
	assertion(found != _shadowVolumes.end());

	if (found == _shadowVolumes.end()) { return 0; }

	return found->second.numIndices;
}

char* ShadowVolume::getIndices(uint32_t volumeID)
{
	ShadowVolumeMapType::iterator found = _shadowVolumes.find(volumeID);
	assertion(found != _shadowVolumes.end());

	if (found == _shadowVolumes.end()) { return 0; }

	return found->second.indexData;
}

bool ShadowVolume::projectSilhouette(uint32_t volumeID, uint32_t flags, const glm::vec3& lightModel, bool isPointLight, char** externalIndexBuffer)
{
	if (_volumeMesh.needs32BitIndices) { return project<uint32_t>(volumeID, flags, lightModel, isPointLight, reinterpret_cast<uint32_t**>(externalIndexBuffer)); }
	else
	{
		return project<uint16_t>(volumeID, flags, lightModel, isPointLight, reinterpret_cast<uint16_t**>(externalIndexBuffer));
	}
}

template<typename INDEXTYPE>
bool ShadowVolume::project(uint32_t volumeID, uint32_t flags, const glm::vec3& lightModel, bool isPointLight, INDEXTYPE** externalIndexBuffer)
{
	ShadowVolumeMapType::iterator found = _shadowVolumes.find(volumeID);
	assertion(found != _shadowVolumes.end());

	if (found != _shadowVolumes.end()) { return false; }

	ShadowVolumeData& volume = found->second;
	INDEXTYPE* indices = externalIndexBuffer ? *externalIndexBuffer : reinterpret_cast<INDEXTYPE*>(volume.indexData);

	if (indices == NULL) { return false; }

	float f;
	volume.numIndices = 0;

	// Run through triangles, testing which face the From point
	for (uint32_t i = 0; i < _volumeMesh.numTriangles; ++i)
	{
		VolumeEdge *edge0, *edge1, *edge2;
		edge0 = &_volumeMesh.edges[_volumeMesh.triangles[i].edgeIndices[0]];
		edge1 = &_volumeMesh.edges[_volumeMesh.triangles[i].edgeIndices[1]];
		edge2 = &_volumeMesh.edges[_volumeMesh.triangles[i].edgeIndices[2]];

		if (isPointLight)
		{
			glm::vec3 v;
			v.x = _volumeMesh.vertices[edge0->vertexIndices[0]].x - lightModel.x;
			v.y = _volumeMesh.vertices[edge0->vertexIndices[0]].y - lightModel.y;
			v.z = _volumeMesh.vertices[edge0->vertexIndices[0]].z - lightModel.z;

			// Dot product
			f = _volumeMesh.triangles[i].normal.x * v.x + _volumeMesh.triangles[i].normal.y * v.y + _volumeMesh.triangles[i].normal.z * v.z;
		}
		else
		{
			f = _volumeMesh.triangles[i].normal.x * lightModel.x + _volumeMesh.triangles[i].normal.y * lightModel.y + _volumeMesh.triangles[i].normal.z * lightModel.z;
		}

		if (f >= 0)
		{
			// Triangle is in the light
			edge0->visibilityFlags |= 0x01;
			edge1->visibilityFlags |= 0x01;
			edge2->visibilityFlags |= 0x01;

			if (flags & Cap_front)
			{
				// Add the triangle to the volume, un-extruded.
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[0]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[1]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[2]);
			}
		}
		else
		{
			// Triangle is in shade; set Bit3 if the winding order needs reversing
			edge0->visibilityFlags |= 0x02 | (_volumeMesh.triangles[i].winding & 0x01) << 2;
			edge1->visibilityFlags |= 0x02 | (_volumeMesh.triangles[i].winding & 0x02) << 1;
			edge2->visibilityFlags |= 0x02 | (_volumeMesh.triangles[i].winding & 0x04);

			if (flags & Cap_back)
			{
				// Add the triangle to the volume, extruded.
				// numVertices is used as an offset so that the new index refers to the
				// corresponding position in the second array of vertices (which are extruded)
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[0] + _volumeMesh.numVertices);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[1] + _volumeMesh.numVertices);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.triangles[i].vertexIndices[2] + _volumeMesh.numVertices);
			}
		}
	}

#ifdef DEBUG // Sanity checks
	assertion(volume.numIndices * sizeof(INDEXTYPE) <= getIndexDataSize()); // Have we accessed memory we shouldn't have?

	for (uint32_t i = 0; i < volume.numIndices; ++i) { assertion(indices[i] < _volumeMesh.numVertices * 2); }
#endif

	// Run through edges, testing which are silhouette edges
	for (uint32_t i = 0; i < _volumeMesh.numEdges; ++i)
	{
		if ((_volumeMesh.edges[i].visibilityFlags & 0x03) == 0x03)
		{
			/*
			  Silhouette edge found!
			  The edge is both visible and hidden, so it is along the silhouette of the model (See header notes for more info)
			*/
			if (_volumeMesh.edges[i].visibilityFlags & 0x04)
			{
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0] + _volumeMesh.numVertices);

				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0] + _volumeMesh.numVertices);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1] + _volumeMesh.numVertices);
			}
			else
			{
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1] + _volumeMesh.numVertices);

				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[1] + _volumeMesh.numVertices);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0]);
				indices[volume.numIndices++] = static_cast<INDEXTYPE>(_volumeMesh.edges[i].vertexIndices[0] + _volumeMesh.numVertices);
			}
		}

		// Zero for next render
		_volumeMesh.edges[i].visibilityFlags = 0;
	}

#ifdef DEBUG // Sanity checks
	assertion(volume.numIndices * sizeof(INDEXTYPE) <= getIndexDataSize()); // Have we accessed memory we shouldn't have?

	for (uint32_t i = 0; i < volume.numIndices; ++i) { assertion(indices[i] < _volumeMesh.numVertices * 2); }
#endif

	return true;
}

static inline void transformPoint(const glm::mat4x4& projection, float bx, float by, float bz, float lightProjZ, glm::vec4& out, uint32_t& numClipZ, uint32_t& clipFlagsA)
{
	out.x = (projection[0][0] * bx) + (projection[1][0] * by) + (projection[2][0] * bz) + projection[3][0];
	out.y = (projection[0][1] * bx) + (projection[1][1] * by) + (projection[2][1] * bz) + projection[3][1];
	out.z = (projection[0][2] * bx) + (projection[1][2] * by) + (projection[2][2] * bz) + projection[3][2];
	out.w = (projection[0][3] * bx) + (projection[1][3] * by) + (projection[2][3] * bz) + projection[3][3];

	if (out.z <= 0) { ++numClipZ; }

	if (out.z <= lightProjZ) { ++clipFlagsA; }
}

static inline void extrudeAndTransformPoint(
	const glm::mat4x4& projection, float bx, float by, float bz, const glm::vec3& lightModel, bool isPointLight, float extrudeLength, glm::vec4& out)
{
	// Extrude ...
	if (isPointLight)
	{
		out.x = bx + extrudeLength * (bx - lightModel.x);
		out.y = by + extrudeLength * (by - lightModel.y);
		out.z = bz + extrudeLength * (bz - lightModel.z);
	}
	else
	{
		out.x = bx + extrudeLength * lightModel.x;
		out.y = by + extrudeLength * lightModel.y;
		out.z = bz + extrudeLength * lightModel.z;
	}

	// ... and transform
	out.x = (projection[0][0] * out.x) + (projection[1][0] * out.y) + (projection[2][0] * out.z) + projection[3][0];
	out.y = (projection[0][1] * out.x) + (projection[1][1] * out.y) + (projection[2][1] * out.z) + projection[3][1];
	out.z = (projection[0][2] * out.x) + (projection[1][2] * out.y) + (projection[2][2] * out.z) + projection[3][2];
	out.w = (projection[0][3] * out.x) + (projection[1][3] * out.y) + (projection[2][3] * out.z) + projection[3][3];
}

static inline bool isBoundingHyperCubeVisible(const glm::vec4 (&boundingHyperCube)[16], float cameraZProj)
{
	uint32_t clipFlagsA(0), clipFlagsB(0);
	const glm::vec4* extrudedVertices(&boundingHyperCube[8]);

	for (uint32_t i = 0; i < 8; ++i)
	{
		// Far
		if (extrudedVertices[i].x < extrudedVertices[i].w) { clipFlagsA |= 1 << 0; }

		if (extrudedVertices[i].x > -extrudedVertices[i].w) { clipFlagsA |= 1 << 1; }

		if (extrudedVertices[i].y < extrudedVertices[i].w) { clipFlagsA |= 1 << 2; }

		if (extrudedVertices[i].y > -extrudedVertices[i].w) { clipFlagsA |= 1 << 3; }

		if (extrudedVertices[i].z > 0) { clipFlagsA |= 1 << 4; }

		// Near
		if (boundingHyperCube[i].x < boundingHyperCube[i].w) { clipFlagsA |= 1 << 0; }

		if (boundingHyperCube[i].x > -boundingHyperCube[i].w) { clipFlagsA |= 1 << 1; }

		if (boundingHyperCube[i].y < boundingHyperCube[i].w) { clipFlagsA |= 1 << 2; }

		if (boundingHyperCube[i].y > -boundingHyperCube[i].w) { clipFlagsA |= 1 << 3; }

		if (boundingHyperCube[i].z > 0) { clipFlagsA |= 1 << 4; }
	}

	// Volume is hidden if all the vertices are over a screen edge
	if ((clipFlagsA | clipFlagsB) != 0x1F) { return false; }

	/*
	  Well, according to the simple bounding box check, it might be
	  visible. Let's now test the view frustum against the bounding
	  hyper cube. (Basically the reverse of the previous test!)

	  This catches those cases where a diagonal hyper cube passes near a
	  screen edge.
	*/

	// Subtract the camera position from the vertices. I.e. move the camera to 0,0,0
	glm::vec3 shifted[16];

	for (uint32_t i = 0; i < 16; ++i)
	{
		shifted[i].x = boundingHyperCube[i].x;
		shifted[i].y = boundingHyperCube[i].y;
		shifted[i].z = boundingHyperCube[i].z - cameraZProj;
	}

	unsigned short w0, w1;
	uint32_t clipFlags;
	glm::vec3 v;

	for (uint32_t i = 0; i < 12; ++i)
	{
		w0 = c_linesHyperCube[2 * i + 0];
		w1 = c_linesHyperCube[2 * i + 1];

		v = glm::cross(shifted[w0], shifted[w1]);

		clipFlags = 0;

		if (glm::dot(c_rect0, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect1, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect2, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect3, v) < 0) { ++clipFlags; }

		// clipFlags will be 0 or 4 if the screen edges are on the outside of this bounding-box-silhouette-edge.
		if (clipFlags % 4) { continue; }

		for (uint32_t j = 0; j < 8; ++j)
		{
			if ((j != w0) & (j != w1) && (glm::dot(shifted[j], v) > 0)) { ++clipFlags; }
		}

		// clipFlags will be 0 or 12 if this is a silhouette edge of the bounding box
		if (clipFlags % 12) { continue; }

		return false;
	}

	return true;
}

static inline bool isFrontClipInVolume(const glm::vec4 (&boundingHyperCube)[16])
{
	uint32_t clipFlags(0);
	float scale, x, y, w;

	/*
	  OK. The hyper-bounding-box is in the view frustum.

	  Now decide if we can use Z-pass instead of Z-fail.

	  TODO: if we calculate the convex hull of the front-clip intersection points, we can use the connecting lines to do a more
	  accurate on-screen check (currently it just uses the bounding box of the intersection points.)
	*/

	for (uint32_t i = 0; i < 32; ++i)
	{
		const glm::vec4& v0 = boundingHyperCube[c_linesHyperCube[2 * i + 0]];
		const glm::vec4& v1 = boundingHyperCube[c_linesHyperCube[2 * i + 1]];

		// If both coordinates are negative, or both coordinates are positive, it doesn't cross the Z=0 plane
		if (v0.z * v1.z > 0) { continue; }

		// TODO: if fScale > 0.5f, do the lerp in the other direction; this is
		// because we want fScale to be close to 0, not 1, to retain accuracy.
		scale = (0 - v0.z) / (v1.z - v0.z);

		x = scale * v1.x + (1.0f - scale) * v0.x;
		y = scale * v1.y + (1.0f - scale) * v0.y;
		w = scale * v1.w + (1.0f - scale) * v0.w;

		if (x > -w) { clipFlags |= 1 << 0; }

		if (x < w) { clipFlags |= 1 << 1; }

		if (y > -w) { clipFlags |= 1 << 2; }

		if (y < w) { clipFlags |= 1 << 3; }
	}

	if (clipFlags == 0x0F) { return true; }

	return false;
}

static inline bool isBoundingBoxVisible(const glm::vec4* const boundingHyperCube, float cameraZProj)
{
	glm::vec3 v, shifted[16];
	uint32_t clipFlags(0);
	unsigned short w0, w1;

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (boundingHyperCube[i].x < boundingHyperCube[i].w) { clipFlags |= 1 << 0; }

		if (boundingHyperCube[i].x > -boundingHyperCube[i].w) { clipFlags |= 1 << 1; }

		if (boundingHyperCube[i].y < boundingHyperCube[i].w) { clipFlags |= 1 << 2; }

		if (boundingHyperCube[i].y > -boundingHyperCube[i].w) { clipFlags |= 1 << 3; }

		if (boundingHyperCube[i].z > 0) { clipFlags |= 1 << 4; }
	}

	// Volume is hidden if all the vertices are over a screen edge
	if (clipFlags != 0x1F) { return false; }

	/*
	  Well, according to the simple bounding box check, it might be
	  visible. Let's now test the view frustum against the bounding
	  cube. (Basically the reverse of the previous test!)

	  This catches those cases where a diagonal cube passes near a
	  screen edge.
	*/

	// Subtract the camera position from the vertices. I.e. move the camera to 0,0,0
	for (uint32_t i = 0; i < 8; ++i)
	{
		shifted[i].x = boundingHyperCube[i].x;
		shifted[i].y = boundingHyperCube[i].y;
		shifted[i].z = boundingHyperCube[i].z - cameraZProj;
	}

	for (uint32_t i = 0; i < 12; ++i)
	{
		w0 = c_linesHyperCube[2 * i + 0];
		w1 = c_linesHyperCube[2 * i + 1];

		v = glm::cross(shifted[w0], shifted[w1]);
		clipFlags = 0;

		if (glm::dot(c_rect0, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect1, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect2, v) < 0) { ++clipFlags; }

		if (glm::dot(c_rect3, v) < 0) { ++clipFlags; }

		// clipFlags will be 0 or 4 if the screen edges are on the outside of this bounding-box-silhouette-edge.
		if (clipFlags % 4) { continue; }

		for (uint32_t j = 0; j < 8; ++j)
		{
			if ((j != w0) & (j != w1) && (glm::dot(shifted[j], v) > 0)) { ++clipFlags; }
		}

		// clipFlags will be 0 or 12 if this is a silhouette edge of the bounding box
		if (clipFlags % 12) { continue; }

		return false;
	}

	return true;
}

uint32_t ShadowVolume::isVisible(const glm::mat4x4 projection, const glm::vec3& lightModel, bool isPointLight, float cameraZProj, float extrudeLength)
{
	glm::vec4 boundingHyperCubeT[16];
	uint32_t result(0), numClipZ(0), clipFlagsA(0);

	// Get the light z coordinate in projection space
	float lightProjZ = projection[0][2] * lightModel.x + projection[1][2] * lightModel.y + projection[2][2] * lightModel.z + projection[3][2];

	// Transform the eight bounding box points into projection space
	// Transform the 8 points
	transformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.minimum.y, _volumeMesh.minimum.z, lightProjZ, boundingHyperCubeT[0], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.minimum.y, _volumeMesh.maximum.z, lightProjZ, boundingHyperCubeT[1], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.maximum.y, _volumeMesh.minimum.z, lightProjZ, boundingHyperCubeT[2], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.maximum.y, _volumeMesh.maximum.z, lightProjZ, boundingHyperCubeT[3], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.minimum.y, _volumeMesh.minimum.z, lightProjZ, boundingHyperCubeT[4], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.minimum.y, _volumeMesh.maximum.z, lightProjZ, boundingHyperCubeT[5], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.maximum.y, _volumeMesh.minimum.z, lightProjZ, boundingHyperCubeT[6], numClipZ, clipFlagsA);
	transformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.maximum.y, _volumeMesh.maximum.z, lightProjZ, boundingHyperCubeT[7], numClipZ, clipFlagsA);

	if (numClipZ == 8 && clipFlagsA == 8)
	{
		// We're hidden
		return 0;
	}

	// Extrude the bounding box and transform into projection space
	extrudeAndTransformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.minimum.y, _volumeMesh.minimum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[8]);
	extrudeAndTransformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.minimum.y, _volumeMesh.maximum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[9]);
	extrudeAndTransformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.maximum.y, _volumeMesh.minimum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[10]);
	extrudeAndTransformPoint(projection, _volumeMesh.minimum.x, _volumeMesh.maximum.y, _volumeMesh.maximum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[11]);
	extrudeAndTransformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.minimum.y, _volumeMesh.minimum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[12]);
	extrudeAndTransformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.minimum.y, _volumeMesh.maximum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[13]);
	extrudeAndTransformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.maximum.y, _volumeMesh.minimum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[14]);
	extrudeAndTransformPoint(projection, _volumeMesh.maximum.x, _volumeMesh.maximum.y, _volumeMesh.maximum.z, lightModel, isPointLight, extrudeLength, boundingHyperCubeT[15]);

	// Check whether any part of the hyper bounding box is visible
	if (!isBoundingHyperCubeVisible(boundingHyperCubeT, cameraZProj))
	{
		// Not visible
		return 0;
	}

	// It's visible, so return the appropriate visibility flags
	result = Visible;

	if (numClipZ == 8)
	{
		if (isFrontClipInVolume(boundingHyperCubeT))
		{
			result |= Zfail;

			if (isBoundingBoxVisible(&boundingHyperCubeT[0], cameraZProj)) { result |= Cap_back; }
		}
	}
	else
	{
		if (!(numClipZ | clipFlagsA))
		{
			// 3

			// Nothing to do
		}
		else if (isFrontClipInVolume(boundingHyperCubeT))
		{
			// 5
			result |= Zfail;

			if (isBoundingBoxVisible(&boundingHyperCubeT[0], cameraZProj)) { result |= Cap_front; }

			if (isBoundingBoxVisible(&boundingHyperCubeT[8], cameraZProj)) { result |= Cap_back; }
		}
	}

	return result;
}
} // namespace pvr
//!\endcond
