/*!
\brief Defines a simple mechanism for handling key frames (time based).
\file PVRCore/cameras/CameraKeyFrame.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <vector>
#include "PVRCore/glm.h"

namespace pvr {
/// <summary>A simple structure for handling key camera frames</summary>
struct CameraKeyFrame
{
	/// <summary>The key frame in ms</summary>
	uint64_t frameMs;
	/// <summary>The camera key frame position</summary>
	glm::vec3 position;
	/// <summary>The camera key frame orienation</summary>
	glm::vec2 orientation;

	/// <summary>Constructor</summary>
	/// <param name="frameMs">The key frame in ms</param>
	/// <param name="pos">The camera key frame position</param>
	/// <param name="orientation">The camera key frame orienation</param>
	CameraKeyFrame(uint64_t frameMs, const glm::vec3& pos, const glm::vec2& orientation)
	{
		this->frameMs = frameMs;
		position = pos;
		this->orientation = orientation;
	}

	/// <summary>Default Constructor</summary>
	CameraKeyFrame() : frameMs(0), position(glm::vec3(0.0f)), orientation(glm::vec2(0.0f)) {}
};

/// <summary>A simple structure for controlling camera animations</summary>
struct CameraAnimationController
{
	/// <summary>Default Constructor</summary>
	CameraAnimationController()
	{
		restart();
		totalKeyFrameMs = 0;
	}

	/// <summary>Restarts the camera animation controller</summary>
	void restart()
	{
		startKeyFrame = 0;
		endKeyFrame = 1;
		globalMs = 0;
		localMs = 0;
	}

	/// <summary>Sets the current total time in MS</summary>
	/// <param name="totalMs">The current tile in milli seconds</param>
	void setTotalTimeInMs(uint64_t totalMs) { totalKeyFrameMs = totalMs; }

	/// <summary>Advances the current total time in MS</summary>
	/// <param name="dt">The number of milli seconds to advance the time by</param>
	void advanceTime(uint64_t dt)
	{
		globalMs += dt;
		localMs += dt;
		if (localMs > totalKeyFrameMs)
		{
			localMs = localMs % totalKeyFrameMs; // wrap
			startKeyFrame = 0;
			endKeyFrame = 1;
		}
		while (localMs > keyframes[endKeyFrame].frameMs)
		{
			startKeyFrame = endKeyFrame;
			endKeyFrame += 1;
		}
	}

	/// <summary>Retrieves the position for the current time</summary>
	/// <returns>The camera key frame position based on the current time</returns>
	glm::vec3 getPosition() const
	{
		const glm::vec3& pos0 = keyframes[startKeyFrame].position;
		const glm::vec3& pos1 = keyframes[endKeyFrame].position;

		float s = localMs - keyframes[startKeyFrame].frameMs;
		s /= (keyframes[endKeyFrame].frameMs - keyframes[startKeyFrame].frameMs);
		return pos0 + (s * (pos1 - pos0));
	}

	/// <summary>Retrieves the orientation for the current time</summary>
	/// <returns>The camera key frame orientation based on the current time</returns>
	glm::vec2 getOrientation() const
	{
		const glm::vec2 rot0 = keyframes[startKeyFrame].orientation;
		const glm::vec2 rot1 = keyframes[endKeyFrame].orientation;

		float s = localMs - keyframes[startKeyFrame].frameMs;
		s /= (keyframes[endKeyFrame].frameMs - keyframes[startKeyFrame].frameMs);
		return glm::vec2(rot0.x + (s * (rot1.x - rot0.x)), rot0.y + (s * (rot1.y - rot0.y)));
	}

	/// <summary>Retrieves the current beginning of the key frame for the current time</summary>
	/// <returns>The current beginning of the key frame for the current time</returns>
	uint32_t getCurrentBeginKeyFrame() const { return startKeyFrame; }

	/// <summary>Retrieves the current end of the key frame for the current time</summary>
	/// <returns>The current end of the key frame for the current time</returns>
	uint32_t getCurrentEndKeyFrame() const { return endKeyFrame; }

	/// <summary>Retrieves the number of key frames</summary>
	/// <returns>The number of key frames currently being controlled</returns>
	uint64_t getNumKeyFrames() const { return keyframes.size(); }

	/// <summary>Adds a new key frame</summary>
	/// <param name="keyFrames">A pointer to a list of key frames</param>
	/// <param name="numKeyFrames">The number of key frames pointed to by keyFrames</param>
	void addKeyFrames(const CameraKeyFrame* keyFrames, uint32_t numKeyFrames)
	{
		this->keyframes.reserve(numKeyFrames + this->keyframes.size());
		for(uint32_t i = 0; i < numKeyFrames; ++i) { this->keyframes.emplace_back(keyFrames[i]); }

		totalKeyFrameMs = std::max(totalKeyFrameMs, this->keyframes.back().frameMs);
	}

	/// <summary>The current set of key frames</summary>
	std::vector<CameraKeyFrame> keyframes;
	/// <summary>The start of the key frames</summary>
	uint32_t startKeyFrame;
	/// <summary>The end of the key frames</summary>
	uint32_t endKeyFrame;
	/// <summary>The global ms</summary>
	uint64_t globalMs;
	/// <summary>The local ms</summary>
	uint64_t localMs;
	/// <summary>The total key frame ms</summary>
	uint64_t totalKeyFrameMs;
};
} // namespace pvr
