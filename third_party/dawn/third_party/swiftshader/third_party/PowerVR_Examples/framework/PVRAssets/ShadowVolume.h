/*!
\brief Contains an implementation of Shadow Volume generation.
\file PVRAssets/ShadowVolume.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/model/Mesh.h"
#include "PVRAssets/Volume.h"

namespace pvr {

/// <summary>Represents data for handling Shadow volumes of a single Mesh.</summary>
class ShadowVolume : public Volume
{
public:
	/// <summary>Enumerates the different options for different kinds of Shadow volumes.</summary>
	enum Flags
	{
		Visible = 0x01, //!< The specified part is Visible
		Cap_front = 0x02, //!< The front cap of the volume
		Cap_back = 0x04, //!< The back cap of the volume
		Zfail = 0x08 //!< The specified item is configured as Z-Fail
	};

public:
	/// <summary>dtor, releases all resources held by the shadow volume.</summary>
	~ShadowVolume();

	/// <summary>Allocate memory for a new shadow volume with the specified ID.</summary>
	/// <param name="volumeID">The ID of the volume. If exists, it will be overwritten.</param>
	void alllocateShadowVolume(uint32_t volumeID);

	/// <summary>Delete the Shadow Volume with the provided ID.</summary>
	/// <param name="volumeID">shadow volume id</param>
	/// <returns>pvr::Success of suc</returns>
	bool releaseVolume(uint32_t volumeID);

	/// <summary>Get the number of indices of the specified shadow volume.</summary>
	/// <param name="volumeID">Shadow volume id</param>
	/// <returns>The number of indexes</returns>
	uint32_t getNumIndices(uint32_t volumeID);

	/// <summary>Get the indices of the specified shadow volume.</summary>
	/// <param name="volumeID">shadow volume id</param>
	/// <returns>A pointer to the Index data</returns>
	char* getIndices(uint32_t volumeID);

	/// <summary>Query if this shadow volume is using internal index data.</summary>
	/// <param name="volumeID">Shadow volume id</param>
	/// <returns>Return true if is using internal index data</returns>
	bool isIndexDataInternal(uint32_t volumeID);

	/// <summary>Query if this shadow volume is visible.</summary>
	/// <param name="projection">A projection matrix to test</param>
	/// <param name="lightModel">The light source position (if point) or direction (if directional)</param>
	/// <param name="isPointLight">True if it is a point light, false if it is directional</param>
	/// <param name="cameraZProj">The Z projection of the camera</param>
	/// <param name="extrudeLength">The length of extrusion for the volume.</param>
	/// <returns>True if the volume is visible, otherwise false</returns>
	uint32_t isVisible(const glm::mat4x4 projection, const glm::vec3& lightModel, bool isPointLight, float cameraZProj, float extrudeLength);

	/// <summary>Find the silhouette of the shadow volume for the specified light and prepare it for projection.</summary>
	/// <param name="volumeID">The Shadow Volume to prepare. Must have had alllocateShadowVolume called on it</param>
	/// <param name="flags">The properties of the shadow volume to generate (caps, technique)</param>
	/// <param name="lightModel">The Model-space light. Either point-light(or spot) or directional light supported</param>
	/// <param name="isPointLight">Pass true for point (or spot) light, false for directional</param>
	/// <param name="externalIndexBuffer">An external buffer that contains custom, user provided index data.</param>
	/// <returns>True if successful, otherwise false</returns>
	bool projectSilhouette(uint32_t volumeID, uint32_t flags, const glm::vec3& lightModel, bool isPointLight, char** externalIndexBuffer = NULL);

private:
	// A silhouette?
	struct ShadowVolumeData
	{
		char* indexData;
		uint32_t numIndices; // If the index count is greater than 0 and indexData is NULL then the data is handled externally

		ShadowVolumeData() : indexData(NULL), numIndices(0) {}
		~ShadowVolumeData() { delete indexData; }
	};

	// Extrude
	template<typename INDEXTYPE>
	bool project(uint32_t volumeID, uint32_t flags, const glm::vec3& lightModel, bool isPointLight, INDEXTYPE** externalIndexBuffer);

	typedef std::map<uint32_t, ShadowVolumeData> ShadowVolumeMapType;
	std::map<uint32_t, ShadowVolumeData> _shadowVolumes;
};
} // namespace pvr
