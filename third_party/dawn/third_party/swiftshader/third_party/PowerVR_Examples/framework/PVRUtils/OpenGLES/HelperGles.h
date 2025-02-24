/*!
\brief Contains utility functions to facilitate tasks to create API objects form assets
\file PVRUtils/OpenGLES/HelperGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/IAssetProvider.h"
#include "PVRCore/texture/PVRTDecompress.h"
#include "PVRUtils/PVRUtilsTypes.h"
#include "PVRAssets/Model.h"
#include "PVRCore/texture/TextureLoad.h"
#include "PVRUtils/OpenGLES/TextureUtilsGles.h"
#include "PVRUtils/OpenGLES/ShaderUtilsGles.h"
#include "PVRUtils/OpenGLES/ConvertToGlesTypes.h"
#include "PVRUtils/OpenGLES/ErrorsGles.h"
#include "PVRCore/textureio/TGAWriter.h"
#include "PVRUtils/OpenGLES/PBRUtilsGles.h"
#include <iterator>

namespace pvr {

/// <summary>Contains functionality (especially free-standing functions) used to facilitate and simplify common tasks,
/// such as automated generation of VBOs for specific meshes, or tying together Effects and Meshes to automate
/// Pipeline creation.</summary>
namespace utils {

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
{
	typename container::iterator it = std::upper_bound(begin, end, item, compare);
	int64_t offset = static_cast<int64_t>(it - begin);
	cont.insert(it, item);
	return static_cast<size_t>(offset);
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted(cont, begin, end, item, std::less<val>());
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted(container& cont, const val& item)
{
	return insertSorted(cont, cont.begin(), cont.end(), item);
}

/// <summary>Insert sorted element in to the container</summary>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted(container& cont, const val& item, const cmp& compare)
{
	return insertSorted(cont, cont.begin(), cont.end(), item, compare);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item, const cmp& compare)
{
	typename container::iterator it = std::lower_bound(begin, end, item, compare);
	int64_t offset = static_cast<int64_t>(it - begin);
	if (it != end && !(compare(*it, item) || compare(item, *it))) { *it = item; }
	else
	{
		cont.insert(it, item);
	}
	return static_cast<size_t>(offset);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="begin">Container range begin</param>
/// <param name="end">Container range end</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, typename container::iterator begin, typename container::iterator end, const val& item)
{
	return insertSorted_overwrite(cont, begin, end, item, std::less<val>());
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
template<typename container, typename val>
size_t insertSorted_overwrite(container& cont, const val& item)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item);
}

/// <summary>Insert sorted element, Overwrite if element exist in the container</summary>
/// <param name="cont">Container to insert the element into.</param>
/// <param name="item">Item to insert in to the container</param>
/// <param name="compare">Comparison operator used for sorting</param>
template<typename container, typename val, typename cmp>
size_t insertSorted_overwrite(container& cont, const val& item, const cmp& compare)
{
	return insertSorted_overwrite(cont, cont.begin(), cont.end(), item, compare);
}

inline Api getCurrentGlesVersion()
{
	const char* apistring = (const char*)gl::GetString(GL_VERSION);
	int major, minor;
	int s = sscanf(apistring, "OpenGL ES %d.%d", &major, &minor);
	(void)s;

	if (major == 2) return Api::OpenGLES2;
	if (major == 3)
	{
		if (minor == 0) return Api::OpenGLES3;
		return Api::OpenGLES31;
	}
	throw "";
}

/// <summary>Check the currently bound GL_DRAW_FRAMEBUFFER status. On success, return true. On error, log the
/// actual error log and return false</summary>
/// <returns>True on GL_FRAMEBUFFER_COMPLETE, otherwise false. Additionally logs the error on false</returns>
inline bool checkFboStatus()
{
	// check status
	GLenum fboStatus = gl::CheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch (fboStatus)
	{
#ifdef GL_FRAMEBUFFER_UNDEFINED
	case GL_FRAMEBUFFER_UNDEFINED:
		Log(LogLevel::Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_UNDEFINED");
		assertion(0);
		break;
#endif
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		Log(LogLevel::Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		assertion(0);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		Log(LogLevel::Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		assertion(0);
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		Log(LogLevel::Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_UNSUPPORTED");
		assertion(0);
		break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		Log(LogLevel::Error, "Fbo_::checkFboStatus GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		assertion(0);
		break;
#endif
	case GL_FRAMEBUFFER_COMPLETE: return true;
	default:
		Log(LogLevel::Error, "Fbo_::checkFboStatus UNKNOWN ERROR");
		assertion(0);
		break;
	}
	return false;
}

/// <summary>Reads a block of pixel data from the frame buffer using the dimensions width and height as the dimensions of the
/// pixel rectangle saved. The function will save the pixel data as a TGA file with the name specified by screenshotFileName. The
/// function can be used to take screenshots of the current frame buffer or frame when called prior to presenting the backbuffer i.e.
/// swapping buffers.</summary>
/// <param name="screenshotFileName">The name used as the filename for the saved TGA screenshot.</param>
/// <param name="width">The width of the pixel rectangle retrieved.</param>
/// <param name="height">The width of the pixel rectangle retrieved.</param>
/// <param name="screenshotScale">A scaling factor to use for increasing the size of the saved screenshot.</param>
inline void takeScreenshot(const std::string& screenshotFileName, const uint32_t width, const uint32_t height, const uint32_t screenshotScale = 1)
{
	std::vector<unsigned char> pBuffer(width * height * 4);
	gl::ReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA, GL_UNSIGNED_BYTE, pBuffer.data());

	GLenum err = gl::GetError();

	if (err != GL_NO_ERROR) { Log(LogLevel::Information, "Screenshot was not taken successfully, filename %s.", screenshotFileName.c_str()); }
	else
	{
		uint32_t size = width * height * 4;

		// Switch the red and blue channels to convert to BGRA
		for (uint32_t i = 0; i < size; i += 4)
		{
			const unsigned char tmp = pBuffer[i];
			pBuffer[i] = pBuffer[i + 2];
			pBuffer[i + 2] = tmp;
		}

		Log(LogLevel::Information, "Writing TGA screenshot, filename %s.", screenshotFileName.c_str());
		writeTGA(screenshotFileName.c_str(), width, height, pBuffer.data(), 4, screenshotScale);
	}

	err = gl::GetError();

	if (err != GL_NO_ERROR) { Log(LogLevel::Information, "Screenshot was not taken successfully, filename %s.", screenshotFileName.c_str()); }
}

/// <summary>Reads a block of pixel data from the frame buffer using the dimensions width and height as the dimensions of the
/// pixel rectangle saved. The function will save the pixel data as a TGA file with the name specified by screenshotFileName. The
/// function can be used to take screenshots of the current frame buffer or frame when called prior to presenting the backbuffer i.e.
/// swapping buffers.</summary>
/// <param name="screenshotFileName">The name used as the filename for the saved TGA screenshot.</param>
/// <param name="width">The width of the pixel rectangle retrieved.</param>
/// <param name="height">The width of the pixel rectangle retrieved.</param>
/// <param name="screenshotScale">A scaling factor to use for increasing the size of the saved screenshot.</param>
/// <returns>A scaling factor to use for increasing the size of the saved screenshot.</returns>
inline GLuint textureUpload(const IAssetProvider& app, const char* file, pvr::Texture& outTexture, bool isEs2 = false)
{
	outTexture = pvr::textureLoad(*app.getAssetStream(file), pvr::getTextureFormatFromFilename(file));

	auto res = pvr::utils::textureUpload(outTexture, isEs2, true);

	return res.image;
}

inline GLuint textureUpload(const IAssetProvider& app, const std::string& file, pvr::Texture& outTexture, bool isEs2 = false)
{
	return textureUpload(app, file.c_str(), outTexture, isEs2);
}

inline GLuint textureUpload(const IAssetProvider& app, const char* file, bool isEs2 = false)
{
	pvr::Texture tex;
	return textureUpload(app, file, tex, isEs2);
}

inline GLuint textureUpload(const IAssetProvider& app, const std::string& file, bool isEs2 = false) { return textureUpload(app, file.c_str(), isEs2); }

inline TextureUploadResults textureUploadWithResults(const IAssetProvider& app, const char* file, pvr::Texture& outTexture, bool isEs2 = false)
{
	outTexture = pvr::textureLoad(*app.getAssetStream(file), pvr::getTextureFormatFromFilename(file));

	return pvr::utils::textureUpload(outTexture, isEs2, true);
}
inline TextureUploadResults textureUploadWithResults(const IAssetProvider& app, const char* file, bool isEs2 = false)
{
	return pvr::utils::textureUpload(pvr::textureLoad(*app.getAssetStream(file), pvr::getTextureFormatFromFilename(file)), isEs2, true);
}

inline pvr::Texture getTextureData(const IAssetProvider& app, const char* file)
{
	Texture outTexture = pvr::textureLoad(*app.getAssetStream(file), pvr::getTextureFormatFromFilename(file));

	// Is the texture compressed? RGB9E5 is treated as an uncompressed texture in OpenGL/ES so is a special case.
	bool isCompressedFormat =
		(outTexture.getPixelFormat().getPart().High == 0) && (outTexture.getPixelFormat().getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5));

	if (isCompressedFormat)
	{
		// Get the texture format for the API
		GLenum glInternalFormat = 0;
		GLenum glFormat = 0;
		GLenum glType = 0;
		GLenum glTypeSize = 0;
		bool unused;

		// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES,
		// it simply gets the values that would be set for a KTX file.
		utils::getOpenGLFormat(outTexture.getPixelFormat(), outTexture.getColorSpace(), outTexture.getChannelType(), glInternalFormat, glFormat, glType, glTypeSize, unused);

		// Handles software decompression of PVRTC textures
		Texture cDecompressedTexture;

		// Check for formats only supported by extensions.
		switch (glInternalFormat)
		{
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
#if defined(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG) || defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG) || defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG) || \
	defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)

			// Set up the new texture and header.
			TextureHeader cDecompressedHeader(outTexture);

			cDecompressedHeader.setPixelFormat(GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID);

			cDecompressedHeader.setChannelType(VariableType::UnsignedByteNorm);
			cDecompressedTexture = Texture(cDecompressedHeader);

			// Do decompression, one surface at a time.
			for (uint32_t uiMIPLevel = 0; uiMIPLevel < outTexture.getNumMipMapLevels(); ++uiMIPLevel)
			{
				for (uint32_t uiArray = 0; uiArray < outTexture.getNumArrayMembers(); ++uiArray)
				{
					for (uint32_t uiFace = 0; uiFace < outTexture.getNumFaces(); ++uiFace)
					{
						pvr::PVRTDecompressPVRTC(outTexture.getDataPointer(uiMIPLevel, uiArray, uiFace), (outTexture.getBitsPerPixel() == 2u ? 1u : 0u),
							outTexture.getWidth(uiMIPLevel), outTexture.getHeight(uiMIPLevel), cDecompressedTexture.getDataPointer(uiMIPLevel, uiArray, uiFace));
					}
				}
			}
			// Use the decompressed texture instead
			outTexture = cDecompressedTexture;
			break;
		}
#endif
	}
	return outTexture;
}

inline void generateTextureAtlas(
	const IAssetProvider& app, const StringHash* fileNames, Rectanglef* outUVs, uint32_t numTextures, GLuint* outTexture, TextureHeader* outDescriptor, bool isEs2 = false)
{
	struct SortedImage
	{
		uint32_t id;
		pvr::Texture texture;
		uint16_t width;
		uint16_t height;
		uint16_t srcX;
		uint16_t srcY;
		bool hasAlpha;
	};
	std::vector<SortedImage> sortedImage(numTextures);
	struct SortCompare
	{
		bool operator()(const SortedImage& a, const SortedImage& b)
		{
			uint32_t aSize = a.width * a.height;
			uint32_t bSize = b.width * b.height;
			return (aSize > bSize);
		}
	};

	struct Area
	{
		uint32_t x;
		uint32_t y;
		uint32_t w;
		uint32_t h;
		uint32_t size;
		bool isFilled;

		Area* right;
		Area* left;

	private:
		void setSize(uint32_t width, uint32_t height)
		{
			w = width;
			h = height;
			size = width * height;
		}

	public:
		Area(uint32_t width, uint32_t height) : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(width, height); }
		Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

		Area* insert(uint32_t width, uint32_t height)
		{
			// If this area has branches below it (i.e. is not a leaf) then traverse those.
			// Check the left branch first.
			if (left)
			{
				Area* tempPtr = NULL;
				tempPtr = left->insert(width, height);
				if (tempPtr != NULL) { return tempPtr; }
			}
			// Now check right
			if (right) { return right->insert(width, height); }
			// Already filled!
			if (isFilled) { return NULL; }

			// Too small
			if (size < width * height || w < width || h < height) { return NULL; }

			// Just right!
			if (size == width * height && w == width && h == height)
			{
				isFilled = true;
				return this;
			}
			// Too big. Split up.
			if (size > width * height && w >= width && h >= height)
			{
				// Initializes the children, and sets the left child's coordinates as these don't change.
				left = new Area;
				right = new Area;
				left->x = x;
				left->y = y;

				// --- Splits the current area depending on the size and position of the placed texture.
				// Splits vertically if larger free distance across the texture.
				if ((w - width) > (h - height))
				{
					left->w = width;
					left->h = h;

					right->x = x + width;
					right->y = y;
					right->w = w - width;
					right->h = h;
				}
				// Splits horizontally if larger or equal free distance downwards.
				else
				{
					left->w = w;
					left->h = height;

					right->x = x;
					right->y = y + height;
					right->w = w;
					right->h = h - height;
				}

				// Initializes the child members' size attributes.
				left->size = left->h * left->w;
				right->size = right->h * right->w;

				// Inserts the texture into the left child member.
				return left->insert(width, height);
			}
			// Catch all error return.
			return NULL;
		}

		bool deleteArea()
		{
			if (left != NULL)
			{
				if (left->left != NULL)
				{
					if (!left->deleteArea()) { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			if (right != NULL)
			{
				if (right->left != NULL)
				{
					if (!left->deleteArea()) { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			delete right;
			right = NULL;
			delete left;
			left = NULL;
			return true;
		}
	};

	// load the textures
	for (uint32_t i = 0; i < numTextures; ++i)
	{
		sortedImage[i].texture = pvr::utils::getTextureData(app, fileNames[i].c_str());

		sortedImage[i].id = i;
		sortedImage[i].width = static_cast<uint16_t>(sortedImage[i].texture.getWidth());
		sortedImage[i].height = static_cast<uint16_t>(sortedImage[i].texture.getHeight());
		const unsigned char* pixelString = sortedImage[i].texture.getPixelFormat().getPixelTypeChar();
		if (sortedImage[i].texture.getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA) ||
			sortedImage[i].texture.getPixelFormat().getPixelTypeId() == static_cast<uint64_t>(pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA) || pixelString[0] == 'a' ||
			pixelString[1] == 'a' || pixelString[2] == 'a' || pixelString[3] == 'a')
		{ sortedImage[i].hasAlpha = true; }
		else
		{
			sortedImage[i].hasAlpha = false;
		}
	}

	pvr::utils::throwOnGlError("generateTextureAtlas Begin");

	//// sort the sprites
	std::sort(sortedImage.begin(), sortedImage.end(), SortCompare());
	// find the best width and height
	uint32_t width = 0, height = 0, area = 0;
	uint32_t preferredDim[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
	const uint32_t atlasPixelBorder = 1;
	const uint32_t totalBorder = atlasPixelBorder * 2;
	uint32_t i = 0;
	// calculate the total area
	for (; i < sortedImage.size(); ++i) { area += (sortedImage[i].width + totalBorder) * (sortedImage[i].height + totalBorder); }
	i = 0;
	while ((preferredDim[i] * preferredDim[i]) < area && i < sizeof(preferredDim) / sizeof(preferredDim[0])) { ++i; }
	if (i >= sizeof(preferredDim) / sizeof(preferredDim[0])) { throw InvalidDataError("Cannot find a best size for texture atlas"); }
	width = height = preferredDim[i];
	float oneOverWidth = 1.f / width;
	float oneOverHeight = 1.f / height;

	Area* head = new Area(width, height);
	Area* pRtrn = NULL;
	Offset3D dstOffset[2];

	// create the out texture store
	ImageStorageFormat outFmt(PixelFormat::RGBA_32323232(), 1, ColorSpace::lRGB, VariableType::Float);
	gl::GenTextures(1, outTexture);
	gl::BindTexture(GL_TEXTURE_2D, *outTexture);

	gl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	bool useTexStorage = !isEs2;

	// Get the texture format for the API.
	GLenum glInternalFormat = 0;
	GLenum glFormat = 0;
	GLenum glType = 0;
	GLenum glTypeSize = 0;
	bool unused;

	// Check that the format is a valid format for this API - Doesn't check specifically between OpenGL/ES
	utils::getOpenGLFormat(sortedImage[0].texture.getPixelFormat(), sortedImage[0].texture.getColorSpace(), sortedImage[0].texture.getChannelType(), glInternalFormat, glFormat,
		glType, glTypeSize, unused);

	if (useTexStorage) { gl::TexStorage2D(GL_TEXTURE_2D, 1, glInternalFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height)); }
	else
	{
		if (isEs2) glInternalFormat = glFormat;
		gl::TexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, glFormat, glType, NULL);
	}

	pvr::utils::throwOnGlError("generateTextureAtlas Generate output texture");

	for (uint32_t textureIndex = 0; textureIndex < numTextures; ++textureIndex)
	{
		const SortedImage& image = sortedImage[textureIndex];
		pRtrn = head->insert(sortedImage[textureIndex].width + totalBorder, sortedImage[textureIndex].height + totalBorder);
		if (!pRtrn)
		{
			head->deleteArea();
			delete head;
			throw InvalidDataError("ERROR: Not enough room in texture atlas!");
		}
		dstOffset[0].x = static_cast<uint16_t>(pRtrn->x + atlasPixelBorder);
		dstOffset[0].y = static_cast<uint16_t>(pRtrn->y + atlasPixelBorder);
		dstOffset[0].z = 0;

		dstOffset[1].x = static_cast<uint16_t>(dstOffset[0].x + sortedImage[textureIndex].width);
		dstOffset[1].y = static_cast<uint16_t>(dstOffset[0].y + sortedImage[textureIndex].height);
		dstOffset[1].z = 1;

		outUVs[image.id].x = dstOffset[0].x * oneOverWidth;
		outUVs[image.id].y = dstOffset[0].y * oneOverHeight;
		outUVs[image.id].width = sortedImage[textureIndex].width * oneOverWidth;
		outUVs[image.id].height = sortedImage[textureIndex].height * oneOverHeight;

		gl::TexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(dstOffset[0].x), static_cast<GLint>(dstOffset[0].y), static_cast<GLsizei>(sortedImage[textureIndex].width),
			static_cast<GLsizei>(sortedImage[textureIndex].height), glFormat, glType, image.texture.getDataPointer());
	}
	if (outDescriptor)
	{
		outDescriptor->setWidth(width);
		outDescriptor->setHeight(height);
		outDescriptor->setChannelType(outFmt.dataType);
		outDescriptor->setColorSpace(outFmt.colorSpace);
		outDescriptor->setDepth(1);
		outDescriptor->setPixelFormat(outFmt.format);
	}
	if (isEs2) { gl::Finish(); }
	else
	{
		gl::FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0x00000000);
	}

	head->deleteArea();
	delete head;

	pvr::utils::throwOnGlError("generateTextureAtlas End");
}

inline void deleteTexturesAndZero(GLuint& texture)
{
	if (texture != 0)
	{
		gl::DeleteTextures(1, &texture);
		texture = 0;
	}
}
template<typename... Args>
inline void deleteTexturesAndZero(GLuint& first, Args&... rest)
{
	deleteTexturesAndZero(first);
	deleteTexturesAndZero(rest...);
}

/// <summary>Represents a shader Explicit binding, tying a Semantic name to an Attribute Index.</summary>
struct VertexBindings
{
	std::string semanticName; //< effect semantic
	uint16_t binding; //< binding id
};

/// <summary>Represents a shader Reflective binding, tying a Semantic name to an Attribute variable name.</summary>
struct VertexBindings_Name
{
	StringHash semantic; //< effect semantic
	StringHash variableName; //< shader attribute name
};

namespace {
struct VertexAttributeInfoCmp_BindingLess_IndexLess
{
	bool operator()(const VertexAttributeInfoWithBinding& lhs, const VertexAttributeInfoWithBinding& rhs) const
	{
		return lhs.binding < rhs.binding || (lhs.binding == rhs.binding && lhs.index < rhs.index);
	}
};
struct VertexBindingInfoCmp_BindingLess
{
	bool operator()(const VertexInputBindingInfo& lhs, const VertexInputBindingInfo& rhs) const { return lhs.bindingId < rhs.bindingId; }
};

struct VertexBindingInfoPred_BindingLess
{
	bool operator()(uint16_t lhs, const VertexInputBindingInfo& rhs) const { return lhs < rhs.bindingId; }
};
} // namespace

struct VertexConfiguration
{
	PrimitiveTopology topology;
	std::vector<VertexAttributeInfoWithBinding> attributes;
	std::vector<VertexInputBindingInfo> bindings;

	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.</summary>
	/// <param name="bufferBinding">The binding index to add the vertex attribute information.</param>
	/// <param name="attrib">Vertex Attribute information object.</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexConfiguration& addVertexAttribute(uint16_t bufferBinding, const VertexAttributeInfo& attrib)
	{
		pvr::utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(attrib, bufferBinding), VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/// <summary>Add vertex layout information to a buffer binding index using an array of VertexAttributeInfo object.</summary>
	/// <param name="bufferBinding">The binding index to add the vertex attribute information.</param>
	/// <param name="attrib">Attribute information object.</param>
	/// <param name="numAttributes">Number of attributues in the array</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexConfiguration& addVertexAttributes(uint16_t bufferBinding, const VertexAttributeInfo* attrib, uint32_t numAttributes)
	{
		for (uint32_t i = 0; i < numAttributes; ++i)
		{ pvr::utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(attrib[i], bufferBinding), VertexAttributeInfoCmp_BindingLess_IndexLess()); }
		return *this;
	}

	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeLayout object and an
	/// attrib name.</summary>
	/// <param name="index">The index of the vertex attribute</param>
	/// <param name="bufferBinding">The binding index of the buffer from which vertex data will be read.</param>
	/// <param name="layout">Vertex Attribute Layout object</param>
	/// <param name="attributeName">The name of the variable in shader code. Required for API's that only support
	/// Reflective attribute binding and not Explicit binding of attributes to indices in shader code.</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexConfiguration& addVertexAttribute(uint16_t index, uint16_t bufferBinding, const VertexAttributeLayout& layout, const char* attributeName = "")
	{
		pvr::utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(index, layout.dataType, layout.width, layout.offset, bufferBinding, attributeName),
			VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/// <summary>Set the vertex input buffer bindings.</summary>
	/// <param name="bufferBinding">Vertex buffer binding index</param>
	/// <param name="strideInBytes">specifies the char offset between consecutive generic vertex attributes. If stride is 0,
	/// the generic vertex attributes are understood to be tightly packed in the array. The initial value is 0.</param>
	/// <param name="stepRate">The rate at which this binding is incremented (used for Instancing).</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexConfiguration& setInputBinding(uint16_t bufferBinding, uint16_t strideInBytes = 0, StepRate stepRate = StepRate::Vertex)
	{
		pvr::utils::insertSorted_overwrite(bindings, VertexInputBindingInfo(bufferBinding, strideInBytes, stepRate), VertexBindingInfoCmp_BindingLess());
		return *this;
	}
};

/// <summary>A container struct carrying Vertex Attribute information (vertex layout, plus binding point)
struct VertexAttributeInfoGles
{
	/// <summary>The Vertex Buffer binding point this attribute is bound to
	GLuint index; //!< Attribute index
	GLuint vboIndex;
	GLuint stride;

	GLenum format; //!< Data type of each element of the attribute
	GLint size; //!< Number of elements in attribute, e.g 1,2,3,4
	void* offset; //!< Offset of the first element in the buffer
	VertexAttributeInfoGles() : index(0), vboIndex(0), stride(0), format(0), size(0), offset(0) {}
	VertexAttributeInfoGles(const VertexAttributeInfoWithBinding& attr, const VertexInputBindingInfo& bind)
		: index(attr.index), vboIndex(attr.binding), stride(bind.strideInBytes), format(utils::convertToGles(attr.format)), size(attr.width),
		  offset(reinterpret_cast<void*>(static_cast<size_t>(attr.offsetInBytes)))
	{}

	void callVertexAttribPtr() { gl::VertexAttribPointer(index, size, format, false, static_cast<GLsizei>(stride), offset); }
};

/// <summary>A container struct carrying Vertex Attribute information (vertex layout, plus binding point)
struct VertexBindingInfoGles
{
	GLuint bindingId; //< buffer binding index
	GLuint stride; //< buffer stride in bytes
	GLenum stepRate;

	VertexBindingInfoGles() : bindingId(0), stride(0), stepRate(0) {}
};

inline VertexConfiguration createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings* bindingMap, uint16_t numBindings, uint16_t* outNumBuffers = NULL)
{
	VertexConfiguration retval;
	if (outNumBuffers) { *outNumBuffers = 0; }
	int16_t current = 0;
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semanticName.c_str());
		if (attr)
		{
			VertexAttributeLayout layout = attr->getVertexLayout();
			uint32_t stride = mesh.getStride(attr->getDataIndex());
			if (outNumBuffers) { *outNumBuffers = (uint16_t)std::max<uint32_t>(attr->getDataIndex() + 1u, *outNumBuffers); }
			retval.addVertexAttribute(bindingMap[current].binding, static_cast<uint16_t>(attr->getDataIndex()), layout)
				.setInputBinding(static_cast<uint16_t>(attr->getDataIndex()), static_cast<uint16_t>(stride), StepRate::Vertex);
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.", bindingMap[current].semanticName.c_str());
		}
		++current;
	}
	retval.topology = mesh.getMeshInfo().primitiveType;
	return retval;
}

inline VertexConfiguration createInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings_Name* bindingMap, uint16_t numBindings, uint16_t* outNumBuffers = NULL)
{
	VertexConfiguration retval;
	if (outNumBuffers) { *outNumBuffers = 0; }
	uint16_t current = 0;
	// In this scenario, we will be using our own indices instead of user provided ones, correlating them by names.
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semantic);
		if (attr)
		{
			VertexAttributeLayout layout = attr->getVertexLayout();
			uint32_t stride = mesh.getStride(attr->getDataIndex());

			if (outNumBuffers) { *outNumBuffers = (uint16_t)std::max<uint32_t>(attr->getDataIndex() + 1u, *outNumBuffers); }
			retval.addVertexAttribute(current, (uint16_t)attr->getDataIndex(), layout, bindingMap[current].variableName.c_str())
				.setInputBinding((uint16_t)attr->getDataIndex(), (uint16_t)stride, StepRate::Vertex);
			retval.topology = mesh.getMeshInfo().primitiveType;
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.", bindingMap[current].semantic.c_str());
		}
		++current;
	}
	return retval;
}

/// <summary>Auto generates a single VBO and a single IBO from all the vertex data of a mesh.
/// RESETS GL STATE: GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbo">The VBO handle where the data will be put. No buffer needs to have been created on the
/// handle. IS ASSUMED TO NOT BE A VALID OPENGL OBJECT.</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be set to zero.
/// IS ASSUMED TO NOT BE A VALID OPENGL OBJECT.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data. If data are not interleaved, they will be packed on
/// the same VBO, each interleaved block (Data element on the mesh) will be appended at the end of the buffer, and
/// the offsets will need to be calculated by the user when binding the buffer.</remarks>
inline void createSingleBuffersFromMesh(const assets::Mesh& mesh, GLuint& outVbo, GLuint& outIbo)
{
	size_t total = 0;
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i) { total += mesh.getDataSize(i); }

	gl::GenBuffers(1, &outVbo);
	gl::BindBuffer(GL_ARRAY_BUFFER, outVbo);
	gl::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(total), NULL, GL_STATIC_DRAW);

	size_t current = 0;
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
	{
		gl::BufferSubData(GL_ARRAY_BUFFER, static_cast<uint32_t>(current), static_cast<uint32_t>(mesh.getDataSize(i)), static_cast<const void*>(mesh.getData(i)));
		current += mesh.getDataSize(i);
	}
	gl::BindBuffer(GL_ARRAY_BUFFER, 0);
	if (mesh.getNumFaces())
	{
		gl::GenBuffers(1, &outIbo);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, outIbo);
		gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(mesh.getFaces().getDataSize()), static_cast<const void*>(mesh.getFaces().getData()), GL_STATIC_DRAW);
	}
	else
	{
		outIbo = 0;
	}
}

/// <summary>Auto generates a set of VBOs and a single IBO from all the vertex data of a mesh, respecting the Mesh's vertex layout
/// configuration: Each Data Element of the mesh will produce another VBO.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbos">Reference to a std::vector of VBO handles where the data will be put. Buffers will be appended
/// at the end.</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be null.</param>
/// <remarks>This utility function will read all vertex data from the mesh and create one Buffer for each data
/// element (block of interleaved data) in the mesh. It is thus commonly used for for meshes containing multiple
/// sets of interleaved data (for example, a VBO with static and a VBO with streaming data).</remarks>
inline void createMultipleBuffersFromMesh(const assets::Mesh& mesh, std::vector<GLuint>& outVbos, GLuint& outIbo)
{
	outVbos.resize(mesh.getNumDataElements());
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
	{
		gl::GenBuffers(1, &outVbos[i]);
		gl::BindBuffer(GL_ARRAY_BUFFER, outVbos[i]);
		gl::BufferData(GL_ARRAY_BUFFER, static_cast<uint32_t>(mesh.getDataSize(i)), static_cast<const void*>(mesh.getData(i)), GL_STATIC_DRAW);
	}
	if (mesh.getNumFaces())
	{
		gl::GenBuffers(1, &outIbo);
		gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, outIbo);
		gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(mesh.getFaces().getDataSize()), static_cast<const void*>(mesh.getFaces().getData()), GL_STATIC_DRAW);
	}
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and uses
/// std::inserter provided by the user to insert them to any container.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">std::inserter for a collection of api::Buffer handles. It will be used to insert one VBO per mesh.</param>
/// <param name="outIbos">std::inserter for a collection of api::Buffer handles. It will be used to insert one IBO per mesh.
/// If face data is not present on the mesh, a null handle will be inserted.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets. The std::inserter this function requires can be created from any container with an insert()
/// function with (for example, for insertion at the end of a vector) std::inserter(std::vector,
/// std::vector::end()) .</remarks>
template<typename MeshIterator_, typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromMeshes(MeshIterator_ meshIter, MeshIterator_ meshIterEnd, VboInsertIterator_ outVbos, IboInsertIterator_ outIbos)
{
	while (meshIter != meshIterEnd)
	{
		size_t total = 0;
		for (uint32_t ii = 0; ii < meshIter->getNumDataElements(); ++ii) { total += meshIter->getDataSize(ii); }

		GLuint vbo;
		gl::GenBuffers(1, &vbo);
		gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
		gl::BufferData(GL_ARRAY_BUFFER, total, NULL, GL_STATIC_DRAW);

		size_t current = 0;
		for (size_t ii = 0; ii < meshIter->getNumDataElements(); ++ii)
		{
			gl::BufferSubData(GL_ARRAY_BUFFER, static_cast<uint32_t>(current), static_cast<uint32_t>(meshIter->getDataSize(static_cast<uint32_t>(ii))),
				(const void*)meshIter->getData(static_cast<uint32_t>(ii)));
			current += meshIter->getDataSize(static_cast<uint32_t>(ii));
		}

		*outVbos = vbo;
		if (meshIter->getNumFaces())
		{
			GLuint ibo;
			gl::GenBuffers(1, &ibo);
			gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			gl::BufferData(
				GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(meshIter->getFaces().getDataSize()), reinterpret_cast<const void*>(meshIter->getFaces().getData()), GL_STATIC_DRAW);
			*outIbos = ibo;
		}
		else
		{
			*outIbos = 0;
		}
		++outVbos;
		++outIbos;
		++meshIter;
	}
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and insert them
/// at the specified spot in a user-provided container.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">Collection of api::Buffer handles. It will be used to insert one VBO per mesh.</param>
/// <param name="outIbos">Collection of api::Buffer handles. It will be used to insert one IBO per mesh. If face data is
/// not present on the mesh, a null handle will be inserted.</param>
/// <param name="vbos_where">Iterator on outVbos - the position where the insertion will happen.</param>
/// <param name="ibos_where">Iterator on outIbos - the position where the insertion will happen.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets.</remarks>
template<typename MeshIterator_, typename VboContainer_, typename IboContainer_>
inline void createSingleBuffersFromMeshes(MeshIterator_ meshIter, MeshIterator_ meshIterEnd, VboContainer_& outVbos, typename VboContainer_::iterator vbos_where,
	IboContainer_& outIbos, typename IboContainer_::iterator ibos_where)
{
	createSingleBuffersFromMeshes(meshIter, meshIterEnd, std::inserter(outVbos, vbos_where), std::inserter(outIbos, ibos_where));
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// inserts them into containers provided by the user using std::inserters.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">An insert iterator to a std::Buffer container for the VBOs. Vbos will be inserted using
/// this iterator.</param>
/// <param name="ibos">An insert iterator to an std::Buffer container for the IBOs. Ibos will be inserted using
/// this iterator.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other. The std::inserter this function requires can be created
/// from any container with an insert() function with (for example, for insertion at the end of a vector)
/// std::inserter(std::vector, std::vector::end()) .</remarks>
template<typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromModel(const assets::Model& model, VboInsertIterator_ vbos, IboInsertIterator_ ibos)
{
	createSingleBuffersFromMeshes(model.beginMeshes(), model.endMeshes(), vbos, ibos);
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// appends them at the end of containers provided by the user.</summary>
/// <param name="context">The device context where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">A container of api::Buffer handles. The VBOs will be inserted at the end of this
/// container.</param>
/// <param name="ibos">A container of api::Buffer handles. The IBOs will be inserted at the end of this
/// container.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other.</remarks>
template<typename VboContainer_, typename IboContainer_>
inline void appendSingleBuffersFromModel(const assets::Model& model, VboContainer_& vbos, IboContainer_& ibos)
{
	createSingleBuffersFromMeshes(model.beginMeshes(), model.endMeshes(), std::inserter(vbos, vbos.end()), std::inserter(ibos, ibos.end()));
}

inline void create3dPlaneMesh(uint32_t width, uint32_t length, bool vertexAttribTex, bool vertexAttribNormal, assets::Mesh& outMesh)
{
	const float halfWidth = width * .5f;
	const float halfLength = length * .5f;

	glm::vec3 normal[4] = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

	glm::vec2 texCoord[4] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
	};

	glm::vec3 pos[4] = { glm::vec3(-halfWidth, 0.0f, -halfLength), glm::vec3(-halfWidth, 0.0f, halfLength), glm::vec3(halfWidth, 0.0f, halfLength),
		glm::vec3(halfWidth, 0.0f, -halfLength) };

	uint32_t indexData[] = { 0, 1, 2, 0, 2, 3 };

	float vertData[32];
	uint32_t offset = 0;

	for (uint32_t i = 0; i < 4; ++i)
	{
		memcpy(&vertData[offset], &pos[i], sizeof(pos[i]));
		offset += 3;
		if (vertexAttribNormal)
		{
			memcpy(&vertData[offset], &normal[i], sizeof(normal[i]));
			offset += 3;
		}
		if (vertexAttribTex)
		{
			memcpy(&vertData[offset], &texCoord[i], sizeof(texCoord[i]));
			offset += 2;
		}
	}

	uint32_t stride = sizeof(glm::vec3) + (vertexAttribNormal ? sizeof(glm::vec3) : 0) + (vertexAttribTex ? sizeof(glm::vec2) : 0);

	outMesh.addData((const uint8_t*)vertData, sizeof(vertData), stride, 0);
	outMesh.addFaces((const uint8_t*)indexData, sizeof(indexData), IndexType::IndexType32Bit);
	offset = 0;
	outMesh.addVertexAttribute("POSITION", DataType::Float32, 3, offset, 0);
	offset += sizeof(float) * 3;
	if (vertexAttribNormal)
	{
		outMesh.addVertexAttribute("NORMAL", DataType::Float32, 3, offset, 0);
		offset += sizeof(float) * 2;
	}
	if (vertexAttribTex) { outMesh.addVertexAttribute("UV0", DataType::Float32, 2, offset, 0); }
	outMesh.setPrimitiveType(PrimitiveTopology::TriangleList);
	outMesh.setStride(0, stride);
	outMesh.setNumFaces(ARRAY_SIZE(indexData) / 3);
	outMesh.setNumVertices(ARRAY_SIZE(pos));
}
} // namespace utils
} // namespace pvr
