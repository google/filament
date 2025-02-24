/*!
\brief Implementation of methods of the PODReader class.
\file PVRAssets/fileio/PODReader.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/fileio/PODReader.h"
#include "PVRAssets/fileio/PODDefines.h"
#include "PVRAssets/Model.h"
#include "PVRCore/Log.h"
#include "PVRAssets/Helper.h"
#include "PVRCore/stream/Stream.h"
#include <cstdio>
#include <algorithm>
using std::vector;

namespace { // LOCAL FUNCTIONS
using namespace pvr;
using namespace assets;
template<typename T>
void readBytes(const Stream& stream, T& data)
{
	stream.readExact(sizeof(T), 1, &data);
}

template<typename T>
void readByteArray(const Stream& stream, T* data, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i) { readBytes(stream, data[i]); }
}

template<typename T>
void readByteArrayIntoTypedMem(const Stream& stream, TypedMem& mem, uint32_t count)
{
	mem.allocate(GpuDatatypesHelper::Metadata<T>::dataTypeOf(), count);
	readByteArray(stream, mem.rawAs<T>(), count);
}
template<typename T>
void readByteArrayIntoFreeValue(const Stream& stream, FreeValue& mem, uint32_t count)
{
	debug_assertion(count * sizeof(T) <= 64, "PODReader: Error trying to read more than 64 bytes into FreeValue");
	mem.setDataType(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	readByteArray(stream, mem.rawAs<T>(), count);
}

template<typename T>
inline void read4Bytes(const Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read4BytesSizeAssert, sizeof(T) == 4)
	unsigned char ub[4];
	stream.readExact(4, 1, &ub);
	uint32_t p;
	p = static_cast<uint32_t>((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
	memcpy(&data, &p, 4);
}

template<typename T>
inline void read4BytesIntoFreeVal(const Stream& stream, FreeValue& value)
{
	value.setDataType(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	read4Bytes(stream, *value.rawAs<T>());
}
template<typename T>
inline void read4BytesIntoTypedMem(const Stream& stream, TypedMem& value)
{
	value.allocate(GpuDatatypesHelper::Metadata<T>::dataTypeOf());
	read4Bytes(stream, value.rawAs<T>());
}

template<typename T>
void read4ByteArray(const Stream& stream, T* data, uint32_t count)
{
	// PVR_STATIC_ASSERT(read4ByteArraySizeAssert, sizeof(T) == 4)
	for (uint32_t i = 0; i < count; ++i) { read4Bytes(stream, data[i]); }
}

template<typename VectorType>
inline void read4ByteArrayIntoGlmVector(const Stream& stream, FreeValue& value)
{
	value.setDataType(GpuDatatypesHelper::Metadata<VectorType>::dataTypeOf());
	read4ByteArray(stream, &(*static_cast<VectorType*>(value.raw()))[0], getNumVecElements(value.dataType()));
}

template<typename T>
inline void read2Bytes(const Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read2BytesSizeAssert, sizeof(T) == 2)
	unsigned char ub[2];
	size_t dataRead;
	stream.read(2, 1, &ub, dataRead);
	unsigned short* p = reinterpret_cast<unsigned short*>(&data);
	*p = static_cast<unsigned short>((ub[1] << 8) | ub[0]);
}

template<typename T>
void read2ByteArray(const Stream& stream, T* data, uint32_t count)
{
	// PVR_STATIC_ASSERT(read2ByteArraySizeAssert, sizeof(T) == 2)
	for (uint32_t i = 0; i < count; ++i) { read2Bytes(stream, data[i]); }
}

template<typename T, typename vector_T>
inline void readByteArrayIntoVector(const Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	readByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template<typename T, typename vector_T>
inline void read2ByteArrayIntoVector(const Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	read2ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}
template<typename T, typename vector_T>
inline void read4ByteArrayIntoVector(const Stream& stream, std::vector<vector_T>& data, uint32_t count)
{
	debug_assertion(sizeof(vector_T) <= sizeof(T), "Wrong size of vector type in PODReader");
	data.resize(count * sizeof(T) / sizeof(vector_T));
	read4ByteArray<T>(stream, reinterpret_cast<T*>(data.data()), count);
}

inline void readByteArrayIntoStringHash(const Stream& stream, StringHash& data, uint32_t count)
{
	std::vector<char> data1;
	data1.resize(count);
	readByteArray(stream, reinterpret_cast<char*>(data1.data()), count);
	data.assign(data1.data());
}

template<typename T>
inline bool read4BytesChecked(const Stream& stream, T& data)
{
	// PVR_STATIC_ASSERT(read4BytesSizeAssert, sizeof(T) == 4)
	unsigned char ub[4];
	size_t dataRead;
	stream.read(4, 1, &ub, dataRead);
	if (dataRead != 1) { return false; }
	uint32_t p;
	p = static_cast<uint32_t>((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]);
	memcpy(&data, &p, 4);
	return true;
}

inline bool readTag(const Stream& stream, uint32_t& identifier, uint32_t& dataLength) { return read4BytesChecked(stream, identifier) && read4BytesChecked(stream, dataLength); }

inline void printMat4(const glm::mat4& mat4, const char* msg)
{
	std::string fmtStr(msg);
	fmtStr.append("\n{%f,%f,%f,%f}\n"
				  "{%f,%f,%f,%f}\n"
				  "{%f,%f,%f,%f}\n"
				  "{%f,%f,%f,%f}\n");
	Log(fmtStr.c_str(), mat4[0][0], mat4[1][0], mat4[2][0], mat4[3][0], // row0
		mat4[0][1], mat4[1][1], mat4[2][1], mat4[3][1], // row1
		mat4[0][2], mat4[1][2], mat4[2][2], mat4[3][2], // row2
		mat4[0][3], mat4[1][3], mat4[2][3], mat4[3][3]); // row3
}

void readVertexIndexData(const Stream& stream, assets::Mesh& mesh)
{
	uint32_t identifier, dataLength, size(0);
	std::vector<uint8_t> data;
	IndexType type(IndexType::IndexType16Bit);
	while (readTag(stream, identifier, dataLength))
	{
		if (identifier == (pod::e_meshVertexIndexList | pod::c_endTagMask))
		{
			mesh.addFaces(data.data(), size, type);
			return;
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			switch (static_cast<DataType>(tmp))
			{
			case DataType::UInt32: type = IndexType::IndexType32Bit; break;
			case DataType::UInt16: type = IndexType::IndexType16Bit; break;
			default:
			{
				throw InvalidDataError("[PODReader::readVertexIndexData]: Unrecognised Index data type");
			}
			}
			continue;
		}
		case pod::e_blockData:
			switch (type)
			{
			case IndexType::IndexType16Bit: read2ByteArrayIntoVector<uint16_t>(stream, data, dataLength / 2); break;
			case IndexType::IndexType32Bit: read4ByteArrayIntoVector<uint32_t>(stream, data, dataLength / 4); break;
			}
			size = dataLength;
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

void readVertexData(const Stream& stream, assets::Mesh& mesh, const char* const semanticName, uint32_t blockIdentifier, int32_t dataIndex, bool& existed)
{
	existed = false;
	uint32_t identifier, dataLength, numComponents(0), stride(0), offset(0);
	DataType type(DataType::None);
	while (readTag(stream, identifier, dataLength))
	{
		if (identifier == (blockIdentifier | pod::c_endTagMask))
		{
			if (numComponents != 0) // Is there a Vertex Attribute to add?
			{
				existed = true;
				mesh.setStride(dataIndex, stride);
				if (mesh.addVertexAttribute(semanticName, type, numComponents, offset, dataIndex) == -1)
				{ throw InvalidDataError("[PODReader::readVertexData] : Add Vertex Attribute [" + std::string(semanticName) + "] failed - Vertex attribute already added"); }
			}
			else
			{
				existed = false;
			}
			return;
		}
		switch (identifier)
		{
		case pod::e_blockDataType:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			type = static_cast<DataType>(tmp);
			continue;
		}
		case pod::e_blockNumComponents: read4Bytes(stream, numComponents); break;
		case pod::e_blockStride: read4Bytes(stream, stride); break;
		case pod::e_blockData:
			if (dataIndex == -1) // This POD file isn't using interleaved data so this data block must be valid vertex data
			{
				std::vector<uint8_t> data;
				switch (dataTypeSize(type))
				{
				case 1: readByteArrayIntoVector<uint8_t>(stream, data, dataLength); break;
				case 2: read2ByteArrayIntoVector<uint16_t>(stream, data, dataLength / 2); break;
				case 4: read2ByteArrayIntoVector<uint32_t>(stream, data, dataLength / 4); break;
				default:
				{
					throw InvalidDataError("[PODReader::readVertexData] : Vertex DataType width was >4");
				}
				}
				dataIndex = mesh.addData(data.data(), dataLength, stride);
			}
			else
			{
				read4Bytes(stream, offset);
			}
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

inline void readTextureIndex(const Stream& stream, const StringHash& semantic, assets::Model::Material::InternalData& data)
{
	int32_t tmp = -1;
	read4Bytes<int32_t>(stream, tmp);
	if (tmp >= 0) { data.textureIndices[semantic] = tmp; }
}

void readMaterialBlock(const Stream& stream, assets::Model::Material& material)
{
	uint32_t identifier, dataLength;
	assets::Model::Material::InternalData& materialInternalData = material.getInternalData();
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneMaterial | pod::c_endTagMask: return;
		case pod::e_materialName | pod::c_startTagMask: readByteArrayIntoStringHash(stream, materialInternalData.name, dataLength); break;
		case pod::e_materialOpacity | pod::c_startTagMask: read4BytesIntoFreeVal<int32_t>(stream, materialInternalData.materialSemantics["OPACITY"]); break;
		case pod::e_materialAmbientColor | pod::c_startTagMask: read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["AMBIENT"]); break;
		case pod::e_materialDiffuseColor | pod::c_startTagMask: read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["DIFFUSE"]); break;
		case pod::e_materialSpecularColor | pod::c_startTagMask: read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["SPECULAR"]); break;
		case pod::e_materialShininess | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["SHININESS"]); break;
		case pod::e_materialEffectFile | pod::c_startTagMask: readByteArrayIntoStringHash(stream, materialInternalData.effectFile, dataLength); break;
		case pod::e_materialEffectName | pod::c_startTagMask: readByteArrayIntoStringHash(stream, materialInternalData.effectName, dataLength); break;
		case pod::e_materialDiffuseTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "DIFFUSETEXTURE", materialInternalData); break;
		case pod::e_materialAmbientTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "AMBIENTTEXTURE", materialInternalData); break;
		case pod::e_materialSpecularColorTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "SPECULARCOLORTEXTURE", materialInternalData); break;
		case pod::e_materialSpecularLevelTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "SPECULARLEVELTEXTURE", materialInternalData); break;
		case pod::e_materialBumpMapTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "NORMALTEXTURE", materialInternalData); break;
		case pod::e_materialEmissiveTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "EMISSIVETEXTURE", materialInternalData); break;
		case pod::e_materialGlossinessTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "GLOSSINESSTEXTURE", materialInternalData); break;
		case pod::e_materialOpacityTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "OPACITYTEXTURE", materialInternalData); break;
		case pod::e_materialReflectionTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "REFLECTIONTEXTURE", materialInternalData); break;
		case pod::e_materialRefractionTextureIndex | pod::c_startTagMask: readTextureIndex(stream, "REFRACTIONTEXTURE", materialInternalData); break;
		case pod::e_materialBlendingRGBSrc | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCSRCCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaSrc | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCSRCALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBDst | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCDSTCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaDst | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDFUNCDSTALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBOperation | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDOPCOLOR"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingAlphaOperation | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			materialInternalData.materialSemantics["BLENDOPALPHA"].setValue(tmp);
			break;
		}
		case pod::e_materialBlendingRGBAColor | pod::c_startTagMask: read4ByteArrayIntoGlmVector<glm::vec4>(stream, materialInternalData.materialSemantics["BLENDCOLOR"]); break;
		case pod::e_materialBlendingFactorArray | pod::c_startTagMask: read4ByteArrayIntoGlmVector<glm::vec4>(stream, materialInternalData.materialSemantics["BLENDFACTOR"]); break;
		case pod::e_materialFlags | pod::c_startTagMask: read4BytesIntoFreeVal<int32_t>(stream, materialInternalData.materialSemantics["FLAGS"]); break;
		case pod::e_materialUserData | pod::c_startTagMask:
			readByteArrayIntoVector<uint8_t>(stream, materialInternalData.userData, dataLength);
			break;

			// pbr
		case pod::e_materialMetallicity | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["METALLICITY"]); break;
		case pod::e_materialRoughness | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["ROUGHNESS"]); break;
		case pod::e_materialIOR | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["IOR"]); break;
		case pod::e_materialFresnel | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["FRESENEL"]); break;
		case pod::e_materialReflectivity | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["REFLECTIVITY"]); break;
		case pod::e_materialSSScattering | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["SSSCATERING"]); break;
		case pod::e_materialSSScateringDepth | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["SSCATERINGDEPTH"]); break;
		case pod::e_materialSSScateringColor | pod::c_startTagMask:
			read4ByteArrayIntoGlmVector<glm::vec3>(stream, materialInternalData.materialSemantics["SSCATERINGCOLOR"]);
			break;
		case pod::e_materialEmission | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["EMISSION"]); break;
		case pod::e_materialEmissionLuminance | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["EMISSIONLUMINANCE"]); break;
		case pod::e_materialEmissionKelvin | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["EMISSIONKELVIN"]); break;
		case pod::e_materialAnisotropy | pod::c_startTagMask: read4BytesIntoFreeVal<float>(stream, materialInternalData.materialSemantics["ANISTROPHY"]); break;
		case pod::e_materialIdxTexMetallicity | pod::c_startTagMask: readTextureIndex(stream, "METALLICITYTEXTURE", materialInternalData); break;
		case pod::e_materialIdxTexRoughness | pod::c_startTagMask: readTextureIndex(stream, "ROUGHNESSTEXTURE", materialInternalData); break;

		default:
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
			break;
		}
	}
}

void readTextureBlock(const Stream& stream, assets::Model::Texture& texture)
{
	uint32_t identifier, dataLength;
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneTexture | pod::c_endTagMask: return;
		case pod::e_textureFilename | pod::c_startTagMask:
		{
			StringHash s;
			readByteArrayIntoStringHash(stream, s, dataLength);
			texture.setName(s);
			break;
		}
		break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
}

void readCameraBlock(const Stream& stream, Camera& camera, float fps)
{
	uint32_t identifier, dataLength;
	Camera::InternalData& cameraInternalData = camera.getInternalData();

	std::vector<float> cameraFovs;

	const float frameDurration = 1 / fps;

	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneCamera | pod::c_endTagMask: return;
		case pod::e_cameraTargetObjectIndex | pod::c_startTagMask: read4Bytes(stream, cameraInternalData.targetNodeIdx); break;
		case pod::e_cameraFOV | pod::c_startTagMask:
		{
			if (cameraInternalData.fovs.size()) { stream.seek(dataLength, Stream::SeekOriginFromCurrent); }
			else
			{
				read4ByteArrayIntoVector<float, float>(stream, cameraFovs, 1);
				cameraInternalData.fovs.resize(1);
				cameraInternalData.fovs[0].timeInSec = 0.0f;
				cameraInternalData.fovs[0].fov = cameraFovs[0];
			}
			break;
		}
		case pod::e_cameraFarPlane | pod::c_startTagMask: read4Bytes(stream, cameraInternalData.farClip); break;
		case pod::e_cameraNearPlane | pod::c_startTagMask: read4Bytes(stream, cameraInternalData.nearClip); break;
		case pod::e_cameraFOVAnimation | pod::c_startTagMask:
		{
			read4ByteArrayIntoVector<float, float>(stream, cameraFovs, dataLength / sizeof(*cameraInternalData.fovs.data()));

			for (uint32_t i = 0; i < cameraFovs.size(); ++i)
			{
				cameraInternalData.fovs[i].fov = cameraFovs[i];
				cameraInternalData.fovs[i].timeInSec = i * frameDurration;
			}

			break;
		}
		default: stream.seek(dataLength, Stream::SeekOriginFromCurrent); break;
		}
	}
}

void readLightBlock(const Stream& stream, Light& light)
{
	uint32_t identifier, dataLength;
	Light::InternalData& lightInternalData = light.getInternalData();
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneLight | pod::c_endTagMask: return;
		case pod::e_lightTargetObjectIndex | pod::c_startTagMask: read4Bytes(stream, lightInternalData.spotTargetNodeIdx); break;
		case pod::e_lightColor | pod::c_startTagMask:
			read4ByteArray(stream, &lightInternalData.color[0], sizeof(lightInternalData.color) / sizeof(lightInternalData.color[0]));
			break;
		case pod::e_lightType | pod::c_startTagMask:
		{
			uint32_t tmp;
			read4Bytes(stream, tmp);
			lightInternalData.type = static_cast<Light::LightType>(tmp);
			break;
		}
		case pod::e_lightConstantAttenuation | pod::c_startTagMask: read4Bytes(stream, lightInternalData.constantAttenuation); break;
		case pod::e_lightLinearAttenuation | pod::c_startTagMask: read4Bytes(stream, lightInternalData.linearAttenuation); break;
		case pod::e_lightQuadraticAttenuation | pod::c_startTagMask: read4Bytes(stream, lightInternalData.quadraticAttenuation); break;
		case pod::e_lightFalloffAngle | pod::c_startTagMask: read4Bytes(stream, lightInternalData.falloffAngle); break;
		case pod::e_lightFalloffExponent | pod::c_startTagMask: read4Bytes(stream, lightInternalData.falloffExponent); break;
		default: stream.seek(dataLength, Stream::SeekOriginFromCurrent); break;
		}
	}
}

// Return the durration (time in sec) of this keyframe.
float addKeyFrameTimeInMS(pvr::assets::Model& model, uint32_t numFrames, pvr::assets::KeyFrameData& outKeyFrame)
{
	// do the time
	const float fps = model.getFPS();
	const float duration = numFrames / fps;

	// divide the durration between number of frames
	float perFrameDurration = duration / numFrames;
	outKeyFrame.timeInSeconds.resize(numFrames);
	for (uint32_t f = 0; f < numFrames; ++f) { outKeyFrame.timeInSeconds[f] = f * perFrameDurration; }
	return duration;
}

void readNodeBlock(const Stream& stream, pvr::assets::Model& model, assets::Model::Node& node)
{
	uint32_t identifier, dataLength;
	assets::Model::Node::InternalData& nodeInternData = node.getInternalData();
	assets::AnimationData& animationData = model.getInternalData().animationsData[0];
	assets::AnimationInstance& animationInstance = model.getAnimationInstance(0);

	assets::AnimationInstance::KeyframeChannel nodeKeyframe[3]; // srt or mat4 as 0
	animationInstance.animationData = &animationData;

	float animationTotalDurration = 0.0f;
	AnimationData::InternalData& animInternData = animationData.getInternalData();
	animInternData.numFrames = 0;
	bool isOldFormat = false;
	float pos[3] = { 0, 0, 0 };
	float rotation[4] = { 0, 0, 0, 1 };
	float scale[7] = { 1, 1, 1, 0, 0, 0, 0 };
	float matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	std::vector<float> transformationData;
	uint32_t transformArraySize = 0; // Transformation elements size (scale/ rotate/ translate/ Matrix)
	pvr::assets::KeyFrameData* keyFrameData = nullptr;

	while (readTag(stream, identifier, dataLength) /*&& dataLength < stream.getSize()*/)
	{
		switch (identifier)
		{
		case pod::e_sceneNode | pod::c_endTagMask:
		{
			if (isOldFormat)
			{
				if (nodeInternData.transformFlags & pvr::assets::Node::InternalData::TransformFlags::Translate) { memcpy(&nodeInternData.getTranslation(), pos, sizeof(pos)); }
				if (nodeInternData.transformFlags & pvr::assets::Node::InternalData::TransformFlags::Rotate) { memcpy(&nodeInternData.getRotate(), rotation, sizeof(rotation)); }
				if (nodeInternData.transformFlags & pvr::assets::Node::InternalData::TransformFlags::Scale) { memcpy(&nodeInternData.getScale(), scale, sizeof(scale)); }
				if (nodeInternData.transformFlags & pvr::assets::Node::InternalData::TransformFlags::Matrix) { memcpy(nodeInternData.frameTransform, matrix, sizeof(matrix)); }
			}
			animationData.getInternalData().durationTime = std::max(animationTotalDurration, animationData.getInternalData().durationTime);

			for (uint32_t i = 0; i < ARRAY_SIZE(nodeKeyframe); ++i)
			{
				if (nodeKeyframe[i].nodes.size()) { animationInstance.keyframeChannels.emplace_back(nodeKeyframe[i]); }
			}

			return;
		}
		case pod::e_nodeIndex | pod::c_startTagMask: read4Bytes(stream, nodeInternData.objectIndex); break;
		case pod::e_nodeName | pod::c_startTagMask: readByteArrayIntoStringHash(stream, nodeInternData.name, dataLength); break;
		case pod::e_nodeMaterialIndex | pod::c_startTagMask: read4Bytes(stream, nodeInternData.materialIndex); break;
		case pod::e_nodeParentIndex | pod::c_startTagMask: read4Bytes(stream, nodeInternData.parentIndex); break;
		// START OLD FORMAT --- DEPRECATED
		case pod::e_nodePosition | pod::c_startTagMask:
			read4ByteArray(stream, &pos[0], 3);
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Translate;

			isOldFormat = true;
			break;
		case pod::e_nodeRotation | pod::c_startTagMask:
			read4ByteArray(stream, &rotation[0], 4);
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Rotate;

			isOldFormat = true;
			break;
		case pod::e_nodeScale | pod::c_startTagMask:
			read4ByteArray(stream, &scale[0], 3);
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Scale;
			isOldFormat = true;
			break;
		case pod::e_nodeMatrix | pod::c_startTagMask:
			read4ByteArray(stream, &matrix[0], 16);
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Matrix;
			isOldFormat = true;
			break;
		// END OLD FORMAT
		case pod::e_nodeAnimationPosition | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, transformationData, dataLength / sizeof(float));
			transformArraySize = dataLength / sizeof(float) / 3;
			if (transformArraySize > 1)
			{
				animationData.getInternalData().keyFrames.emplace_back(pvr::assets::KeyFrameData());
				keyFrameData = &animationData.getInternalData().keyFrames.back();

				nodeKeyframe[2].nodes.emplace_back(&node);
				nodeKeyframe[2].keyFrame = static_cast<uint32_t>(animationData.getInternalData().keyFrames.size()) - 1;

				keyFrameData->translation.resize(transformArraySize);
				memcpy(keyFrameData->translation.data(), transformationData.data(), dataLength);
				keyFrameData->interpolation = pvr::assets::KeyFrameData::InterpolationType::Linear;
				animationTotalDurration = std::max(addKeyFrameTimeInMS(model, transformArraySize, *keyFrameData), animationTotalDurration);
				nodeInternData.hasAnimation = true;
			}

			// store the first frame as the node transformation
			if (transformationData.size())
			{
				memcpy(&nodeInternData.frameTransform[7], transformationData.data(), sizeof(float) * 3);
				memcpy(&nodeInternData.getTranslation(), transformationData.data(), sizeof(float) * 3);
			}
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Translate;
			break;
		case pod::e_nodeAnimationRotation | pod::c_startTagMask:

			transformArraySize = dataLength / sizeof(float) / 4;
			read4ByteArrayIntoVector<float, float>(stream, transformationData, dataLength / sizeof(float));
			if (transformArraySize > 1)
			{
				animationData.getInternalData().keyFrames.emplace_back(pvr::assets::KeyFrameData());
				keyFrameData = &animationData.getInternalData().keyFrames.back();
				nodeKeyframe[1].nodes.emplace_back(&node);
				nodeKeyframe[1].keyFrame = static_cast<uint32_t>(animationData.getInternalData().keyFrames.size()) - 1;

				keyFrameData->rotate.resize(transformArraySize);

				// loop through each quaternion and swap stred them
				for (uint32_t k = 0; k < transformArraySize; ++k)
				{
					keyFrameData->rotate[k] = glm::quat(-transformationData[k * 4 + 3], transformationData[k * 4 + 0], transformationData[k * 4 + 1], transformationData[k * 4 + 2]);
				}

				keyFrameData->interpolation = pvr::assets::KeyFrameData::InterpolationType::Linear;
				animationTotalDurration = std::max(addKeyFrameTimeInMS(model, transformArraySize, *keyFrameData), animationTotalDurration);
				nodeInternData.hasAnimation = true;
			}
			// store the first frame as the node transformation
			if (transformationData.size())
			{
				memcpy(&nodeInternData.frameTransform[3], transformationData.data(), sizeof(float) * 4);
				memcpy(&nodeInternData.getRotate(), transformationData.data(), sizeof(float) * 4);
				nodeInternData.getRotate().w *= -1.0f;
			}
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Rotate;
			break;
		case pod::e_nodeAnimationScale | pod::c_startTagMask:
			transformArraySize = dataLength / sizeof(float) / 7;
			read4ByteArrayIntoVector<float, float>(stream, transformationData, dataLength / sizeof(float));
			if (transformArraySize > 1)
			{
				animationData.getInternalData().keyFrames.emplace_back(pvr::assets::KeyFrameData());
				keyFrameData = &animationData.getInternalData().keyFrames.back();
				nodeKeyframe[0].nodes.emplace_back(&node);
				nodeKeyframe[0].keyFrame = static_cast<uint32_t>(animationData.getInternalData().keyFrames.size()) - 1;

				keyFrameData->scale.resize(transformArraySize);

				for (uint32_t k = 0; k < transformArraySize; ++k)
				{ keyFrameData->scale[k] = glm::vec3(transformationData[k * 7], transformationData[k * 7 + 1], transformationData[k * 7 + 2]); }

				keyFrameData->interpolation = pvr::assets::KeyFrameData::InterpolationType::Linear;
				animationTotalDurration = std::max(addKeyFrameTimeInMS(model, transformArraySize, *keyFrameData), animationTotalDurration);
				nodeInternData.hasAnimation = true;
			}

			if (transformationData.size())
			{
				memcpy(nodeInternData.frameTransform, transformationData.data(), sizeof(float) * 3);
				memcpy(&nodeInternData.getScale(), transformationData.data(), sizeof(float) * 3);
			}
			nodeInternData.transformFlags |= pvr::assets::Node::InternalData::TransformFlags::Scale;
			break;

		case pod::e_nodeAnimationMatrix | pod::c_startTagMask:
			read4ByteArrayIntoVector<float, float>(stream, transformationData, dataLength / sizeof(float));
			transformArraySize = dataLength / sizeof(float) / 16;
			if (transformArraySize > 1)
			{
				animationData.getInternalData().keyFrames.emplace_back(pvr::assets::KeyFrameData());
				keyFrameData = &animationData.getInternalData().keyFrames.back();
				nodeKeyframe[0].nodes.emplace_back(&node);
				nodeKeyframe[0].keyFrame = static_cast<uint32_t>(animationData.getInternalData().keyFrames.size()) - 1;
				keyFrameData->mat4.resize(transformArraySize);
				for (uint32_t m = 0; m < transformArraySize; m++) { memcpy(&keyFrameData->mat4[m], &transformationData[m * 16], sizeof(float) * 16); }

				memcpy(keyFrameData->mat4.data(), transformationData.data(), sizeof(float) * transformationData.size());

				keyFrameData->interpolation = pvr::assets::KeyFrameData::InterpolationType::Linear;
				animationTotalDurration = std::max(addKeyFrameTimeInMS(model, transformArraySize, *keyFrameData), animationTotalDurration);
			}
			memcpy(nodeInternData.frameTransform, transformationData.data(), sizeof(float) * 16);
			nodeInternData.transformFlags = pvr::assets::Node::InternalData::TransformFlags::Matrix;

			break;
		case pod::e_nodeAnimationFlags | pod::c_startTagMask: read4Bytes(stream, animInternData.flags); break;
		case pod::e_nodeAnimationPositionIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.positionIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.positionIndices.size()));
			break;
		case pod::e_nodeAnimationRotationIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.rotationIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.rotationIndices.size()));
			break;
		case pod::e_nodeAnimationScaleIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.scaleIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.scaleIndices.size()));
			break;
		case pod::e_nodeAnimationMatrixIndex | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, animInternData.matrixIndices, dataLength / sizeof(uint32_t));
			animInternData.numFrames = std::max(animInternData.numFrames, static_cast<uint32_t>(animInternData.matrixIndices.size()));
			break;
		case pod::e_nodeUserData | pod::c_startTagMask: readByteArrayIntoVector<uint8_t>(stream, nodeInternData.userData, dataLength); break;
		default: stream.seek(dataLength, Stream::SeekOriginFromCurrent); break;
		}
	}
}

