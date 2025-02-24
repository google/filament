/*!
\brief Android implementation of the camera streaming interface.
\file PVRCamera/CameraInterface_Dummy.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCamera/CameraInterface.h"
#include "PVRCore/Errors.h"
//!\cond NO_DOXYGEN

namespace pvr {
class CameraInterfaceImpl
{
public:
	CameraInterface* myParent;
	GLuint myTexture;
	uint32_t height, width;
	CameraInterfaceImpl(CameraInterface* parent) : myParent(parent), myTexture(0), height(512), width(512) {}
	void generateTexture()
	{
		gl::GetError(); // Make sure you don't break due to previous errors
		if (!myTexture) { gl::GenTextures(1, &myTexture); }

		gl::BindTexture(GL_TEXTURE_2D, myTexture);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		std::vector<uint32_t> rawBuffer;
		rawBuffer.resize(height * width);
		bool one = false, two = false;
		uint32_t hfheight = height / 2, hfwidth = width / 2;
		for (uint32_t j = 0; j < height; ++j)
			for (uint32_t i = 0; i < width; ++i)
			{
				if ((i < hfwidth + 64) && (i > hfwidth - 65) && (j < hfheight + 72) && (j > hfheight - 57))
				{
					one = (((i + hfwidth) / 8) % 2) != 0;
					two = (((j + hfheight) / 8) % 2) != 0;
				}
				else if ((i < hfwidth + 128) && (i > hfwidth - 129) && (j < hfheight + 136) && (j > hfheight - 121))
				{
					one = (((i + hfwidth) / 16) % 2) != 0;
					two = (((j + hfheight) / 16) % 2) != 0;
				}
				else if ((i < hfwidth + 256) && (i > hfwidth - 257) && (j < hfheight + 264) && (j > hfheight - 249))
				{
					one = (((i + hfwidth) / 32) % 2) != 0;
					two = (((j + hfheight) / 32) % 2) != 0;
				}
				else
				{
					one = false;
					two = false;
				}

				rawBuffer[j * width + i] = (one ^ two ? 0xFFC0C0C0 : 0xFF606060);
			}
		gl::TexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, rawBuffer.data());
		GLenum err = gl::GetError();
		if (err != GL_NO_ERROR) { throw pvr::PvrError("PVRCamera, Dummy version - Error while generating the dummy camera texture."); }
		myParent->_isReady = true;
	}
	void destroyTexture()
	{
		if (myTexture) gl::DeleteTextures(1, &myTexture);
		myTexture = 0;
	}
	const GLuint& getRgbTexture() { return myTexture; }
};

CameraInterface::CameraInterface() { pImpl = new CameraInterfaceImpl(this); }
CameraInterface::~CameraInterface() { delete static_cast<CameraInterfaceImpl*>(pImpl); }

void CameraInterface::initializeSession(HWCamera::Enum, int preferredResX, int preferredResY)
{
	static_cast<CameraInterfaceImpl*>(pImpl)->width = preferredResX ? static_cast<uint32_t>(preferredResX) : 512u;
	static_cast<CameraInterfaceImpl*>(pImpl)->height = preferredResY ? static_cast<uint32_t>(preferredResY) : 512u;
	static_cast<CameraInterfaceImpl*>(pImpl)->generateTexture();
}

bool CameraInterface::updateImage() { return false; }

bool CameraInterface::hasProjectionMatrixChanged() { return false; }

const glm::mat4& CameraInterface::getProjectionMatrix()
{
	static const glm::mat4 proj(1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.);
	return proj;
}

GLuint CameraInterface::getRgbTexture() { return static_cast<CameraInterfaceImpl*>(pImpl)->getRgbTexture(); }

GLuint CameraInterface::getLuminanceTexture() { return 0; }

GLuint CameraInterface::getChrominanceTexture() { return 0; }

void CameraInterface::destroySession() { static_cast<CameraInterfaceImpl*>(pImpl)->destroyTexture(); }

bool CameraInterface::getCameraResolution(uint32_t& x, uint32_t& y)
{
	x = static_cast<CameraInterfaceImpl*>(pImpl)->width;
	y = static_cast<CameraInterfaceImpl*>(pImpl)->height;
	return true;
}

bool CameraInterface::hasRgbTexture() { return true; }

bool CameraInterface::hasLumaChromaTextures() { return false; }
} // namespace pvr
//!\endcond
