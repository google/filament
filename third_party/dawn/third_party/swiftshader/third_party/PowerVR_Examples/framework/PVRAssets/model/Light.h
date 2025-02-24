/*!
\brief Represents a Light in the scene (Model).
\file PVRAssets/model/Light.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/math/MathUtils.h"
namespace pvr {
namespace assets {
/// <summary>Represents a Light source in the scene.</summary>
class Light
{
public:
	/// <summary>The type of the light</summary>
	enum LightType
	{
		Point = 0, //!< Point light
		Directional, //!< Directional light
		Spot, //!< Spot light

		NumLightTypes //!< number of supported light type
	};

	/// <summary>Raw internal structure of the Light.</summary>
	struct InternalData
	{
		//------------- What is the targetindex?

		int32_t spotTargetNodeIdx; /*!< Index of the target object */ // Should this be a point to the actual node?
		glm::vec3 color; /*!< Light color (0.0f -> 1.0f for each channel) */
		LightType type; /*!< Light type (point, directional, spot etc.) */
		float constantAttenuation; /*!< Constant attenuation */
		float linearAttenuation; /*!< Linear attenuation */
		float quadraticAttenuation; /*!< Quadratic attenuation */
		float falloffAngle; /*!< Falloff angle (in radians) */
		float falloffExponent; /*!< Falloff exponent */

		InternalData()
			: spotTargetNodeIdx(-1), type(Light::Point), constantAttenuation(1.0f), linearAttenuation(0.0f), quadraticAttenuation(0.0f), falloffAngle(glm::pi<float>()),
			  falloffExponent(0.0f)
		{
			color[0] = color[1] = color[2] = 1.0f;
		}
	};

public:
	/// <summary>Get the node ID of the target of a light with a direction.</summary>
	/// <returns>The Node ID of the target of a light with a direction</returns>
	int32_t getTargetIdx() const;

	/// <summary>Get light color.</summary>
	/// <returns>RGB color triplet in a glm::vec3</returns>
	const glm::vec3& getColor() const;

	/// <summary>Get light type (spot, point, directional).</summary>
	/// <returns>The light type</returns>
	LightType getType() const;

	/// <summary>Get the Constant attenuation of a spot or point light.</summary>
	/// <returns>The Constant attenuation</returns>
	float getConstantAttenuation() const;

	/// <summary>Get the Linear attenuation of a spot or point light.</summary>
	/// <returns>The Linear attenuation</returns>
	float getLinearAttenuation() const;

	/// <summary>Get the Quadratic attenuation of a spot or point light. (If gamma correct,
	/// quadratic attenuation should be the closest approximation to physically correct).</summary>
	/// <returns>The Quadratic attenuation</returns>
	float getQuadraticAttenuation() const;

	/// <summary>Get the Falloff angle of a spot light. (minimum angle where penumbra starts)</summary>
	/// <returns>The fallof angle</returns>
	float getFalloffAngle() const;

	/// <summary>Get the Falloff exponent of a spot light. (number defining who fast the falloff is)</summary>
	/// <returns>The fallof exponent</returns>
	float getFalloffExponent() const;

	/// <summary>Set a Target for a spot light. (The spotlight will be always "looking" at the target</summary>
	/// <param name="idx">The node index of the target of the spotlight.</param>
	void setTargetNodeIdx(int32_t idx);

	/// <summary>Set light color.</summary>
	/// <param name="r">Red color channel ([0..1])</param>
	/// <param name="g">Green color channel ([0..1])</param>
	/// <param name="b">Blue color channel ([0..1])</param>
	void setColor(float r, float g, float b);

	/// <summary>Set light type.</summary>
	/// <param name="t">The type of the light</param>
	void setType(LightType t);
	/// <summary>Set constant attenuation.</summary>
	/// <param name="c">Constant attenuation factor</param>
	void setConstantAttenuation(float c);
	/// <summary>Set linear attenuation.</summary>
	/// <param name="l">Linear attenuation factor</param>
	void setLinearAttenuation(float l);
	/// <summary>Set Quadratic attenuation.</summary>
	/// <param name="q">Quadratic attenuation factor</param>
	void setQuadraticAttenuation(float q);
	/// <summary>Set spot Falloff angle. This is the angle inside of which the spotlight is full strength.</summary>
	/// <param name="fa">Falloff angle</param>
	void setFalloffAngle(float fa);

	/// <summary>Set a spot Falloff exponent.</summary>
	/// <param name="fe">Falloff exponent</param>
	void setFalloffExponent(float fe);

	/// <summary>Get a reference to the internal representation of this object. Handle with care.</summary>
	/// <returns>A reference to the internal representation of this object</returns>
	InternalData& getInternalData(); // If you know what you're doing

private:
	InternalData _data;
};
} // namespace assets
} // namespace pvr