static void fixInterleavedEndiannessUsingVertexData(StridedBuffer& interleaved, const assets::Mesh::VertexAttributeData& data, uint32_t numVertices)
{
	if (!data.getN()) { return; }
	size_t ui32TypeSize = dataTypeSize(data.getVertexLayout().dataType);
	char ub[4];
	uint8_t* pData = interleaved.data() + static_cast<size_t>(data.getOffset());
	switch (ui32TypeSize)
	{
	case 1: return;
	case 2:
	{
		for (uint32_t i = 0; i < numVertices; ++i)
		{
			for (uint32_t j = 0; j < data.getN(); ++j)
			{
				ub[0] = pData[ui32TypeSize * j + 0];
				ub[1] = pData[ui32TypeSize * j + 1];
				((unsigned short*)pData)[j] = (unsigned short)((ub[1] << 8) | ub[0]);
			}
			pData += interleaved.stride;
		}
	}
	break;
	case 4:
	{
		for (uint32_t i = 0; i < numVertices; ++i)
		{
			for (uint32_t j = 0; j < data.getN(); ++j)
			{
				ub[0] = pData[ui32TypeSize * j + 0];
				ub[1] = pData[ui32TypeSize * j + 1];
				ub[2] = pData[ui32TypeSize * j + 2];
				ub[3] = pData[ui32TypeSize * j + 3];
				((uint32_t*)pData)[j] = static_cast<uint32_t>(((ub[3] << 24) | (ub[2] << 16) | (ub[1] << 8) | ub[0]));
			}
			pData += interleaved.stride;
		}
	}
	break;
	default:
	{
		throw InvalidDataError("[PODReader::fixInterleavedEndiannessUsingVertexData] Interleaved endianness fix - data type had width >4!");
	}
	};
}

