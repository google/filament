/*!
\brief Implementations of methods of the Animation class.
\file PVRAssets/model/Animation.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/Model.h"
#include "PVRAssets/model/Animation.h"
#include "PVRCore/Errors.h"
#include "PVRCore/math/MathUtils.h"
#include "PVRCore/strings/StringFunctions.h"
namespace pvr {
namespace assets {

uint32_t AnimationData::getNumFrames() const { return _data.numFrames; }

const uint32_t* AnimationData::getPositionIndices() const { return _data.positionIndices.data(); }

const uint32_t* AnimationData::getRotationIndices() const { return _data.rotationIndices.data(); }

const uint32_t* AnimationData::getScaleIndices() const { return _data.scaleIndices.data(); }

const uint32_t* AnimationData::getMatrixIndices() const { return _data.matrixIndices.data(); }

uint32_t AnimationData::getFlags() const { return _data.flags; }

AnimationData::InternalData& AnimationData::getInternalData() { return _data; }

void AnimationInstance::updateAnimation(float time)
{
	time *= 0.001f; // ms to sec.
	for (uint32_t i = 0; i < keyframeChannels.size(); ++i)
	{
		KeyframeChannel& keyframeNodes = keyframeChannels[i];
		KeyFrameData& keyFrame = animationData->getInternalData().keyFrames[keyframeNodes.keyFrame];

		// find the time slice.
		uint32_t f1 = 0, f2 = 0;
		float t = 0.0f;

		// TODO: OPTIMZE FOR FAST PATH
		KeyFrameData::InterpolationType interp = keyFrame.interpolation;
		if (time <= keyFrame.timeInSeconds[0]) { interp = KeyFrameData::InterpolationType::Step; }
		else if (time >= keyFrame.timeInSeconds.back())
		{
			f1 = static_cast<uint32_t>(keyFrame.timeInSeconds.size()) - 1;
			f2 = f1;
			interp = KeyFrameData::InterpolationType::Step;
		}
		else
		{
			f2 = 0;
			// find which f2 includes the time.
			for (; f2 < keyFrame.timeInSeconds.size() && keyFrame.timeInSeconds[f2] < time; ++f2)
				;
			f1 = f2 - 1;
			t = (time - keyFrame.timeInSeconds[f1]) / (keyFrame.timeInSeconds[f2] - keyFrame.timeInSeconds[f1]);
		}

		//----------------------------
		// SRT
		if (keyFrame.scale.size())
		{
			glm::vec3 scale(1.0f);
			if (interp == KeyFrameData::InterpolationType::Step) { scale = keyFrame.scale[f1]; }
			else if (interp == KeyFrameData::InterpolationType::Linear)
			{
				scale = keyFrame.scale[f1] * (1.f - t) + keyFrame.scale[f2] * t;
			}

			// animate all the nodes.
			for (uint32_t nodeId = 0; nodeId < keyframeNodes.nodes.size(); ++nodeId)
			{ static_cast<Node*>(keyframeNodes.nodes[nodeId])->getInternalData().getFrameScaleAnimation() = scale; }
		}
		else if (keyFrame.rotate.size())
		{
			glm::quat quat = glm::quat();
			if (interp == KeyFrameData::InterpolationType::Step) { quat = keyFrame.rotate[f1]; }
			else if (interp == KeyFrameData::InterpolationType::Linear)
			{
				quat = glm::slerp(keyFrame.rotate[f1], keyFrame.rotate[f2], t);
			}

			// animate all the node.
			for (uint32_t nodeId = 0; nodeId < keyframeNodes.nodes.size(); ++nodeId)
			{ static_cast<Node*>(keyframeNodes.nodes[nodeId])->getInternalData().getFrameRotationAnimation() = quat; }
		}

		else if (keyFrame.translation.size())
		{
			glm::vec3 trans(0.0f);
			if (interp == KeyFrameData::InterpolationType::Step) { trans = keyFrame.translation[f1]; }
			else if (interp == KeyFrameData::InterpolationType::Linear)
			{
				trans = keyFrame.translation[f1] * (1.f - t) + keyFrame.translation[f2] * t;
			}

			// animate all the node.
			for (uint32_t ii = 0; ii < keyframeNodes.nodes.size(); ++ii)
			{
				Node& n = *static_cast<Node*>(keyframeNodes.nodes[ii]);
				pvr::assets::Node::InternalData& internalData = n.getInternalData();
				internalData.getFrameTranslationAnimation() = trans;
			}
		}

		else if (keyFrame.mat4.size())
		{
			glm::mat4 transX = keyFrame.mat4[f1];
			// animate all the node.
			for (uint32_t ii = 0; ii < keyframeNodes.nodes.size(); ++ii)
			{
				pvr::assets::Node::InternalData& internalData = static_cast<Node*>(keyframeNodes.nodes[ii])->getInternalData();
				glm::mat4 srtMatrix = pvr::math::constructSRT(internalData.getScale(), internalData.getRotate(), internalData.getTranslation());
				srtMatrix = transX * srtMatrix;
				memcpy(internalData.frameTransform, glm::value_ptr(srtMatrix), sizeof(glm::mat4));
			}
		}
	}
}

} // namespace assets
} // namespace pvr
//!\endcond
