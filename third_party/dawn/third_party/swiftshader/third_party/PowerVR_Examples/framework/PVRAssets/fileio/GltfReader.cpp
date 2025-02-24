#include "GltfReader.h"
#include "PVRAssets/Model.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#pragma warning(push)
#pragma warning(disable : 4456 4189 4774)
#include "../../../external/tinygltf/tiny_gltf.h"
#pragma warning(pop)
#ifdef ANDROID
#include "PVRCore/Android/AndroidAssetStream.h"
#endif
namespace pvr {
namespace assets {

namespace {
std::pair<pvr::DataType, size_t> tinyGltf_getComponentTypeToDataType(int32_t tinyComponent)
{
	const pvr::DataType dataType[] = { pvr::DataType::Int8, pvr::DataType::UInt8, pvr::DataType::Int16, pvr::DataType::UInt16, pvr::DataType::Int32, pvr::DataType::UInt32,
		pvr::DataType::Float32 };

	const size_t sizeInBytes[] = { sizeof(int8_t), sizeof(uint8_t), sizeof(int16_t), sizeof(uint16_t), sizeof(int32_t), sizeof(uint32_t), sizeof(float) };

	return std::make_pair(dataType[tinyComponent - TINYGLTF_COMPONENT_TYPE_BYTE], sizeInBytes[tinyComponent - TINYGLTF_COMPONENT_TYPE_BYTE]);
}

uint32_t tinyGltf_getTypeNumComponents(int32_t tinyComponent)
{
	switch (tinyComponent)
	{
	case TINYGLTF_TYPE_VEC2: return 2;
	case TINYGLTF_TYPE_VEC3: return 3;
	case TINYGLTF_TYPE_VEC4: return 4;
	case TINYGLTF_TYPE_MAT2: return 4;
	case TINYGLTF_TYPE_MAT3: return 9;
	case TINYGLTF_TYPE_MAT4: return 16;
	case TINYGLTF_TYPE_SCALAR: return 1;
	}
	debug_assertion(false, "unknown tinygltf type");
	return static_cast<uint32_t>(-1);
}

pvr::IndexType tinyGltf_getIndexType(uint32_t tinyComponent)
{
	if (tinyComponent == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) { return pvr::IndexType::IndexType32Bit; }
	if (tinyComponent == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) { return pvr::IndexType::IndexType16Bit; }
	debug_assertion(false, "Unsupported index type");
	return pvr::IndexType(uint32_t(-1));
}

pvr::PrimitiveTopology tinyGltf_primitiveTopology(uint32_t primitiveTopology)
{
	if (primitiveTopology == TINYGLTF_MODE_POINTS) { return pvr::PrimitiveTopology::PointList; }
	if (primitiveTopology == TINYGLTF_MODE_LINE) { return pvr::PrimitiveTopology::LineList; }
	if (primitiveTopology == TINYGLTF_MODE_LINE_LOOP) { debug_assertion(false, "Not supported"); }
	if (primitiveTopology == TINYGLTF_MODE_TRIANGLES) { return pvr::PrimitiveTopology::TriangleList; }
	if (primitiveTopology == TINYGLTF_MODE_TRIANGLE_STRIP) { return pvr::PrimitiveTopology::TriangleStrip; }
	if (primitiveTopology == TINYGLTF_MODE_TRIANGLE_FAN) { return pvr::PrimitiveTopology::TriangleFan; }
	debug_assertion(false, "Unsupported primitive topology");
	return pvr::PrimitiveTopology::Count;
}

// Contains framework meshes(gltf primitives) belongs to a single gltfmesh.
struct MeshprimitivesIterator
{
	pvr::assets::Mesh* begin;
	uint32_t numPrimitives;
};

// contains a mapping with tinygltf and framework nodes.
struct NodeMapping
{
	pvr::assets::Model::Node* node;
	NodeMapping() : node(nullptr) {}
};

// Parse the node transformation data. The Trasformation data can be either stored in a matrix or as a SRT
void parseNodeTransformation(const tinygltf::Node& tinyNode, pvr::assets::Node& outNode)
{
	auto& data = outNode.getInternalData();
	if (tinyNode.matrix.size())
	{
		data.frameTransform[0] = static_cast<float>(tinyNode.matrix[0]);
		data.frameTransform[1] = static_cast<float>(tinyNode.matrix[1]);
		data.frameTransform[2] = static_cast<float>(tinyNode.matrix[2]);
		data.frameTransform[3] = static_cast<float>(tinyNode.matrix[3]);

		data.frameTransform[4] = static_cast<float>(tinyNode.matrix[4]);
		data.frameTransform[5] = static_cast<float>(tinyNode.matrix[5]);
		data.frameTransform[6] = static_cast<float>(tinyNode.matrix[6]);
		data.frameTransform[7] = static_cast<float>(tinyNode.matrix[7]);

		data.frameTransform[8] = static_cast<float>(tinyNode.matrix[8]);
		data.frameTransform[9] = static_cast<float>(tinyNode.matrix[9]);
		data.frameTransform[10] = static_cast<float>(tinyNode.matrix[10]);
		data.frameTransform[11] = static_cast<float>(tinyNode.matrix[11]);

		data.frameTransform[12] = static_cast<float>(tinyNode.matrix[12]);
		data.frameTransform[13] = static_cast<float>(tinyNode.matrix[13]);
		data.frameTransform[14] = static_cast<float>(tinyNode.matrix[14]);
		data.frameTransform[15] = static_cast<float>(tinyNode.matrix[15]);
		data.transformFlags = Node::InternalData::TransformFlags::Matrix;
	}
	else
	{
		if (tinyNode.scale.size())
		{
			data.getScale() = glm::vec3(static_cast<float>(tinyNode.scale[0]), static_cast<float>(tinyNode.scale[1]), static_cast<float>(tinyNode.scale[2]));
			data.transformFlags |= Node::InternalData::TransformFlags::Scale;
		}
		if (tinyNode.rotation.size())
		{
			// re order the w for the glm.
			data.getRotate() = glm::quat(static_cast<float>(tinyNode.rotation[3]), static_cast<float>(tinyNode.rotation[0]), static_cast<float>(tinyNode.rotation[1]),
				static_cast<float>(tinyNode.rotation[2]));
			data.transformFlags |= Node::InternalData::TransformFlags::Rotate;
		}
		if (tinyNode.translation.size())
		{
			data.getTranslation() = glm::vec3(static_cast<float>(tinyNode.translation[0]), static_cast<float>(tinyNode.translation[1]), static_cast<float>(tinyNode.translation[2]));
			data.transformFlags |= Node::InternalData::TransformFlags::Translate;
		}
		// construct the initial frame.
		*(glm::mat4*)data.frameTransform = pvr::math::constructSRT(data.getScale(), data.getRotate(), data.getTranslation());
	}
}

inline float normalizedSignedByteToFloat(signed char c) { return std::max(static_cast<float>(c) / 127.f, -1.f); }

inline float normalizedUnSignedByteToFloat(unsigned char c) { return static_cast<float>(c) / 255.f; }

inline float normalizedSignedShortToFloat(signed short c) { return std::max(static_cast<float>(c) / 32767.f, -1.f); }

inline float normalizedUnSignedShortToFloat(unsigned short c) { return static_cast<float>(c) / 65535.f; }

void parseAllAnimation(const tinygltf::Model& tinyModel, pvr::assets::Model& model, std::vector<NodeMapping>& nodeMapping)
{
	model.allocateAnimationsData(static_cast<uint32_t>(tinyModel.animations.size()));
	model.allocateAnimationInstances(static_cast<uint32_t>(tinyModel.animations.size()));

	for (size_t a = 0; a < tinyModel.animations.size(); ++a)
	{
		const tinygltf::Animation& tinyAnim = tinyModel.animations[a];
		pvr::assets::AnimationData& animData = model.getInternalData().animationsData[a];
		pvr::assets::AnimationInstance& animInstance = model.getInternalData().animationInstances[a];

		animInstance.animationData = &animData;
		animData.allocateKeyFrames(static_cast<uint32_t>(tinyAnim.samplers.size()));
		animInstance.keyframeChannels.resize(static_cast<uint32_t>(tinyAnim.samplers.size()));
		animData.getInternalData().animationName = tinyAnim.name;

		float durationTime = 0.0f; // key track of the animation duration of all the sampler.
		// Keep track of which keyframes has been parsed.
		std::vector<char> processedKeyFrame(tinyAnim.samplers.size(), false);

		// For each channels parse the sampler data only if it is not processed already.
		// assign the nodes to the animation instance keyframes.
		for (size_t c = 0; c < tinyAnim.channels.size(); ++c)
		{
			const tinygltf::AnimationChannel& tinyAnimChannel = tinyAnim.channels[c];
			int32_t targetNode = tinyAnimChannel.target_node;
			const int32_t tinySampleIndex = tinyAnimChannel.sampler;
			debug_assertion(tinySampleIndex >= 0, "Invalid sampler id");
			pvr::assets::KeyFrameData& keyFrameData = animData.getAnimationData(static_cast<uint32_t>(tinySampleIndex));

			// Process the key frame data only if its not processed already.
			if (!processedKeyFrame[static_cast<uint32_t>(tinySampleIndex)])
			{
				tinygltf::AnimationSampler tinyAnimSampler = tinyAnim.samplers[static_cast<uint32_t>(tinyAnimChannel.sampler)];
				const tinygltf::Accessor& tinyInAccessor = tinyModel.accessors[static_cast<uint32_t>(tinyAnimSampler.input)];
				const tinygltf::Accessor& tinyOutAccessor = tinyModel.accessors[static_cast<uint32_t>(tinyAnimSampler.output)];

				// time in seconds
				const tinygltf::BufferView& tinyInBufferView = tinyModel.bufferViews[static_cast<uint32_t>(tinyInAccessor.bufferView)];
				const tinygltf::Buffer& tinyInBuffer = tinyModel.buffers[static_cast<uint32_t>(tinyInBufferView.buffer)];

				// S/R/T
				const tinygltf::BufferView& tinyOutBufferView = tinyModel.bufferViews[static_cast<uint32_t>(tinyOutAccessor.bufferView)];
				const tinygltf::Buffer& tinyOutBuffer = tinyModel.buffers[static_cast<uint32_t>(tinyOutBufferView.buffer)];

				if (tinyAnimSampler.interpolation == "LINEAR") { keyFrameData.interpolation = KeyFrameData::InterpolationType::Linear; }
				else if (tinyAnimSampler.interpolation == "STEP")
				{
					keyFrameData.interpolation = KeyFrameData::InterpolationType::Step;
				}
				else if (tinyAnimSampler.interpolation == "CUBICSPLINE")
				{
					keyFrameData.interpolation = KeyFrameData::InterpolationType::CubicSpline;
				}

				// get the time
				if (tinyInAccessor.count)
				{
					keyFrameData.timeInSeconds.resize(tinyInAccessor.count);

					memcpy(&keyFrameData.timeInSeconds[0], tinyInBuffer.data.data() + tinyInAccessor.byteOffset + tinyInBufferView.byteOffset, sizeof(float) * tinyInAccessor.count);
					durationTime = glm::max(durationTime, keyFrameData.timeInSeconds.back());
				}

				// copy the animation data
				const std::string& targetPath = tinyAnimChannel.target_path;
				if (targetPath == "scale")
				{
					debug_assertion(tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Invalid Component type");
					keyFrameData.scale.resize(tinyOutAccessor.count);

					// copy the data
					memcpy(&keyFrameData.scale[0], tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset, sizeof(float) * 3 * tinyOutAccessor.count);
				}
				else if (targetPath == "rotation")
				{
					debug_assertion(tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE ||
							tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT ||
							tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
						"Invalid Component type");

					// copy the data
					keyFrameData.rotate.resize(tinyOutAccessor.count);

					switch (tinyOutAccessor.componentType)
					{
					case TINYGLTF_COMPONENT_TYPE_FLOAT:
						// switch the w component of the quaternion for glm.
						for (uint32_t q = 0; q < tinyOutAccessor.count; ++q)
						{
							const float* data =
								(const float*)(tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset + (q * tinyOutBufferView.byteStride));

							keyFrameData.rotate[q] = glm::quat(data[3], data[0], data[1], data[2]); // wxyz
							keyFrameData.rotate[q] = glm::normalize(keyFrameData.rotate[q]);
						}
						break;
					case TINYGLTF_COMPONENT_TYPE_BYTE:
						// switch the w component of the quaternion for glm.
						for (size_t q = 0; q < tinyOutAccessor.count; ++q)
						{
							const char* data =
								(const char*)((tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset + (q * tinyOutBufferView.byteStride)));

							keyFrameData.rotate[q] = glm::quat(normalizedSignedByteToFloat(data[3]), normalizedSignedByteToFloat(data[0]), normalizedSignedByteToFloat(data[1]),
								normalizedSignedByteToFloat(data[2]));

							keyFrameData.rotate[q] = glm::normalize(keyFrameData.rotate[q]);
						}
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:

						// switch the w component of the quaternion for glm.
						for (size_t q = 0; q < tinyOutAccessor.count; ++q)
						{
							const char* data =
								(const char*)(tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset + (q * tinyOutBufferView.byteStride));

							keyFrameData.rotate[q] = glm::quat(normalizedUnSignedByteToFloat(data[3]), normalizedUnSignedByteToFloat(data[0]),
								normalizedUnSignedByteToFloat(data[1]), normalizedUnSignedByteToFloat(data[2]));
							keyFrameData.rotate[q] = glm::normalize(keyFrameData.rotate[q]);
						}
						break;

					case TINYGLTF_COMPONENT_TYPE_SHORT:
						// switch the w component of the quaternion for glm.
						for (size_t q = 0; q < tinyOutAccessor.count; ++q)
						{
							const int16_t* data =
								(const int16_t*)((tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset + (q * tinyOutBufferView.byteStride)));

							keyFrameData.rotate[q] = glm::quat(normalizedSignedShortToFloat(data[3]), normalizedSignedShortToFloat(data[0]), normalizedSignedShortToFloat(data[1]),
								normalizedSignedShortToFloat(data[2]));
							keyFrameData.rotate[q] = glm::normalize(keyFrameData.rotate[q]);
						}
						break;

					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						// switch the w component of the quaternion for glm.
						for (size_t q = 0; q < tinyOutAccessor.count; ++q)
						{
							const uint16_t* data =
								(const uint16_t*)(tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset + (q * tinyOutBufferView.byteStride));

							keyFrameData.rotate[q] = glm::quat(normalizedUnSignedShortToFloat(data[3]), normalizedUnSignedShortToFloat(data[0]),
								normalizedUnSignedShortToFloat(data[1]), normalizedUnSignedShortToFloat(data[2]));
							keyFrameData.rotate[q] = glm::normalize(keyFrameData.rotate[q]);
						}
						break;
					}
				}
				else if (targetPath == "translation")
				{
					debug_assertion(tinyOutAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Invalid Component type");

					// copy the data
					keyFrameData.translation.resize(tinyOutAccessor.count);

					memcpy(&keyFrameData.translation[0], tinyOutBuffer.data.data() + tinyOutAccessor.byteOffset + tinyOutBufferView.byteOffset, sizeof(float) * 3 * tinyOutAccessor.count);
				}
				processedKeyFrame[static_cast<uint32_t>(tinyAnimChannel.sampler)] = true; // mark as processed
			}

			// assign all the nodes influenced by this sampler.
			pvr::assets::AnimationInstance::KeyframeChannel& channel = animInstance.keyframeChannels[static_cast<uint32_t>(tinySampleIndex)];
			if (targetNode >= 0 && nodeMapping[static_cast<uint32_t>(targetNode)].node != nullptr)
			{
				channel.keyFrame = tinySampleIndex;
				channel.nodes.emplace_back(nodeMapping[static_cast<uint32_t>(targetNode)].node);
				nodeMapping[static_cast<uint32_t>(targetNode)].node->getInternalData().hasAnimation = true;
			}
		}

		animData.getInternalData().durationTime = durationTime;
	}
}

void parseNode(tinygltf::Model& tinyModel, uint32_t tinyNodeId, std::vector<MeshprimitivesIterator>& meshPrimitives, uint32_t& nodeId, int32_t nodeParentId,
	uint32_t& meshNodeIndex, uint32_t& cameraNodeIndex, pvr::assets::Model& outModel, std::vector<NodeMapping>& nodeMapping, std::vector<bool>& processedNodes)
{
	if (processedNodes[tinyNodeId]) return;
	const tinygltf::Node& tinyNode = tinyModel.nodes[tinyNodeId];

	pvr::assets::Model::Node& node = outModel.getNode(nodeId);
	node.getInternalData().name = tinyNode.name;

	parseNodeTransformation(tinyNode, node);
	// set parent if it has one.
	if (nodeParentId != -1) { node.setParentID(nodeParentId); }

	nodeParentId = nodeId;

	// Camera
	if (tinyNode.camera >= 0)
	{
		outModel.getCameraNode(cameraNodeIndex).setIndex(tinyNode.camera);
		++cameraNodeIndex;
	}

	// process the child mesh nodes
	// For each primitives in a mesh:
	// Create a Mesh node and set its parent node.
	if (tinyNode.mesh != -1)
	{
		MeshprimitivesIterator& primitive = meshPrimitives[tinyNode.mesh];
		const tinygltf::Mesh& tinyMesh = tinyModel.meshes[tinyNode.mesh];

		// Retrieve skin used for the mesh
		const tinygltf::Skin* tinySkin = nullptr;

		if (tinyNode.skin != -1) { tinySkin = &tinyModel.skins[tinyNode.skin]; }

		for (uint32_t i = 0; i < primitive.numPrimitives; ++i, ++meshNodeIndex)
		{
			pvr::assets::Model::Node& meshnode = outModel.getMeshNode(meshNodeIndex);
			meshnode.setParentID(nodeParentId);
			pvr::assets::Mesh* mesh = (primitive.begin + i);
			if (tinySkin != nullptr)
			{
				mesh->getInternalData().skeleton = tinyNode.skin;
				mesh->getMeshInfo().isSkinned = true;
			}
			meshnode.setIndex(static_cast<uint32_t>(mesh - outModel.getInternalData().meshes.data()));

			// MATERIAL
			if (tinyMesh.primitives[i].material >= 0) { meshnode.setMaterialIndex(tinyMesh.primitives[i].material); }
		}
	}

	// create a node mapping between the gltf node and the framework node.
	nodeMapping[tinyNodeId].node = &node;
	processedNodes[tinyNodeId] = true;
	++nodeId;
	// do child nodes recursively.
	for (uint32_t child = 0; child < tinyNode.children.size(); ++child)
	{ parseNode(tinyModel, tinyNode.children[child], meshPrimitives, nodeId, nodeParentId, meshNodeIndex, cameraNodeIndex, outModel, nodeMapping, processedNodes); }
}

inline glm::vec4 getColorFactor(const tinygltf::Parameter& parameter)
{
	/// Return the color of a material
	/// Returned value is only valid if the parameter represent a texture from a
	/// material
	return glm::vec4(parameter.number_array[0], parameter.number_array[1], parameter.number_array[2], parameter.number_array.size() > 3 ? parameter.number_array[3] : 1.0);
}

void parseAllTextureAndMaterials(const tinygltf::Model& tinyModel, pvr::assets::Model& outModel)
{
	// TEXTURES
	for (uint32_t i = 0; i < tinyModel.textures.size(); ++i)
	{
		const tinygltf::Texture& tinyTex = tinyModel.textures[i];
		if (tinyTex.source != -1)
		{
			const tinygltf::Image& tinyImage = tinyModel.images[tinyTex.source];
			outModel.addTexture(pvr::assets::Model::Texture(tinyImage.uri));
		}
	}

	// MATERIALS
	for (uint32_t i = 0; i < tinyModel.materials.size(); ++i)
	{
		const tinygltf::Material& tinyMaterial = tinyModel.materials[i];
		pvr::assets::Model::Material mat;
		pvr::assets::Model::Material::GLTFMetallicRoughnessSemantics pbrSemantics(mat);

		mat.setEffectName(tinyMaterial.name);
		//-----------------------------
		// PBR metal/roughness workflow
		for (auto it : tinyMaterial.values)
		{
			const tinygltf::Parameter& parameter = it.second;
			if (it.first == "baseColorFactor")
			{
				const auto& colorFactor = parameter.ColorFactor();
				pbrSemantics.setBaseColor(glm::vec4(colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]));
			}
			else if (it.first == "baseColorTexture")
			{
				pbrSemantics.setBaseColorTextureIndex(parameter.TextureIndex());
			}
			else if (it.first == "metallicFactor")
			{
				pbrSemantics.setMetallicity(static_cast<float>(parameter.Factor()));
			}
			else if (it.first == "roughnessFactor")
			{
				pbrSemantics.setRoughness(static_cast<float>(parameter.Factor()));
			}
			else if (it.first == "metallicRoughnessTexture")
			{
				pbrSemantics.setRoughnessTextureIndex(parameter.TextureIndex());
				pbrSemantics.setMetallicityTextureIndex(parameter.TextureIndex());
			}
		}
		//---------------------------------
		// Normal/Occlusion/Emissive values
		for (auto it : tinyMaterial.additionalValues)
		{
			const tinygltf::Parameter& parameter = it.second;
			if (it.first == "normalTexture") { pbrSemantics.setNormalTextureIndex(parameter.TextureIndex()); }
			else if (it.first == "occlusionTexture")
			{
				pbrSemantics.setOcclusionTextureIndex(parameter.TextureIndex());
			}
			else if (it.first == "emissiveTexture")
			{
				pbrSemantics.setEmissiveTextureIndex(parameter.TextureIndex());
			}
			else if (it.first == "emissiveFactor")
			{
				pbrSemantics.setEmissiveColor(glm::vec3(parameter.number_array[0], parameter.number_array[1], parameter.number_array[2]));
			}
			else if (it.first == "alphaMode")
			{
				if (parameter.string_value == "OPAQUE") { pbrSemantics.setAlphaMode(pvr::assets::Model::Material::GLTFAlphaMode::Opaque); }
				else if (parameter.string_value == "MASK")
				{
					pbrSemantics.setAlphaMode(pvr::assets::Model::Material::GLTFAlphaMode::Mask);
				}
				else if (parameter.string_value == "BLEND")
				{
					pbrSemantics.setAlphaMode(pvr::assets::Model::Material::GLTFAlphaMode::Blend);
				}
			}
			else if (it.first == "alphaCutoff")
			{
				pbrSemantics.setAlphaCutOff(static_cast<float>(parameter.number_value));
			}
			else if (it.first == "doubleSided")
			{
				pbrSemantics.setDoubleSided(parameter.bool_value);
			}
		}
		outModel.addMaterial(mat);
	}
}

void parseAllSkins(const tinygltf::Model& tinyModel, pvr::assets::Model& outModel)
{
	const size_t numSkins = tinyModel.skins.size();
	if (numSkins == 0) { return; }

	outModel.getInternalData().skeletons.resize(numSkins);
	for (uint32_t i = 0; i < numSkins; ++i)
	{
		const tinygltf::Skin& tinySkin = tinyModel.skins[i];
		pvr::assets::Skeleton& skeleton = outModel.getInternalData().skeletons[i];

		skeleton.name = tinySkin.name;
		skeleton.bones.resize(tinySkin.joints.size());
		memcpy(skeleton.bones.data(), tinySkin.joints.data(), tinySkin.joints.size() * sizeof(uint32_t));

		// remap the bones ids
		for (uint32_t bones = 0; bones < skeleton.bones.size(); ++bones) { skeleton.bones[bones] += outModel.getNumMeshNodes(); }

		const tinygltf::Accessor& tinyAccessor = tinyModel.accessors[tinySkin.inverseBindMatrices];
		const tinygltf::BufferView& tinyView = tinyModel.bufferViews[tinyAccessor.bufferView];
		const tinygltf::Buffer& tinyBuffer = tinyModel.buffers[tinyView.buffer];
		const unsigned char* invBindMatData = tinyBuffer.data.data() + tinyAccessor.byteOffset + tinyView.byteOffset;
		debug_assertion(tinySkin.joints.size() == tinyAccessor.count, "Number of joints must be equal to the number of inverseBindMatrices");

		skeleton.invBindMatrices.resize(tinySkin.joints.size());
		memcpy(skeleton.invBindMatrices.data(), invBindMatData, sizeof(glm::mat4) * tinySkin.joints.size());
	}
}

enum class VertexAttributeIndex
{
	Position,
	Normal,
	UV0,
	UV1,
	Tangent,
	BoneIndices,
	BoneWeights,
	Count,
};

void parseAllMesh(const tinygltf::Model& tinyModel, pvr::assets::Model& asset, std::vector<MeshprimitivesIterator>& meshPrimitives)
{
	uint32_t meshIndex = 0;
	const auto& tinyAccessors = tinyModel.accessors;

	struct GltfAttribute
	{
		const unsigned char* data;
		uint32_t strideInBytes;
		uint32_t attribStrideInBytes;
		std::pair<pvr::DataType, size_t> dataType;
		uint32_t N;
		pvr::StringHash semantic;
		GltfAttribute() : data(), strideInBytes(), attribStrideInBytes(), dataType(), N() {}
	};

	for (uint32_t m = 0; m < tinyModel.meshes.size(); ++m)
	{
		const tinygltf::Mesh& tinyMesh = tinyModel.meshes[m];
		// store the mesh primitives range
		meshPrimitives[m].begin = &asset.getMesh(meshIndex);
		meshPrimitives[m].numPrimitives = static_cast<uint32_t>(tinyMesh.primitives.size());

		// process primitive meshes
		std::vector<char> attributeInterleaved;

		GltfAttribute gltfAttributes[static_cast<uint32_t>(VertexAttributeIndex::Count)];

		for (uint32_t p = 0; p < tinyMesh.primitives.size(); ++p)
		{
			pvr::assets::Mesh& mesh = asset.getMesh(meshIndex);
			const tinygltf::Primitive& tinyPrimitive = tinyMesh.primitives[p];
			mesh.setPrimitiveType(tinyGltf_primitiveTopology(static_cast<int32_t>(tinyPrimitive.mode)));

			// VERTEX ATTRIBUTES
			uint32_t numvertices = 0;

			// save string comparison if the attributes is found.
			bool positionAttribFound = false;
			bool texAttrib0Found = false;
			bool texAttrib1Found = false;
			bool normalAttribFound = false;
			bool tangentAttribFound = false;
			bool boneIndicesAttribFound = false;
			bool boneWeightsAttribFound = false;

			uint32_t dataAttribsStride = 0;

			uint32_t totalBufferSizeInBytes = 0;
			bool isInterleaved = false;
			for (auto attrib : tinyPrimitive.attributes)
			{
				const tinygltf::Accessor& tinyAccessor = tinyAccessors[attrib.second];
				const tinygltf::BufferView& tinyBufferView = tinyModel.bufferViews[tinyAccessor.bufferView];
				const tinygltf::Buffer& tinyBuffer = tinyModel.buffers[tinyBufferView.buffer];
				VertexAttributeIndex attribIndex = VertexAttributeIndex::Count;
				// bounding box
				if (!positionAttribFound && attrib.first == "POSITION")
				{
					mesh.getMeshInfo().min = glm::vec3(tinyAccessor.minValues[0], tinyAccessor.minValues[1], tinyAccessor.minValues[2]);
					mesh.getMeshInfo().max = glm::vec3(tinyAccessor.maxValues[0], tinyAccessor.maxValues[1], tinyAccessor.maxValues[2]);
					attribIndex = VertexAttributeIndex::Position;

					positionAttribFound = true;
				}
				else if (!texAttrib0Found && attrib.first == "TEXCOORD_0")
				{
					// rename the semantic
					texAttrib0Found = true;
					attribIndex = VertexAttributeIndex::UV0;
				}
				else if (!texAttrib1Found && attrib.first == "TEXCOORD_1")
				{
					// rename the semantic
					texAttrib1Found = true;
					attribIndex = VertexAttributeIndex::UV1;
				}
				else if (!normalAttribFound && attrib.first == "NORMAL")
				{
					attribIndex = VertexAttributeIndex::Normal;
					normalAttribFound = true;
				}
				else if (!boneIndicesAttribFound && attrib.first == "JOINTS_0")
				{
					attribIndex = VertexAttributeIndex::BoneIndices;
					boneIndicesAttribFound = true;
				}
				else if (!boneWeightsAttribFound && attrib.first == "WEIGHTS_0")
				{
					attribIndex = VertexAttributeIndex::BoneWeights;
					boneWeightsAttribFound = true;
				}
				else if (!tangentAttribFound && attrib.first == "TANGENT")
				{
					attribIndex = VertexAttributeIndex::Tangent;
					tangentAttribFound = true;
				}
				gltfAttributes[static_cast<uint32_t>(attribIndex)].data = tinyBuffer.data.data() + tinyBufferView.byteOffset + tinyAccessor.byteOffset;
				gltfAttributes[static_cast<uint32_t>(attribIndex)].strideInBytes = tinyBufferView.byteStride
					? static_cast<uint32_t>(tinyBufferView.byteStride)
					: tinyGltf_getTypeNumComponents(tinyAccessor.type) * tinyGltf_getComponentTypeToDataType(tinyAccessor.componentType).second;
				gltfAttributes[static_cast<uint32_t>(attribIndex)].N = tinyGltf_getTypeNumComponents(tinyAccessor.type); // Get number of component this type has. e.g vec3, vec4
				gltfAttributes[static_cast<uint32_t>(attribIndex)].dataType = tinyGltf_getComponentTypeToDataType(tinyAccessor.componentType);
				gltfAttributes[static_cast<uint32_t>(attribIndex)].semantic = attrib.first;
				numvertices = static_cast<uint32_t>(tinyAccessor.count);
				dataAttribsStride += static_cast<uint32_t>(gltfAttributes[static_cast<uint32_t>(attribIndex)].strideInBytes);

				if (gltfAttributes[static_cast<uint32_t>(attribIndex)].strideInBytes >
					(gltfAttributes[static_cast<uint32_t>(attribIndex)].N * gltfAttributes[static_cast<uint32_t>(attribIndex)].dataType.second))
				{ isInterleaved = true; }
			}

			totalBufferSizeInBytes = dataAttribsStride * numvertices;
			uint32_t bufferOffset = 0;

			// If the vertices already interleaved than do nothing  and copy the data, else interleave the vertices.
			// Interleave the vertices.
			if (!isInterleaved)
			{
				attributeInterleaved.resize(totalBufferSizeInBytes);
				for (uint32_t i = 0; i < numvertices; ++i)
				{
					for (uint32_t j = 0; j < ARRAY_SIZE(gltfAttributes); ++j)
					{
						if (gltfAttributes[j].data != nullptr)
						{
							const auto& tinyAttrib = gltfAttributes[j];
							memcpy(attributeInterleaved.data() + bufferOffset, tinyAttrib.data + tinyAttrib.strideInBytes * i, tinyAttrib.strideInBytes);

							// set the vertex attribute only once for each attribute.
							if (i == 0)
							{
								pvr::assets::VertexAttributeData attribData;

								attribData.setN((uint8_t)tinyAttrib.N);
								attribData.setDataType(tinyAttrib.dataType.first);
								attribData.setDataIndex(0);
								attribData.setOffset(bufferOffset);

								if (j == static_cast<uint32_t>(VertexAttributeIndex::UV0)) { attribData.setSemantic("UV0"); }
								else if (j == static_cast<uint32_t>(VertexAttributeIndex::UV1))
								{
									attribData.setSemantic("UV1");
								}
								else
								{
									attribData.setSemantic(tinyAttrib.semantic);
								}
								mesh.addVertexAttribute(attribData);
							}
							bufferOffset += static_cast<uint32_t>(tinyAttrib.dataType.second) * tinyAttrib.N;
						}
					}
				}
				mesh.addData((const uint8_t*)attributeInterleaved.data(), totalBufferSizeInBytes, dataAttribsStride, 0);
			}

			mesh.setNumVertices(numvertices);

			if (tinyPrimitive.indices != -1)
			{
				const tinygltf::Accessor& tinyAccessor = tinyAccessors[tinyPrimitive.indices];
				const tinygltf::BufferView& tinyBufferView = tinyModel.bufferViews[tinyAccessor.bufferView];
				const tinygltf::Buffer& tinyBuffer = tinyModel.buffers[tinyBufferView.buffer];

				pvr::IndexType indexType = tinyGltf_getIndexType(tinyAccessor.componentType);
				mesh.addFaces(tinyBuffer.data.data() + tinyBufferView.byteOffset + tinyAccessor.byteOffset,
					(indexType == pvr::IndexType::IndexType16Bit ? sizeof(uint16_t) : sizeof(uint32_t)) * static_cast<uint32_t>(tinyAccessor.count), indexType);
			}
			++meshIndex; // next mesh
		}
	}
}

void parseAllCameras(const tinygltf::Model& tinyModel, pvr::assets::Model& asset)
{
	if (tinyModel.cameras.size())
	{
		asset.allocCameras(static_cast<uint32_t>(tinyModel.cameras.size()));

		// parse the cameras
		for (uint32_t i = 0; i < tinyModel.cameras.size(); ++i)
		{
			const tinygltf::Camera& tinyCamera = tinyModel.cameras[i];
			pvr::assets::Camera& camera = asset.getCamera(i);
			if (tinyCamera.type == "perspective")
			{
				const tinygltf::PerspectiveCamera& tinyCamData = tinyCamera.perspective;
				camera.setNear(tinyCamData.znear);
				camera.setFar(tinyCamData.zfar);
				camera.setFOV(tinyCamData.yfov);
			}
		}
	}
}
// Implements load external file function which get called by the tinygltf for loading secondary assets.
class GltfFileLoader : public tinygltf::IFileLoader
{
public:
	GltfFileLoader(const pvr::IAssetProvider& assetProvider) : assetProvider(&assetProvider) {}
	bool loadExternalFile(std::vector<unsigned char>* out, std::string* err, const std::string& filename, const std::string& basedir, size_t reqBytes, bool checkSize)
	{
		(void)basedir; // UNREFERENCE_VARIABLE
		auto stream = assetProvider->getAssetStream(filename);
		if (!stream) { return false; }
		const uint32_t sz = static_cast<uint32_t>(stream->getSize());
		out->resize(sz);
		size_t readSize = 0;
		stream->read(out->size(), 1, out->data(), readSize);
		if (checkSize)
		{
			if (reqBytes == sz) { return true; }
			else
			{
				std::stringstream ss;
				ss << "File size mismatch : " << filename << ", requestedBytes " << reqBytes << ", but got " << sz << std::endl;
				if (err) { (*err) += ss.str(); }
				return false;
			}
		}
		return true;
	}

private:
	const pvr::IAssetProvider* assetProvider;
};
} // namespace
Model readGLTF(const ::pvr::Stream& stream, const IAssetProvider& assetProvider)
{
	Model asset;
	readGLTF(stream, assetProvider, asset);
	return asset;
}
void readGLTF(const ::pvr::Stream& stream, const IAssetProvider& assetProvider, Model& asset)
{
	/// IMPLEMENTATION NOTES
	// Mesh: GLTF has number of primitives in a mesh and each of those can have different properties, like materials, primitive topology.
	//       Each of the primitives are considered as mesh in the framework.
	//
	tinygltf::Model tinyModel;
	tinygltf::TinyGLTF tinyLoader;
	std::string err;
	std::vector<char> data = stream.readToEnd<char>();
	uint32_t findIndex = static_cast<uint32_t>(stream.getFileName().find_last_of("."));
	std::string ext = stream.getFileName().substr(findIndex, std::string::npos);
	std::string dir;
	pvr::strings::getFileDirectory(stream.getFileName(), dir);

	GltfFileLoader gltfStreamProvider(assetProvider);

	if (!tinyLoader.LoadASCIIFromString(gltfStreamProvider, &tinyModel, &err, static_cast<const char*>(data.data()), static_cast<uint32_t>(data.size()), dir))
	{
		Log("%s", err.c_str());
		throw pvr::FileNotFoundError(err);
	}

	// Count total number of meshes
	uint32_t totalNumMeshes = 0;
	for (size_t m = 0; m < tinyModel.meshes.size(); ++m)
	{
		const tinygltf::Mesh& tinyMesh = tinyModel.meshes[m];
		totalNumMeshes += static_cast<uint32_t>(tinyMesh.primitives.size());
	}

	// Calculate how many nodes required.
	uint32_t numNodes = 0;
	uint32_t numMeshNodes = 0;
	uint32_t numCameraNodes = 0;
	for (size_t m = 0; m < tinyModel.nodes.size(); ++m)
	{
		++numNodes;
		if (tinyModel.nodes[m].mesh >= 0) { numMeshNodes += static_cast<uint32_t>(tinyModel.meshes[tinyModel.nodes[m].mesh].primitives.size()); }
		if (tinyModel.nodes[m].camera >= 0) { ++numCameraNodes; }
	}

	// allocate meshes and nodes.
	asset.allocMeshes(totalNumMeshes);
	asset.allocMeshNodes(numMeshNodes);
	asset.allocNodes(numMeshNodes + numNodes + numCameraNodes);

	// Parse all the meshes
	// Keep a list which maps between the gltf mesh with the framework meshes.
	// For each gltf meshes there must be at least 1 or more (more than one primitives) framework meshes.
	std::vector<MeshprimitivesIterator> meshPrimitives(tinyModel.meshes.size());
	parseAllMesh(tinyModel, asset, meshPrimitives);

	uint32_t cameraNodeIndex = 0;

	// parse the nodes
	uint32_t nodeIndex = asset.getNumMeshNodes();
	uint32_t meshNodeIndex = 0;
	std::vector<NodeMapping> nodeMappings(tinyModel.nodes.size());
	// start from the root node and recursively parse the sub nodes.
	std::vector<bool> processedNodes(tinyModel.nodes.size(), false);
	for (uint32_t scene = 0; scene < tinyModel.scenes.size(); ++scene)
	{
		const tinygltf::Scene& tinyScene = tinyModel.scenes[scene];
		for (uint32_t rootNode = 0; rootNode < tinyScene.nodes.size(); ++rootNode)
		{ parseNode(tinyModel, tinyScene.nodes[rootNode], meshPrimitives, nodeIndex, -1, meshNodeIndex, cameraNodeIndex, asset, nodeMappings, processedNodes); }
	}

	//  Animation
	parseAllAnimation(tinyModel, asset, nodeMappings);

	// Texture and materials
	parseAllTextureAndMaterials(tinyModel, asset);

	// Skins
	parseAllSkins(tinyModel, asset);

	// Cameras
	parseAllCameras(tinyModel, asset);
} // namespace assets
} // namespace assets
} // namespace pvr