static bool isLittleEndian()
{
	short int word = 0x0001;
	char ret;
	memcpy(&ret, &word, sizeof(char));
	return ret ? true : false;
}

static void fixInterleavedEndianness(assets::Mesh::InternalData& data, int32_t interleavedDataIndex)
{
	if (interleavedDataIndex == -1 || isLittleEndian()) { return; }
	StridedBuffer& interleavedData = data.vertexAttributeDataBlocks[interleavedDataIndex];
	assets::Mesh::VertexAttributeContainer::iterator walk = data.vertexAttributes.begin();
	for (; walk != data.vertexAttributes.end(); ++walk)
	{
		assets::Mesh::VertexAttributeData& vertexData = walk->value;
		if (static_cast<int32_t>(vertexData.getDataIndex()) == interleavedDataIndex) // It should do
		{ fixInterleavedEndiannessUsingVertexData(interleavedData, vertexData, data.primitiveData.numVertices); }
	}
}

struct BoneBatches
{
	// BATCH STRIDE
	uint32_t boneBatchStride; //!< Is the number of bones per batch.
	std::vector<uint32_t> batches; //!< Space for batchBoneMax bone indices, per batch
	std::vector<uint32_t> numBones; //!< Actual number of bone indices per batch
	std::vector<uint32_t> offsets; //!< Offset in triangle array per batch

