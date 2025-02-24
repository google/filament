/*!
\brief A class representing a First person camera and functionality to manipulate it.
\file PVRCore/cameras/FPSCamera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/glm.h"

namespace pvr {
/// <summary>A simple first person camera implementation</summary>
class FPSCamera
{
public:
	/// <summary>Constructor.</summary>
	FPSCamera() : _yaw(0.f), _pos(0.f), _isPosDirty(true), _isOrientationDirty(true), _moveZ(0.f), _moveX(0.f), _pitch(0.f)
	{
		_right = glm::vec3(1.f, 0.0f, 0.0f);
		_up = glm::vec3(0.f, 1.0f, 0.0f);
		_look = glm::vec3(0.f, 0.0f, -1.0f);
	}

	/// <summary>Move camera on its local z axis.</summary>
	/// <param name="z">distance to move</param>
	void moveZ(float z)
	{
		_moveZ += z;
		_isPosDirty = true;
	}

	/// <summary>Move camera on its local x axis.</summary>
	/// <param name="x">distance to move</param>
	void moveX(float x)
	{
		_moveX += x;
		_isPosDirty = true;
	}

	/// <summary>Set camera new world position and orientation.</summary>
	/// <param name="camPos">Camera position</param>
	/// <param name="yaw">Camera yaw</param>
	/// <param name="pitch">Camera pitch</param>
	void set(const glm::vec3& camPos, float yaw, float pitch)
	{
		setPosition(camPos);
		setOrientation(yaw, pitch);
	}

	/// <summary>Set camera new world position.</summary>
	/// <param name="camPos">Camera position</param>
	void setPosition(const glm::vec3& camPos)
	{
		_pos = camPos;
		_isPosDirty = true;
	}

	/// <summary>Set camera new orientation.</summary>
	/// <param name="yaw">Camera yaw</param>
	/// <param name="pitch">Camera pitch</param>
	void setOrientation(float yaw, float pitch)
	{
		_yaw = _pitch = 0.f;
		orientate(yaw, pitch);
	}

	/// <summary>Set camera new yaw.</summary>
	/// <param name="yaw">Camera yaw</param>
	void yaw(float yaw)
	{
		_yaw += yaw;
		if (_yaw <= -180.f) { _yaw += 360.f; }
		if (_yaw > 180.f) { _yaw -= 360.f; }
		_isOrientationDirty = true;
	}

	/// <summary>Reset camera position and orientation.</summary>
	/// <param name="pos">Camera position</param>
	/// <param name="yaw">Camera yaw</param>
	/// <param name="pitch">Camera pitch</param>
	void reset(const glm::vec3& pos, float yaw, float pitch)
	{
		_pos = pos;
		_isPosDirty = true;
		_pitch = 0.0f;
		_yaw = 0.0f;
		orientate(yaw, pitch);
	}

	/// <summary>Set camera orientation.</summary>
	/// <param name="pitch">Camera pitch</param>
	void pitch(float pitch)
	{
		_pitch += pitch;
		_pitch = glm::clamp(_pitch, -90.f, 90.f);
		_isOrientationDirty = true;
	}

	/// <summary>Set camera orientation.</summary>
	/// <param name="yaw">Camera yaw</param>
	/// <param name="pitch">Camera pitch</param>
	void orientate(float yaw, float pitch)
	{
		_isOrientationDirty = true;
		_pitch += pitch;
		_pitch = glm::clamp(_pitch, -90.f, 90.f);
		this->yaw(yaw);
	}

	/// <summary>Get camera position.</summary>
	/// <returns>Camera position</returns>
	const glm::vec3& getPosition()
	{
		updateRUL();
		updatePos();
		return _pos;
	}

	/// <summary>Get camera view matrix.</summary>
	/// <returns>Camera view matrix</returns>
	glm::mat4 getViewMatrix() const
	{
		updateRUL();
		updatePos();
		// Create a 4x4 view matrix from the right, up, forward and eye position vectors
		glm::mat4 viewMatrix = { glm::vec4(_right.x, _up.x, _look.x, 0), glm::vec4(_right.y, _up.y, _look.y, 0), glm::vec4(_right.z, _up.z, _look.z, 0),
			glm::vec4(-glm::dot(_right, _pos), -glm::dot(_up, _pos), -glm::dot(_look, _pos), 1) };
		return viewMatrix;
	}

private:
	void updateRUL() const
	{
		if (_isOrientationDirty)
		{
			// If the pitch and yaw angles are in degrees,
			// they need to be converted to radians. Here
			// I assume the values are already converted to radians.
			const float cosPitch = cos(glm::radians(_pitch));
			const float sinPitch = sin(glm::radians(_pitch));
			const float cosYaw = cos(glm::radians(_yaw));
			const float sinYaw = sin(glm::radians(_yaw));

			_right = { cosYaw, 0, -sinYaw };
			_up = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
			_look = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
			_isOrientationDirty = false;
		}
	}

	void updatePos() const
	{
		if (_isPosDirty)
		{
			_pos += _moveZ * _look;
			_isPosDirty = false;
			_moveZ = 0.0f;
		}
	}

	mutable glm::vec3 _pos;
	mutable float _moveZ, _moveX;
	float _yaw, _pitch;
	mutable glm::vec3 _right, _look, _up;
	mutable glm::mat4 _viewX;
	mutable bool _isPosDirty;
	mutable bool _isOrientationDirty;
};
} // namespace pvr
