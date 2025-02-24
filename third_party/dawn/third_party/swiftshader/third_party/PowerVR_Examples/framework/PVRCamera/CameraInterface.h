/*!
\brief Common interfaceof the PVRCamera camera streaming interface.
\file PVRCamera/CameraInterface.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "DynamicGles.h"
#include "PVRCore/math/MathUtils.h"

/// <summary>Main PowerVR Namespace</summary>
namespace pvr {
/// <summary>Enumeration of the possible hardware cameras present (front, back)</summary>
namespace HWCamera {
/// <summary>Enumeration of the possible hardware cameras present (front, back)</summary>
enum Enum
{
	Front, ///< Front camera
	Back, ///< Back camera
};
} // namespace HWCamera

class CameraInterfaceImpl;

/// <summary>A class design to provide you with a Texture handle to the Camera's image.</summary>
class CameraInterface
{
	friend class ::pvr::CameraInterfaceImpl;

public:
	/// <summary>Constructor</summary>
	CameraInterface();

	/// <summary>Destructor</summary>
	~CameraInterface();

	/// <summary>Initializes the capture session using the given hardware camera, if it is available.</summary>
	/// <param name="eCamera">The hardware camera to attempt to stream from</param>
	/// <param name="preferredResX,preferredResY">If supported by the implementation, set a preferred resolution</param>
	void initializeSession(HWCamera::Enum eCamera, int preferredResX = 0, int preferredResY = 0);

	/// <summary>Shutdown the AV capture session and release associated objects.</summary>
	void destroySession();

	/// <summary>Checks to see if the image has been updated.</summary>
	/// <returns>True if the image has been updated, otherwise false</returns>
	bool updateImage();

	/// <summary>Checks to see if the projection matrix has changed.</summary>
	/// <returns>True if the projection matrix changed, otherwise false</returns>
	bool hasProjectionMatrixChanged();

	/// <summary>Retrieves the current texture projection matrix and resets the 'changed' flag.</summary>
	/// <returns>pointer to 16 float values representing the matrix</returns>
	const glm::mat4& getProjectionMatrix();

	/// <summary>Retrieves the texture name for the YUV camera texture.</summary>
	/// <returns>A native API handle that can be used to get the texture.</returns>
	GLuint getRgbTexture();

	/// <summary>Query if this implementation supports a single RGB texture for the camera streaming interface.</summary>
	/// <returns>True if the implementation supports an RGB texture, false otherwise</returns>
	/// <remarks>This function will return true if the getRgbTexture() can be used. In implementations where this is
	/// not supported (e.g. iOS), this function will return false, and the getRgbTexture() function will return an
	/// empty (invalid) texture if used. See hasLumaChromaTextures(). In implementations where RGB textures are
	/// supported (e.g. Android) this function will return true and the getRgbTexture() will return a valid texture
	/// handle (if called after this interface was successfully initialized).</remarks>
	bool hasRgbTexture();

	/// <summary>Query if this implementation supports YUV (Luma/Chroma)planar textures.</summary>
	/// <returns>True if the implementation supports Luminance/Chrominance textures, false otherwise</returns>
	/// <remarks>This function will return true if the getLuminanceTexture() and and getChrominanceTexture() can be
	/// used. In implementations where this is not supported (e.g. Android), this function will return false, and the
	/// getLuminanceTexture/getChrominanceTexture will return empty (invalid) textures if used. In implementations
	/// where Luminance/Chrominance textures are supported (e.g. iOS) this function will return true and the
	/// getLuminanceTexture(), getChrominanceTexture() will return valid texture handles that can each be used to
	/// directly query the Luminance texture (Y channel of the format) and the Chrominance texture(UV channels of the
	/// format)</remarks>
	bool hasLumaChromaTextures();

	/// <summary>Retrieves the texture name for the YUV camera texture.</summary>
	/// <returns>GL texture ID</returns>
	GLuint getLuminanceTexture();

	/// <summary>Retrieves the texture name for the YUV camera texture.</summary>
	/// <returns>GL texture ID</returns>
	GLuint getChrominanceTexture();

	/// <summary>Returns the resolution of the currently active camera.</summary>
	/// <param name="width">The horizontal resolution of the currently active camera will be saved here</param>
	/// <param name="height">The vertical resolution of the currently active camera will be saved here</param>
	bool getCameraResolution(uint32_t& width, uint32_t& height);

	bool isReady() { return _isReady; }

private:
	bool _isReady = false;
	void* pImpl;
};
} // namespace pvr