	std::vector<glm::mat4x4> inverseBindMatrices;

	/// <summary>Get number of bone indices of the batches.</summary>
	/// <returns>The number of bone indices in the batches</returns>
	uint16_t getNumBones() const { return static_cast<uint16_t>(numBones.size()); }

	/// <summary>Default Constructor.</summary>
	BoneBatches() : boneBatchStride(0) {}

	/// <summary>Get the offset in the Faces data that the specified batch begins at.</summary>
	/// <param name="batch">The index of a BoneBatch</param>
	/// <returns>The offset, in bytes, in the Faces data that the specified batch begins at.</returns>
	uint32_t getBatchFaceOffset(uint32_t batch) const { return batch < numBones.size() ? offsets[batch] : 0; }
	uint32_t getBatchFaceOffsetBytes(uint32_t batch, IndexType faceDataType) const { return getBatchFaceOffset(batch) * 3 * (faceDataType == IndexType::IndexType16Bit ? 2 : 4); }
};

struct DataCarrier
{
	uint8_t* indexData;
	uint8_t* vertexData;
	size_t vboStride;
	size_t attribOffset;
	size_t indexDataSize;
	size_t valueToAddToVertices;
};
template<typename OP>
class ProcessVertexByIndex
{
	OP op;
	uint8_t* vbo;
	size_t stride;
	size_t offset;

public:
	ProcessVertexByIndex(OP op, uint8_t* vbo, size_t stride, size_t offset) : op(op), vbo(vbo), stride(stride), offset(offset) {}

