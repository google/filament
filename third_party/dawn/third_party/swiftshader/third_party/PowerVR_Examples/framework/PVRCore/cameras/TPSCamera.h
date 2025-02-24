/*!
\brief A class representing a Third person camera and functionality to manipulate it.
\file PVRCore/cameras/TPSCamera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/glm.h"

// Third persorn camera
namespace pvr {
/// <summary>A simple third person camera implementation</summary>
class TPSCamera
{
public:
	/// <summary>Default Constructor</summary>
	TPSCamera() : _azimuth(0.0f), offsetY(0.0f), offsetZ(0.0f), _targetPos(0.0f), _updatePosition(true), _updateView(false) {}

	/// <summary>Set the height from the floor</summary>
	/// <param name="height">height</param>
	void setHeight(float height)
	{
		offsetY = height;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Set the camera distance from the target</summary>
	/// <param name="dist">Distance</param>
	void setDistanceFromTarget(const float dist)
	{
		offsetZ = dist;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Set the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="targetPos">Target position</param>
	void setTargetPosition(const glm::vec3& targetPos)
	{
		_targetPos = glm::vec3(0.f);
		updateTargetPosition(targetPos);
	}
	/// <summary>Get the camera target position i.e. the 'lookat' value.</summary>
	/// <returns>The target's position</returns>
	const glm::vec3& getTargetPosition() { return _targetPos; }

	/// <summary>Update the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="pos">The targets new position</param>
	void updateTargetPosition(const glm::vec3& pos)
	{
		_targetPos += pos;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Update the camera target look angle.</summary>
	/// <param name="angleDeg">The camera's new angle in degrees</param>
	void updateTargetLookAngle(float angleDeg)
	{
		_azimuth += angleDeg;
		_updatePosition = true;
		_updateView = true;
	}
	/// <summary>Set the camera target look angle. In this implementation, angle 0 means the camera is facing the north.</summary>
	/// <param name="angleDeg">The camera's new angle in degrees</param>
	void setTargetLookAngle(float angleDeg)
	{
		_azimuth = 0.0f;
		updateTargetLookAngle(angleDeg);
	}

	/// <summary>Get the camera target look angle. Tn this implementation, angle 0 means the camera is facing the north.</summary>
	/// <returns>The camera's angle in degrees</returns>
	float getTargetLookAngle() { return _azimuth; }

	/// <summary>Calculates and returns the camera view matrix based on the most up to date camera properties if they have been updated (are dirty) or returns the last view matrix
	/// caluclated.</summary>
	/// <returns>The TPS camera view matrix</returns>
	const glm::mat4& getViewMatrix() const
	{
		const glm::vec3& cameraPos = getCameraPosition();
		// Construct the matrix and return
		if (_updateView)
		{
			_updateView = false;
			// rotate the position of the camera in
			_viewX = glm::lookAt(cameraPos, _targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		return _viewX;
	}

	/// <summary>Calculates and returns the camera position based on the most up to date camera properties.</summary>
	/// <returns>The TPS camera position</returns>
	const glm::vec3& getCameraPosition() const
	{
		// Construct the matrix and return
		if (_updatePosition)
		{
			_updatePosition = false;
			_updateView = true;
			// this makes the camera aligned behind the target and offset it by 90 degree because our initial axis start from north.
			float rotation = _azimuth + 180.f + 90.f;

			glm::vec3 dir = glm::mat3(glm::rotate(glm::radians(rotation), glm::vec3(0.0, 1.f, 0.0))) * glm::vec3(1.0f, 0.0f, 0.0f);
			_cameraPos = dir * offsetZ + _targetPos;
			_cameraPos.y = offsetY;
		}
		return _cameraPos;
	}

private:
	float _azimuth;
	float offsetY, offsetZ; // Camera offset
	glm::vec3 _targetPos; // Character
	mutable glm::vec3 _cameraPos;
	mutable glm::mat4 _viewX;
	mutable bool _updatePosition, _updateView;
};

/// <summary>A simple third person orbit camera implementation</summary>
class TPSOrbitCamera
{
/// <summary>A small value which acts as the minimum amount which can be used for inclination and distance to the object being observed</summary>
#define epsilon 1e-5f

public:
	/// <summary>Default Constructor for a TPSOrbitCamera</summary>
	TPSOrbitCamera() : _azimuth(0.0f), _inclination(0.0f), _targetPos(0.0f), _updatePosition(true), _updateView(false) {}

	/// <summary>Get the azimuth of the camera around the target (angle around the Z axis). Angle zero is on the X axis.</summary>
	/// <returns>The camera's azimuth in degrees</returns>
	float getAzimuth() { return _azimuth; }

	/// <summary>Set the azimuth of the camera along the target's XZ plane. 0 is on the X axis.</summary>
	/// <param name="azimuth">The azimuth of the camera along the target's XZ plane</param>
	void setAzimuth(float azimuth)
	{
		while (azimuth < -180.f) { azimuth += 360.f; }
		while (azimuth > 180.f) { azimuth -= 360.f; }
		_azimuth = azimuth;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Increase or decrease the azimuth of the camera by a (positive or negative) delta amount.</summary>
	/// <param name="deltaAzimuth">The increase or decrease in azimuth</param>
	void addAzimuth(float deltaAzimuth) { setAzimuth(_azimuth + deltaAzimuth); }

	/// <summary>Get the inclination of the camera (vertical angle). 90 is top, -90 is bottom, 0 is horizontal.</summary>
	/// <returns>The camera's inclination in the range of -90..90</returns>
	float getInclination() { return _inclination; }

	/// <summary>Set the vertical angle of the camera. 0 is on the target's XZ plane, -90 is below the target, 90 is above the target. Values over 90 or under -90 are
	/// clamped.</summary>
	/// <param name="inclination">height</param>
	void setInclination(float inclination)
	{
		if (inclination > 90 - epsilon) { inclination = 90 - epsilon; }
		if (inclination < -90 + epsilon) { inclination = -90 + epsilon; }
		_inclination = inclination;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Increase or decrease the inclination of the camera by a (positive or negative) delta amount. Resulting inclinations smaller than (-90 + epsilon)
	/// or greater than (90-epsilon) are clamped.</summary>
	/// <param name="deltaInclination">An inclination to add to the vertical angle of the camera.</param>
	void addInclination(float deltaInclination) { setInclination(_inclination + deltaInclination); }

	/// <summary>Set the camera distance from the target</summary>
	/// <param name="distance">The distance from the target</param>
	void setDistanceFromTarget(float distance)
	{
		if (distance < epsilon) { distance = epsilon; }
		_distance = distance;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Add a delta amount (positive or negative) to the distance of the camera from the target. Resulting distances smaller than epsilon are clamped to epsilon.</summary>
	/// <param name="deltaDistance">The distance from the target</param>
	void addDistanceFromTarget(float deltaDistance) { setDistanceFromTarget(_distance + deltaDistance); }

	/// <summary>Set the camera target position i.e. the 'lookat' value.</summary>
	/// <param name="targetPos">Target position</param>
	void setTargetPosition(const glm::vec3& targetPos)
	{
		_targetPos = targetPos;
		_updatePosition = true;
		_updateView = true;
	}

	/// <summary>Get the camera target position i.e. the 'lookat' value.</summary>
	/// <returns>The target's position</returns>
	const glm::vec3& getTargetPosition() { return _targetPos; }

	/// <summary>Update the camera target position i.e. the 'lookat' value by a delta amount.</summary>
	/// <param name="pos">The targets new position</param>
	void addTargetPosition(const glm::vec3& pos) { setTargetPosition(_targetPos + pos); }

	/// <summary>Calculates and returns the camera view matrix based on the most up to date camera properties if they have been updated (are dirty) or returns the last view matrix
	/// caluclated.</summary>
	/// <returns>The TPS camera view matrix</returns>
	const glm::mat4& getViewMatrix() const
	{
		const glm::vec3& cameraPos = getCameraPosition();
		// Construct the matrix and return
		if (_updateView)
		{
			_updateView = false;
			// rotate the position of the camera in
			_viewX = glm::lookAt(cameraPos, _targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		return _viewX;
	}

	/// <summary>Calculates and returns the camera position based on the most up to date camera properties.</summary>
	/// <returns>The TPS camera position</returns>
	const glm::vec3& getCameraPosition() const
	{
		// Construct the matrix and return
		if (_updatePosition)
		{
			_updatePosition = false;
			_updateView = true;
			// this makes the camera aligned behind the target and offset it by 90 degree because our initial axis start from north.
			float r = _distance;
			float sinphi = glm::sin(glm::radians(_azimuth));
			float cosphi = glm::cos(glm::radians(_azimuth));
			float sintheta = glm::sin(glm::radians(_inclination));
			float costheta = glm::cos(glm::radians(_inclination));
			_cameraPos.x = r * costheta * cosphi;
			_cameraPos.y = r * sintheta;
			_cameraPos.z = r * costheta * sinphi;
		}
		return _cameraPos;
	}

private:
	float _azimuth, _inclination, _distance;
	glm::vec3 _targetPos; // Character
	mutable glm::vec3 _cameraPos;
	mutable glm::mat4 _viewX;
	mutable bool _updatePosition, _updateView;
};
#undef epsilon
} // namespace pvr
