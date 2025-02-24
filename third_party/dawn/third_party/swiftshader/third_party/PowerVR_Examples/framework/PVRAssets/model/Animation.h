/*!
\brief Contains an Animation class.
\file PVRAssets/model/Animation.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/math/MathUtils.h"

namespace pvr {
namespace assets {
/// <summary>Represents an Animation that can be applied to different objects.</summary>
struct KeyFrameData
{
	/// <summary>Specifies the type of interpolation to use between neighbouring frames of animation data.</summary>
	enum struct InterpolationType : uint32_t
	{
		Step = 0,
		Linear = 1,
		CubicSpline = 2,
	};

	/// <summary>The time in seconds at which the key frame takes plcae.</summary>
	std::vector<float> timeInSeconds;
	/// <summary>Scaling matrix.</summary>
	std::vector<glm::vec3> scale;
	/// <summary>Rotation matrix.</summary>
	std::vector<glm::quat> rotate;
	/// <summary>Translation matrix.</summary>
	std::vector<glm::vec3> translation;
	/// <summary>Matrix data - note that this is only currently used by POD.</summary>
	std::vector<glm::mat4> mat4;
	/// <summary>The interpolation used.</summary>
	InterpolationType interpolation = InterpolationType::Step;
};

/// <summary>Specifies animation data.</summary>
class AnimationData
{
public:
	/// <summary>Raw internal structure of the Animation.</summary>
	struct InternalData
	{
		// Since the size is shared by all of those items, we are eschewing the use of vectors in order to save the extra space of size and capacity.
		uint32_t flags; //!< Stores which animation arrays are stored

		// Indices: If you will have loads of repeated value
		std::vector<uint32_t> positionIndices; //!< Index to positions
		std::vector<uint32_t> rotationIndices; //!< Index to rotations
		std::vector<uint32_t> scaleIndices; //!< Index to scales
		std::vector<uint32_t> matrixIndices; //!< Index to matrices

		uint32_t numFrames; //!< The number of frames of animation

		std::string animationName; //!< The name of the animation

		std::vector<float> timeInSeconds; //!< The time is seconds at which the animation occurs

		std::vector<KeyFrameData> keyFrames; //!< Specifies key frame animation data

		float durationTime; //!< Total durration time of this animation

		/// <summary>Constructor. Initializing.</summary>
		InternalData() : flags(0), numFrames(0), durationTime(0.0f) {}
	};

public:
	/// <summary>Constructor..</summary>
	AnimationData() : _cacheF1(0), _cacheF2(1) {}

	/// <summary>Setter for the name of the animation.</summary>
	/// <param name="animationName">The name of the animation</param>
	void setAnimationName(const std::string& animationName) { _data.animationName = animationName; }

	/// <summary>Getter for the name of the animation.</summary>
	/// <returns>The name of the animation</returns>
	const std::string& getAnimationName() const { return _data.animationName; }

	/// <summary>Getter for the number of key frames.</summary>
	/// <returns>The number of key frames</returns>
	size_t getNumKeyFrames() const { return _data.keyFrames.size(); }

	/// <summary>Allocates a number of key frames.</summary>
	/// <param name="keyFrames">The number of key frames to allocate</param>
	void allocateKeyFrames(uint32_t keyFrames) { _data.keyFrames.resize(keyFrames); }

	/// <summary>Getter animation data for a specified key frame.</summary>
	/// <param name="index">The key frames for which to retrieve animation data</param>
	/// <returns>The key frame data for the given frame</returns>
	KeyFrameData& getAnimationData(uint32_t index) { return _data.keyFrames[index]; }

	/// <summary>Getter for the total time taken for the animation in seconds</summary>
	/// <returns>The total time in seconds taken for the animation in seconds</returns>
	float getTotalTimeInSec() { return _data.durationTime; }

	/// <summary>Getter for the total time taken for the animation in milli seconds</summary>
	/// <returns>The total time in seconds taken for the animation in milli seconds</returns>
	float getTotalTimeInMs() { return getTotalTimeInSec() * 1000.f; }

	/// <summary>Get the transformation matrix of specific frame and amount of interpolation.</summary>
	/// <param name="frame">The first frame for which the transformation matrix will be returned</param>
	/// <param name="interp">Interpolation value used between the frames</param>
	/// <returns>The transformation matrix for the point in time that is at point <paramref name="interp"/>between
	/// frame <paramref name="frame"/>and frame <paramref name="frame+1"/></returns>
	/// <remarks>If the animation consists of Transformation Matrices, they will NOT be interpolated as this would be
	/// a very expensive operation. Rather, the closest matrix will be returned. If the transformation consists of
	/// Scale/ translation vectors and Rotation quaternia, Scale and Translation will be Linear Interpolated, and
	/// Rotation will be SLERPed (Smooth Linear Interpolation) as normal.</remarks>
	glm::mat4x4 getTransformationMatrix(uint32_t frame = 0, float interp = 0) const;

	/// <summary>Get number of frames in this animation.</summary>
	/// <returns>The number of frames</returns>
	uint32_t getNumFrames() const;

	/// <summary>Get the flags data on this animation.</summary>
	/// <returns>The flags</returns>
	uint32_t getFlags() const;

	/// <summary>Get a pointer to the position data of this animation.</summary>
	/// <returns>Position data - null if indexed</returns>
	const float* getPositions() const;

	/// <summary>Get a pointer to the indices of the position data of this animation.</summary>
	/// <returns>Position indexes - null if not indexed</returns>
	const uint32_t* getPositionIndices() const;

	/// <summary>Get a pointer to the rotation data of this animation (normally quaternions).</summary>
	/// <returns>Rotation data - null if indexed</returns>
	const float* getRotations() const;

	/// <summary>Get a pointer to the indices of the rotation data of this animation.</summary>
	/// <returns>Rotation indexes - null if not indexed</returns>
	const uint32_t* getRotationIndices() const;

	/// <summary>Get a pointer to the Scale data of this animation.</summary>
	/// <returns>Scales - null if indexed</returns>
	const float* getScales() const;

	/// <summary>Get a pointer to the indices of the Scale data.</summary>
	/// <returns>Scale indexes - null if not indexed</returns>
	const uint32_t* getScaleIndices() const;

	/// <summary>Get a pointer to the transformation matrices of this animation.</summary>
	/// <returns>Matrices - null if indexed</returns>
	const float* getMatrices() const;

	/// <summary>Get a pointer to the indices of the transformation matrices of this animation.</summary>
	/// <returns>Matrix indexes - null if not indexed</returns>
	const uint32_t* getMatrixIndices() const;

	/// <summary>Set the position transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The position data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation</returns>
	void setPositions(uint32_t numFrames, const float* data,
		const uint32_t* indices = nullptr); // Expects an array of 3 floats

	/// <summary>Set the rotation transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 4 of which
	/// will be interpreted as x,y,z,w quaternion values.</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation</returns>
	void setRotations(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 4 floats

	/// <summary>Set the scale transformation data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The rotation data that will be copied. Must be packed floats, each successive 3 of which
	/// will be interpreted as x,y,z scale factors</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation</returns>
	void setScales(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 7 floats

	/// <summary>Set the transformation matrices data for this animation.</summary>
	/// <param name="numFrames">The number of frames of animation to set</param>
	/// <param name="data">The Transformation Matrices data that will be copied. Must be packed floats, each
	/// successive 16 of which will be interpreted as a matrix packed in the usual way (Column-major, with each column
	/// of the matrix stored successively in</param>
	/// <param name="indices">If this array is not NULL, the position data will be indexed. Default NULL.</param>
	/// <returns>True on success, false if passing the wrong amount of data for the number of frames in the animation</returns>
	void setMatrices(uint32_t numFrames, const float* const data,
		const uint32_t* const indices = nullptr); // Expects an array of 16 floats

	/// <summary>Gets a direct, modifiable pointer to the data representation of this object. Advanced tasks only.</summary>
	/// <returns>A pointer to the internal structure of this object</returns>
	InternalData& getInternalData(); // If you know what you're doing

private:
	InternalData _data;
	// cache
	uint32_t _cacheF1, _cacheF2;
};

/// <summary>A specific instance of an animation.</summary>
struct AnimationInstance
{
	/// <summary>Key frame channels.</summary>
	struct KeyframeChannel
	{
		std::vector<void*> nodes; //!< Nodes of the animation

		uint32_t keyFrame; //!< keyframe (Scale/ Rotate/ Translate)

		/// <summary>Constructor.</summary>
		KeyframeChannel() : keyFrame(0) {}
	};

	class AnimationData* animationData; //!< Animation data
	std::vector<KeyframeChannel> keyframeChannels; //!< Key frame data

public:
	/// <summary>Constructor.</summary>
	AnimationInstance() : animationData(nullptr) {}

	/// <summary>Retrieves the time in milli seconds at which the animation will occur.</summary>
	/// <returns>The time in milli seconds at which the animation will occur</returns>
	float getTotalTimeInMs() const { return animationData->getTotalTimeInMs(); }

	/// <summary>Retrieves the time in seconds at which the animation will occur.</summary>
	/// <returns>The time in seconds at which the animation will occur</returns>
	float getTotalTimeInSec() const { return animationData->getTotalTimeInSec(); }

	/// <summary>update animation</summary>
	/// <param name="timeInMs">The time in milli seconds to set for the animation</param>
	void updateAnimation(float timeInMs);
};

} // namespace assets
} // namespace pvr