	void operator()(uint32_t index) { op(vbo + (stride * index + offset)); }
};

static std::map<int, int> indices;

template<typename OP, typename IndexType>
void processByIndex(OP op, const uint8_t* indexData, size_t totalSize)
{
	const uint8_t* const initialData = indexData;
	while (indexData < initialData + totalSize)
	{
		IndexType index = *reinterpret_cast<const IndexType*>(indexData);
		if (indices.find(index) == indices.end())
		{
			indices[index] = 0;
			op(index);
		}
		indexData += (sizeof(IndexType));
	}
}

template<typename T>
class AddOp
{
	T valueToAdd;
	size_t width;

public:
	AddOp(T valueToAdd, size_t width) : valueToAdd(valueToAdd), width(width) {}
	void operator()(void* destination)
	{
		for (size_t i = 0; i < width; ++i)
		{
			T tmp = 0;
			memcpy(&tmp, destination, sizeof(T));
			tmp += valueToAdd;
			memcpy(destination, &tmp, sizeof(T));
			destination = static_cast<void*>(static_cast<char*>(destination) + sizeof(T));
		}
	}
};

template<typename IndexType, typename ValueType>
void addValueWithIndex(const DataCarrier& data, size_t width)
{
	typedef AddOp<ValueType> OP;
	typedef ProcessVertexByIndex<OP> Process;
	processByIndex<Process, IndexType>(Process(OP((ValueType)data.valueToAddToVertices, width), data.vertexData, data.vboStride, data.attribOffset), data.indexData, data.indexDataSize);
}

