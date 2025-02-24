/*!
\brief Contains OpenGL ES specific Helper utilities.
\file PVRUtils/OpenGLES/TextureUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRUtils/OpenGLES/BindingsGles.h"
#include "PVRCore/texture/Texture.h"

namespace pvr {
namespace utils {
/// <summary>The TextureUploadResults structure provides the results for texture upload functions in a simple to use structure</summary>
struct TextureUploadResults
{
	/// <summary>The texture target to be used for the resulting texture</summary>
	GLenum target;

	/// <summary>A native texture handle where the texture was uploaded</summary>
	GLuint image;

	/// <summary>The format of the created texture</summary>
	PixelFormat format;

	/// <summary>Will be set to 'true' if the file was of an uncompressed format unsupported by the
	/// platform, and it was (software) decompressed to a supported uncompressed format</summary>
	bool isDecompressed;

	/// <summary>Default constructor for a TextureUploadResults structure.</summary>
	TextureUploadResults() : target(0), image(0), isDecompressed(0) {}

	/// <summary>Destructor for a TextureUploadResults structure.</summary>
	~TextureUploadResults() {}

	/// <summary>Move constructor for a TextureUploadResults structure.</summary>
	/// <param name="rhs">The TextureUploadResults structure to use as the source of the move.</param>
	TextureUploadResults(TextureUploadResults&& rhs) : target(rhs.target), image(rhs.image), format(rhs.format), isDecompressed(rhs.isDecompressed)
	{
		rhs.target = 0;
		rhs.image = 0;
	}

private:
	TextureUploadResults(const TextureUploadResults&);
};

/// <summary>Upload a texture to the GPU on the current context, and return it as part of the TextureUploadResults structure.</summary>
/// <param name="texture">The pvr::Texture to upload to the GPU</param>
/// <param name="isEs2">Signifies whether the current context being used for the texture upload is ES2 only. If the
/// context is ES2 only then the texture upload should not use ES3+ functionality as it will be unsupported via this context.</param>
/// <param name="allowDecompress">Set to true to allow to attempt to de-compress unsupported compressed textures.
/// The textures will be decompressed if ALL of the following are true: The texture is in a compressed format that
/// can be decompressed by the framework (PVRTC), the platform does NOT support this format (if it is hardware
/// supported, it will never be decompressed), and this flag is set to true. Default:true.</param>
/// <returns>A TextureUploadResults object containing the uploaded texture and all necessary information (size, formats,
/// whether it was actually decompressed. The "result" field will contain Result::Success
/// on success, errorcode otherwise. See the Texture</returns>
TextureUploadResults textureUpload(const Texture& texture, bool isEs2, bool allowDecompress);
} // namespace utils
} // namespace pvr
