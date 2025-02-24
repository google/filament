/*!
\brief Implementations of methods from the Model class.
\file PVRAssets/model/Model.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/Model.h"
#include "PVRAssets/model/Camera.h"
#include "PVRAssets/model/Light.h"
#include "PVRAssets/model/Mesh.h"
#include "PVRCore/stream/Stream.h"
#include "glm/gtx/quaternion.hpp"
namespace pvr {
namespace assets {
void Model::allocCameras(uint32_t no) { _data.cameras.resize(no); }

void Model::allocLights(uint32_t no) { _data.lights.resize(no); }

void Model::allocMeshes(uint32_t no) { _data.meshes.resize(no); }

void Model::allocNodes(uint32_t no) { _data.nodes.resize(no); }

void Model::allocMeshNodes(uint32_t no)
{
	allocNodes(no);
	_data.numMeshNodes = no;
}

glm::mat4x4 Model::getBoneWorldMatrix(uint32_t skinNodeId, uint32_t boneIndex) const
{
	// Back transform bone from frame 0 position using the skin's transformation
	const Mesh& mesh = getMesh(getNode(skinNodeId).getObjectId());
	debug_assertion(mesh.getSkeletonId() >= 0, "Invalid Skeleton index");
	const Skeleton& skeleton = getSkeleton(static_cast<uint32_t>(mesh.getSkeletonId()));

	const Node::InternalData& nodeData = getNode(skinNodeId).getInternalData();
	glm::mat4 nodeWorld(1.f);
	if (nodeData.transformFlags & pvr::assets::Node::InternalData::TransformFlags::SRT)
	{ nodeWorld = pvr::math::constructSRT(nodeData.getScale(), nodeData.getRotate(), nodeData.getTranslation()); }
	else if (nodeData.transformFlags == pvr::assets::Node::InternalData::TransformFlags::Matrix)
	{
		nodeWorld = *(glm::mat4*)nodeData.frameTransform;
	}
	return getWorldMatrix(skeleton.bones[boneIndex]) * skeleton.invBindMatrices[boneIndex] * nodeWorld;
}

glm::mat4x4 Model::getWorldMatrix(uint32_t id) const
{
	const Node& node = _data.nodes[id];
	uint32_t parentID = _data.nodes[id].getParentID();
	const auto& nodeData = node.getInternalData();

	glm::mat4 srtMatrix = glm::mat4(1.0f);
	if (nodeData.transformFlags == Node::InternalData::TransformFlags::Matrix)
	{
		srtMatrix = *(glm::mat4*)nodeData.frameTransform;
		debug_assertion(!nodeData.hasAnimation, "Node cannot have transformation matrix and animation data");
	}
	else if (nodeData.hasAnimation)
	{
		debug_assertion(nodeData.transformFlags & Node::InternalData::TransformFlags::SRT, "Animation data must be stores as SRT");
		srtMatrix = pvr::math::constructSRT(nodeData.getFrameScaleAnimation(), nodeData.getFrameRotationAnimation(), nodeData.getFrameTranslationAnimation());
	}
	else if ((nodeData.transformFlags & Node::InternalData::TransformFlags::SRT))
	{
		if (nodeData.transformFlags & Node::InternalData::TransformFlags::Scale) { srtMatrix = glm::scale(nodeData.getScale()); }
		if (nodeData.transformFlags & Node::InternalData::TransformFlags::Rotate) { srtMatrix = glm::toMat4(nodeData.getRotate()) * srtMatrix; }
		if (nodeData.transformFlags & Node::InternalData::TransformFlags::Translate) { srtMatrix = glm::translate(nodeData.getTranslation()) * srtMatrix; }
	}

	// Concatenate with parent transformation if one exist.
	if (parentID == static_cast<uint32_t>(-1)) { return srtMatrix; }
	else
	{
		return getWorldMatrix(parentID) * srtMatrix;
	}
}

glm::vec3 Model::getLightPosition(uint32_t lightNodeId) const { return glm::vec3(getWorldMatrix(getNodeIdFromLightNodeId(lightNodeId))[3]); }

float Model::getCurrentFrame() { return _data.currentFrame; }

void Model::setUserData(uint32_t size, const char* const data)
{
	_data.userData.resize(data ? size : 0);
	if (data && size) { memcpy(_data.userData.data(), data, size); }
}

void Model::getCameraProperties(uint32_t index, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float& nearClip, float& farClip, float frameTimeInMs) const
{
	if (static_cast<uint32_t>(index) >= _data.cameras.size())
	{
		Log(LogLevel::Error, "Model::getCameraProperties out of bounds [%d]", index);
		assertion(0);
		return;
	}
	nearClip = _data.cameras[index].getNear();
	farClip = _data.cameras[index].getFar();
	return getCameraProperties(index, fov, from, to, up, frameTimeInMs);
}

void Model::getCameraProperties(uint32_t index, float& fov, glm::vec3& from, glm::vec3& to, glm::vec3& up, float frameTimeInMs) const
{
	if (static_cast<uint32_t>(index) >= _data.cameras.size())
	{
		assertion(0, "Model::getCameraProperties index out of range");
		Log(LogLevel::Error, "Model::getCameraProperties out of bounds [%d]", index);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(static_cast<uint32_t>(_data.numMeshNodes + _data.lights.size() + index));
	// View position is 0, 0, 0, 1 transformed by world matrix
	from.x = matrix[3][0];
	from.y = matrix[3][1];
	from.z = matrix[3][2];
	// When you rotate the camera from "straight forward" to "straight down", in openGL the UP vector will be [0, 0, -1]
	up.x = -matrix[2][0];
	up.y = -matrix[2][1];
	up.z = -matrix[2][2];
	up = glm::normalize(up);
	const Camera& camera = getCamera(index);

	if (camera.getTargetNodeIndex() != -1)
	{
		glm::vec3 atCurrent, atTarget;
		glm::mat4x4 targetMatrix = getWorldMatrix(camera.getTargetNodeIndex());
		to.x = targetMatrix[3][0];
		to.y = targetMatrix[3][1];
		to.z = targetMatrix[3][2];

		// Rotate our up vector
		atTarget = to - from;
		glm::normalize(atTarget);
		atCurrent = to - from;
		glm::normalize(atCurrent);
		glm::vec3 axis = glm::cross(atCurrent, atTarget);
		float angle = glm::dot(atCurrent, atTarget);
		glm::quat q = glm::angleAxis(angle, axis);
		up = glm::mat3_cast(q) * up;
		glm::normalize(up);
	}
	else
	{
		// View direction is 0, -1, 0, 1 transformed by world matrix
		to.x = -matrix[1][0] + from.x;
		to.y = -matrix[1][1] + from.y;
		to.z = -matrix[1][2] + from.z;
	}
	fov = camera.getFOV(frameTimeInMs);
}

void Model::getLightDirection(uint32_t lightNodeId, glm::vec3& direction) const
{
	if (static_cast<size_t>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightDirection out of bounds");
		Log(LogLevel::Error, "Model::getLightDirection out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	const Light& light = getLight(lightNodeId);
	int32_t targetIndex = light.getTargetIdx();
	if (targetIndex != -1)
	{
		glm::mat4x4 targetMatrix = getWorldMatrix(targetIndex);
		direction.x = targetMatrix[3][0] - matrix[3][0];
		direction.y = targetMatrix[3][1] - matrix[3][1];
		direction.z = targetMatrix[3][2] - matrix[3][2];
		glm::normalize(direction);
	}
	else
	{
		direction.x = -matrix[1][0];
		direction.y = -matrix[1][1];
		direction.z = -matrix[1][2];
	}
}

void Model::getLightPosition(uint32_t lightNodeId, glm::vec3& position) const
{
	if (static_cast<uint32_t>(lightNodeId) >= getNumLightNodes())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(LogLevel::Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
}

void Model::getLightPosition(uint32_t lightNodeId, glm::vec4& position) const
{
	if (static_cast<uint32_t>(lightNodeId) >= _data.lights.size())
	{
		assertion(0, "Model::getLightPosition out of bounds");
		Log(LogLevel::Error, "Model::getLightPosition out of bounds [%d]", lightNodeId);
		assertion(0);
		return;
	}
	glm::mat4x4 matrix = getWorldMatrix(_data.numMeshNodes + lightNodeId);
	position.x = matrix[3][0];
	position.y = matrix[3][1];
	position.z = matrix[3][2];
	position.w = 1.0f;
}
} // namespace assets
} // namespace pvr
//!\endcond