template<typename ValueType>
void dispatchByIndex(const DataCarrier& data, bool is16bit, size_t width)
{
	if (is16bit) { addValueWithIndex<uint16_t, ValueType>(data, width); }
	else
	{
		addValueWithIndex<uint32_t, ValueType>(data, width);
	}
}

void addOffsetToVertices(const DataCarrier& data, bool is16bit, DataType dataType, size_t width)
{
	switch (dataType)
	{
	case DataType::Int8: dispatchByIndex<int8_t>(data, is16bit, width); break;
	case DataType::UInt8: dispatchByIndex<uint8_t>(data, is16bit, width); break;
	case DataType::Int16: dispatchByIndex<int16_t>(data, is16bit, width); break;
	case DataType::UInt16: dispatchByIndex<uint16_t>(data, is16bit, width); break;
	case DataType::Int32: dispatchByIndex<int32_t>(data, is16bit, width); break;
	case DataType::UInt32: dispatchByIndex<uint32_t>(data, is16bit, width); break;
	case DataType::Float32: dispatchByIndex<float>(data, is16bit, width); break;
	default: assertion(0); break;
	}
}

void mergeBoneBatches(uint32_t boneIndexAttributeId, pvr::assets::Mesh& mesh, BoneBatches& bonebatches)
{
	pvr::assets::Mesh::InternalData& meshData = mesh.getInternalData();
	DataCarrier data;
	if (bonebatches.numBones.size() < 2) { return; }

	uint32_t numNewBones = 0;
	for (size_t i = 0; i < bonebatches.numBones.size(); ++i) { numNewBones += bonebatches.numBones[i]; }

	const auto& attrib = *mesh.getVertexAttribute(boneIndexAttributeId);

	indices.clear();
	IndexType faceDataType = meshData.faces.getDataType();
	for (uint32_t i = 0; i < bonebatches.numBones.size(); ++i)
	{
		data.indexData = meshData.faces.getData() + static_cast<uint32_t>(bonebatches.getBatchFaceOffsetBytes(i, faceDataType));
		data.vertexData = meshData.vertexAttributeDataBlocks[attrib.getDataIndex()].data();
		data.vboStride = meshData.vertexAttributeDataBlocks[attrib.getDataIndex()].stride;
		data.attribOffset = attrib.getOffset();
		if (i + 1u < bonebatches.numBones.size())
		{ data.indexDataSize = bonebatches.getBatchFaceOffsetBytes(i + 1, faceDataType) - static_cast<uint32_t>(bonebatches.getBatchFaceOffsetBytes(i, faceDataType)); }
		else
		{
			data.indexDataSize = mesh.getFaces().getDataSize() - static_cast<uint32_t>(bonebatches.getBatchFaceOffsetBytes(i, faceDataType));
		}

		data.valueToAddToVertices = i * bonebatches.boneBatchStride;

		const bool is16bit = (meshData.faces.getDataType() == IndexType::IndexType16Bit);

		addOffsetToVertices(data, is16bit, attrib.getVertexLayout().dataType, attrib.getVertexLayout().width);
	}
	bonebatches.boneBatchStride = numNewBones;
	bonebatches.numBones.resize(1);
	bonebatches.numBones[0] = numNewBones;
	bonebatches.offsets.resize(1);
	bonebatches.offsets[0] = 0;
}

