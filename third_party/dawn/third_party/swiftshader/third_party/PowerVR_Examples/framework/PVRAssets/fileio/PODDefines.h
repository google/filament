/*!
\brief Contains Enumerations and Defines necessary to read POD model files.
\file PVRAssets/fileio/PODDefines.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <cstdint>

namespace pvr {
namespace pod {

/// <summary>Enum identifiers for pod files.</summary>
enum PodTagConstants : uint32_t
{
	c_startTagMask = 0x00000000,
	c_endTagMask = 0x80000000,
	c_TagMash = 0x80000000,
	c_PODFormatVersionLength = 11
};
static const char* const c_PODFormatVersion = "AB.POD.2.0";

/// <summary>Enum for the identifiers in the pod blocks.</summary>
enum PODIdentifiers
{
	PODFormatVersion = 1000,
	Scene,
	ExportOptions,
	FileHistory,
	EndiannessMismatch = -402456576,

	// Scene
	e_sceneClearColor = 2000,
	e_sceneAmbientColor,
	e_sceneNumCameras,
	e_sceneNumLights,
	e_sceneNumMeshes,
	e_sceneNumNodes,
	e_sceneNumMeshNodes,
	e_sceneNumTextures,
	e_sceneNumMaterials,
	e_sceneNumFrames,
	e_sceneCamera, // Will come multiple times
	e_sceneLight, // Will come multiple times
	e_sceneMesh, // Will come multiple times
	e_sceneNode, // Will come multiple times
	e_sceneTexture, // Will come multiple times
	e_sceneMaterial, // Will come multiple times
	e_sceneFlags,
	e_sceneFPS,
	e_sceneUserData,
	e_sceneUnits,

	// Materials
	e_materialName = 3000,
	e_materialDiffuseTextureIndex,
	e_materialOpacity,
	e_materialAmbientColor,
	e_materialDiffuseColor,
	e_materialSpecularColor,
	e_materialShininess,
	e_materialEffectFile,
	e_materialEffectName,
	e_materialAmbientTextureIndex,
	e_materialSpecularColorTextureIndex,
	e_materialSpecularLevelTextureIndex,
	e_materialBumpMapTextureIndex,
	e_materialEmissiveTextureIndex,
	e_materialGlossinessTextureIndex,
	e_materialOpacityTextureIndex,
	e_materialReflectionTextureIndex,
	e_materialRefractionTextureIndex,
	e_materialBlendingRGBSrc,
	e_materialBlendingAlphaSrc,
	e_materialBlendingRGBDst,
	e_materialBlendingAlphaDst,
	e_materialBlendingRGBOperation,
	e_materialBlendingAlphaOperation,
	e_materialBlendingRGBAColor,
	e_materialBlendingFactorArray,
	e_materialFlags,
	e_materialUserData,
	e_materialMetallicity,
	e_materialRoughness,
	e_materialIOR,
	e_materialFresnel,
	e_materialReflectivity,
	e_materialSSScattering,
	e_materialSSScateringDepth,
	e_materialSSScateringColor,
	e_materialEmission,
	e_materialEmissionLuminance,
	e_materialEmissionKelvin,
	e_materialAnisotropy,
	e_materialIdxTexMetallicity,
	e_materialIdxTexRoughness,

	// Textures
	e_textureFilename = 4000,

	// Nodes
	e_nodeIndex = 5000,
	e_nodeName,
	e_nodeMaterialIndex,
	e_nodeParentIndex,
	e_nodePosition, // Deprecated
	e_nodeRotation, // Deprecated
	e_nodeScale, // Deprecated
	e_nodeAnimationPosition,
	e_nodeAnimationRotation,
	e_nodeAnimationScale,
	e_nodeMatrix, // Deprecated
	e_nodeAnimationMatrix,
	e_nodeAnimationFlags,
	e_nodeAnimationPositionIndex,
	e_nodeAnimationRotationIndex,
	e_nodeAnimationScaleIndex,
	e_nodeAnimationMatrixIndex,
	e_nodeUserData,

	// Mesh
	e_meshNumVertices = 6000,
	e_meshNumFaces,
	e_meshNumUVWChannels,
	e_meshVertexIndexList,
	e_meshStripLength,
	e_meshNumStrips,
	e_meshVertexList,
	e_meshNormalList,
	e_meshTangentList,
	e_meshBinormalList,
	e_meshUVWList, // Will come multiple times
	e_meshVertexColorList,
	e_meshBoneIndexList,
	e_meshBoneWeightList,
	e_meshInterleavedDataList,
	e_meshBoneBatchIndexList,
	e_meshNumBoneIndicesPerBatch,
	e_meshBoneOffsetPerBatch,
	e_meshMaxNumBonesPerBatch,
	e_meshNumBoneBatches,
	e_meshUnpackMatrix,

	// Light
	e_lightTargetObjectIndex = 7000,
	e_lightColor,
	e_lightType,
	e_lightConstantAttenuation,
	e_lightLinearAttenuation,
	e_lightQuadraticAttenuation,
	e_lightFalloffAngle,
	e_lightFalloffExponent,

	// Camera
	e_cameraTargetObjectIndex = 8000,
	e_cameraFOV,
	e_cameraFarPlane,
	e_cameraNearPlane,
	e_cameraFOVAnimation,

	// Mesh data block
	e_blockDataType = 9000,
	e_blockNumComponents,
	e_blockStride,
	e_blockData
};
} // namespace pod
} // namespace pvr