void readMeshBlock(const Stream& stream, assets::Mesh& mesh, assets::Model& model)
{
	BoneBatches boneBatches;

	bool exists = false;
	uint32_t identifier, dataLength, numUVWs(0), podUVWs(0), numBoneBatches(0);
	int32_t interleavedDataIndex(-1);
	assets::Mesh::InternalData& meshInternalData = mesh.getInternalData();
	meshInternalData.numBones = 0;
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::e_sceneMesh | pod::c_endTagMask:
		{
			meshInternalData.primitiveData.isIndexed = (meshInternalData.faces.getDataSize() != 0);
			if (meshInternalData.primitiveData.stripLengths.size()) { meshInternalData.primitiveData.primitiveType = PrimitiveTopology::TriangleStrip; }
			else
			{
				meshInternalData.primitiveData.primitiveType = PrimitiveTopology::TriangleList;
			}
			fixInterleavedEndianness(meshInternalData, interleavedDataIndex);

			int32_t vDataId = mesh.getVertexAttributeIndex("BONEINDEX");
			if (vDataId >= 0) { mergeBoneBatches(vDataId, mesh, boneBatches); }

			// each mesh has own skeleton.
			// Create a skeleton
			if (boneBatches.batches.size())
			{
				model.getInternalData().skeletons.emplace_back(Skeleton());
				Skeleton& skeleton = model.getInternalData().skeletons.back();

				mesh.getInternalData().skeleton = static_cast<int32_t>(model.getInternalData().skeletons.size()) - 1;
				skeleton.bones = boneBatches.batches;
			}

			return;
		}
		case pod::e_meshNumVertices | pod::c_startTagMask: read4Bytes(stream, meshInternalData.primitiveData.numVertices); break;
		case pod::e_meshNumFaces | pod::c_startTagMask: read4Bytes(stream, meshInternalData.primitiveData.numFaces); break;
		case pod::e_meshNumUVWChannels | pod::c_startTagMask: read4Bytes(stream, podUVWs); break;
		case pod::e_meshStripLength | pod::c_startTagMask:
		{
			read4ByteArrayIntoVector<uint32_t, uint32_t>(stream, meshInternalData.primitiveData.stripLengths, dataLength / sizeof(*meshInternalData.primitiveData.stripLengths.data()));
			break;
		}
		case pod::e_meshNumStrips | pod::c_startTagMask:
		{
			int32_t numstr(0);
			read4Bytes(stream, numstr);
			if ((size_t)numstr != meshInternalData.primitiveData.stripLengths.size())
			{ throw InvalidDataError("[PODReader::readMeshBlock]: The number of Triangle Strip Lengths was different to the actual number of triangle strips."); }
			break;
		}
		case pod::e_meshInterleavedDataList | pod::c_startTagMask:
		{
			UInt8Buffer data;
			readByteArrayIntoVector<uint8_t>(stream, data, dataLength);
			interleavedDataIndex = mesh.addData(data.data(), static_cast<uint32_t>(data.size()), 0);
			break;
		}
		case pod::e_meshBoneBatchIndexList | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, boneBatches.batches, dataLength / sizeof(boneBatches.batches[0]));
			break;
		case pod::e_meshNumBoneIndicesPerBatch | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, boneBatches.numBones, dataLength / sizeof(boneBatches.numBones[0]));
			break;
		case pod::e_meshBoneOffsetPerBatch | pod::c_startTagMask:
			read4ByteArrayIntoVector<uint32_t>(stream, boneBatches.offsets, dataLength / sizeof(boneBatches.offsets[0]));
			break;
		case pod::e_meshMaxNumBonesPerBatch | pod::c_startTagMask: read4Bytes(stream, boneBatches.boneBatchStride); break;
		case pod::e_meshNumBoneBatches | pod::c_startTagMask:
		{
			read4Bytes(stream, numBoneBatches);
			break;
		}
		case pod::e_meshUnpackMatrix | pod::c_startTagMask:
		{
			float m[16];
			read4ByteArray(stream, &m[0], 16);
			meshInternalData.unpackMatrix = glm::make_mat4(&m[0]);
			break;
		}
		case pod::e_meshVertexIndexList | pod::c_startTagMask: readVertexIndexData(stream, mesh); break;
		case pod::e_meshVertexList | pod::c_startTagMask: readVertexData(stream, mesh, "POSITION", identifier, interleavedDataIndex, exists); break;
		case pod::e_meshNormalList | pod::c_startTagMask: readVertexData(stream, mesh, "NORMAL", identifier, interleavedDataIndex, exists); break;
		case pod::e_meshTangentList | pod::c_startTagMask: readVertexData(stream, mesh, "TANGENT", identifier, interleavedDataIndex, exists); break;
		case pod::e_meshBinormalList | pod::c_startTagMask: readVertexData(stream, mesh, "BINORMAL", identifier, interleavedDataIndex, exists); break;
		case pod::e_meshUVWList | pod::c_startTagMask:
		{
			char semantic[256];
			sprintf(semantic, "UV%i", numUVWs++);
			readVertexData(stream, mesh, semantic, identifier, interleavedDataIndex, exists);
			break;
		}
		case pod::e_meshVertexColorList | pod::c_startTagMask: readVertexData(stream, mesh, "VERTEXCOLOR", identifier, interleavedDataIndex, exists); break;
		case pod::e_meshBoneIndexList | pod::c_startTagMask:
			readVertexData(stream, mesh, "BONEINDEX", identifier, interleavedDataIndex, exists);
			if (exists) { meshInternalData.primitiveData.isSkinned = true; }
			break;
		case pod::e_meshBoneWeightList | pod::c_startTagMask:
			readVertexData(stream, mesh, "BONEWEIGHT", identifier, interleavedDataIndex, exists);
			if (exists)
			{
				meshInternalData.primitiveData.isSkinned = true;
				meshInternalData.numBones = mesh.getVertexAttributeByName("BONEWEIGHT")->getN();
			}
			break;
		default:
		{
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
		}
	}
	assertion(0, "NOT IMPLEMENTED YET");
	if (boneBatches.numBones.size() != numBoneBatches) { throw InvalidDataError("[PODReader::readMeshBlock]: Number of bone batches was incorrect."); }
}

void readSceneBlock(const Stream& stream, assets::Model& model)
{
	uint32_t identifier, dataLength, temporaryInt;
	assets::Model::InternalData& modelInternalData = model.getInternalData();
	uint32_t numCameras(0), numLights(0), numMaterials(0), numMeshes(0), numTextures(0), numNodes(0);
	modelInternalData.animationsData.resize(1);
	modelInternalData.animationInstances.resize(1);

	AnimationData& animation = modelInternalData.animationsData[0];
	animation.setAnimationName("Default Animation");

	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::Scene | pod::c_endTagMask:
		{
			if (numCameras != modelInternalData.cameras.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of cameras was incorrect."); }
			if (numLights != modelInternalData.lights.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of lights was incorrect."); }
			if (numMaterials != modelInternalData.materials.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of materials was incorrect."); }
			if (numMeshes != modelInternalData.meshes.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of meshes was incorrect."); }
			if (numTextures != modelInternalData.textures.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of textures was incorrect."); }
			if (numNodes != modelInternalData.nodes.size()) { throw InvalidDataError("[PODReader::readSceneBlock]: Unknown error - Number of nodes was incorrect."); }

			// Loop through the skeleton and compute the bone's inverse bin matrices.
			for (auto& skin : model.getInternalData().skeletons)
			{
				skin.invBindMatrices.resize(skin.bones.size());
				for (uint32_t j = 0; j < skin.bones.size(); ++j) { skin.invBindMatrices[j] = glm::inverse(model.getWorldMatrix(skin.bones[j])); }
			}

			return;
		}
		case pod::e_sceneClearColor | pod::c_startTagMask:
			read4ByteArray(stream, &modelInternalData.clearColor[0], sizeof(modelInternalData.clearColor) / sizeof(modelInternalData.clearColor[0]));
			break;
		case pod::e_sceneAmbientColor | pod::c_startTagMask:
			read4ByteArray(stream, &modelInternalData.ambientColor[0], sizeof(modelInternalData.ambientColor) / sizeof(modelInternalData.ambientColor[0]));
			break;
		case pod::e_sceneNumCameras | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.cameras.resize(temporaryInt);
			break;
		case pod::e_sceneNumLights | pod::c_startTagMask:
		{
			read4Bytes(stream, temporaryInt);
			modelInternalData.lights.resize(temporaryInt);
		}
		break;
		case pod::e_sceneNumMeshes | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.meshes.resize(temporaryInt);
			break;
		case pod::e_sceneNumNodes | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.nodes.resize(temporaryInt);
			break;
		case pod::e_sceneNumMeshNodes | pod::c_startTagMask: read4Bytes(stream, modelInternalData.numMeshNodes); break;
		case pod::e_sceneNumTextures | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.textures.resize(temporaryInt);
			break;
		case pod::e_sceneNumMaterials | pod::c_startTagMask:
			read4Bytes(stream, temporaryInt);
			modelInternalData.materials.resize(temporaryInt);
			break;
		case pod::e_sceneNumFrames | pod::c_startTagMask: read4Bytes(stream, modelInternalData.numFrames); break;
		case pod::e_sceneCamera | pod::c_startTagMask: readCameraBlock(stream, modelInternalData.cameras[numCameras++], model.getFPS()); break;
		case pod::e_sceneLight | pod::c_startTagMask: readLightBlock(stream, modelInternalData.lights[numLights++]); break;
		case pod::e_sceneMesh | pod::c_startTagMask: readMeshBlock(stream, modelInternalData.meshes[numMeshes++], model); break;
		case pod::e_sceneNode | pod::c_startTagMask: readNodeBlock(stream, model, modelInternalData.nodes[numNodes++]); break;
		case pod::e_sceneTexture | pod::c_startTagMask: readTextureBlock(stream, modelInternalData.textures[numTextures++]); break;
		case pod::e_sceneMaterial | pod::c_startTagMask: readMaterialBlock(stream, modelInternalData.materials[numMaterials++]); break;
		case pod::e_sceneFlags | pod::c_startTagMask: read4Bytes(stream, modelInternalData.flags); break;
		case pod::e_sceneFPS | pod::c_startTagMask:
			uint32_t fps;
			read4Bytes(stream, fps);
			modelInternalData.FPS = static_cast<float>(fps);
			break;
		case pod::e_sceneUserData | pod::c_startTagMask: readByteArrayIntoVector<uint8_t>(stream, modelInternalData.userData, dataLength); break;
		case pod::e_sceneUnits | pod::c_startTagMask: read4Bytes(stream, modelInternalData.units); break;
		default: stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
	}
}
} // namespace

namespace pvr {
namespace assets {
void readPOD(const ::pvr::Stream& stream, Model& model)
{
	uint32_t identifier, dataLength;
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version std::string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength) { throw InvalidDataError("[PODReader::readAsset_]: File Version Mismatch"); }
			// ... it is. Check to see if the std::string matches
			char filesVersion[pod::c_PODFormatVersionLength];
			stream.readExact(1, dataLength, &filesVersion[0]);
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0) { throw InvalidDataError("[PODReader::readAsset_]: File Version Mismatch"); }
		}
			continue;
		case pod::Scene | pod::c_startTagMask: readSceneBlock(stream, model); return;
		default:
			// Unhandled data, skip it
			stream.seek(dataLength, Stream::SeekOriginFromCurrent);
		}
	}
}
Model readPOD(const ::pvr::Stream& stream)
{
	Model model;
	readPOD(stream, model);
	return model;
}

bool isPOD(const ::pvr::Stream& stream)
{
	if (!stream.isReadable()) { return false; }
	uint32_t identifier, dataLength;
	size_t dataRead;
	while (readTag(stream, identifier, dataLength))
	{
		switch (identifier)
		{
		case pod::PODFormatVersion | pod::c_startTagMask:
		{
			// Is the version std::string in the file the same length as ours?
			if (dataLength != pod::c_PODFormatVersionLength) { return false; }
			// ... it is. Check to see if the std::string matches
			char filesVersion[pod::c_PODFormatVersionLength];
			stream.read(1, dataLength, &filesVersion[0], dataRead);
			if (dataRead != dataLength) { return false; }
			if (strcmp(filesVersion, pod::c_PODFormatVersion) != 0) { return false; }
			return true;
		}
		default: stream.seek(dataLength, Stream::SeekOriginFromCurrent); break;
		}
	}
	return false;
}

} // namespace assets
} // namespace pvr
//!\endcond
