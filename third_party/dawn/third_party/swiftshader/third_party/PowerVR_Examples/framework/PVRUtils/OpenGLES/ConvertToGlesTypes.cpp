/*!
\brief Implementation of the conversions from PVR Framework enums to OpenGL types. See ConvertToApiTypes.h.
\file PVRUtils/OpenGLES/ConvertToGlesTypes.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRUtils/OpenGLES/ConvertToGlesTypes.h"

namespace pvr {
namespace utils {
void getOpenGLFormat(PixelFormat pixelFormat, pvr::ColorSpace colorSpace, pvr::VariableType dataType, uint32_t& glInternalFormat, uint32_t& glFormat, uint32_t& glType,
	uint32_t& glTypeSize, bool& isCompressedFormat)
{
	isCompressedFormat = (pixelFormat.getPart().High == 0) && (pixelFormat.getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5));
	if (pixelFormat.getPart().High == 0)
	{
		// Format and type == 0 for compressed textures.
		glFormat = 0;
		glType = 0;
		glTypeSize = 1;
		switch (pixelFormat.getPixelTypeId())
		{
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
				glInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT
				glInternalFormat = GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
				glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
				glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT
				glInternalFormat = GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
				glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
				glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
				glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_ETC1_RGB8_OES
				glInternalFormat = GL_ETC1_RGB8_OES;
				return;
#endif
			}
			else
			{
#ifdef GL_ETC1_SRGB8_NV
				glInternalFormat = GL_ETC1_SRGB8_NV;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
				glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE
				glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
				glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE;
				return;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV
				glInternalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV;
				return;
#endif
			}
			break;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5):
		{
			// Not technically a compressed format by OpenGL ES standards.
			glType = GL_UNSIGNED_INT_5_9_9_9_REV;
			glTypeSize = 4;
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB9_E5;
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_ETC2; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
			};
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11):
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm || dataType == VariableType::SignedShort ||
				dataType == VariableType::SignedShortNorm || dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm || dataType == VariableType::SignedFloat)
			{ glInternalFormat = GL_COMPRESSED_SIGNED_R11_EAC; } else
			{
				glInternalFormat = GL_COMPRESSED_R11_EAC;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11):
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm || dataType == VariableType::SignedShort ||
				dataType == VariableType::SignedShortNorm || dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm || dataType == VariableType::SignedFloat)
			{ glInternalFormat = GL_COMPRESSED_SIGNED_RG11_EAC; } else
			{
				glInternalFormat = GL_COMPRESSED_RG11_EAC;
			}
			return;
		}

		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x10):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_10x10_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_10x5_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x6):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_10x6_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x8):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_10x8_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x10):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_12x10_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x12):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_12x12_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_3x3x3):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_3x3x3_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_3x3x3_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x3x3):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_4x3x3_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x3x3_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4x3):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4x3_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4x3_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4x4):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4x4_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4x4_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_5x4_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4x4):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_5x4x4_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_5x4x4_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_5x5_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5x4):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_5x4x4_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_5x4x4_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_5x5x5_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_5x5x5_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_6x5_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_6x5x5_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_6x5x5_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x6x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_6x6x5_OES
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_6x6x5_OES;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x5):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_8x5_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x6):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_8x6_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
#endif
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x8):
		{
			if (colorSpace == ColorSpace::lRGB)
			{
#ifdef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
				glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
#endif
			}
			else
			{
#ifdef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
				glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
#endif
			}
			return;
		}

		// Formats not supported by opengl/opengles
		case static_cast<uint64_t>(CompressedPixelFormat::BC4):
		case static_cast<uint64_t>(CompressedPixelFormat::BC5):
		case static_cast<uint64_t>(CompressedPixelFormat::BC6):
		case static_cast<uint64_t>(CompressedPixelFormat::BC7):
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888):
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888):
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY):
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2):
		case static_cast<uint64_t>(CompressedPixelFormat::BW1bpp):
		case static_cast<uint64_t>(CompressedPixelFormat::NumCompressedPFs):
			throw InvalidOperationError("[getOpenGLFormat]: Attempted to convert compressed format not supported on OpenGL ES");
		}
	}
	else
	{
		switch (dataType)
		{
		case VariableType::UnsignedFloat:
			if (pixelFormat.getPixelTypeId() == GeneratePixelType3<'b', 'g', 'r', 10, 11, 11>::ID || pixelFormat.getPixelTypeId() == GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID)
			{
				glTypeSize = 4;
				glType = GL_UNSIGNED_INT_10F_11F_11F_REV;
				glFormat = GL_RGB;
				glInternalFormat = GL_R11F_G11F_B10F;
				return;
			}
			break;
		case VariableType::SignedFloat:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			// HALF_FLOAT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16F;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16F;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG16F;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R16F;
				return;
			}
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA16F_EXT;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE16F_EXT;
				return;
			}
			case GeneratePixelType1<'a', 16>::ID:
			{
				glTypeSize = 2;
				glType = GL_HALF_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R16F;
				return;
			}
			// FLOAT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA32F;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB32F;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RG;
				glInternalFormat = GL_RG32F;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R32F;
				return;
			}
			case GeneratePixelType2<'l', 'a', 32, 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA32F_EXT;
				return;
			}
			case GeneratePixelType1<'l', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE32F_EXT;
				return;
			}
			case GeneratePixelType1<'a', 32>::ID:
			{
				glTypeSize = 4;
				glType = GL_FLOAT;
				glFormat = GL_RED;
				glInternalFormat = GL_R32F;
				return;
			}
			case GeneratePixelType1<'d', 16>::ID:
			{
				glType = GL_UNSIGNED_SHORT;
				glTypeSize = 2;
				glInternalFormat = GL_DEPTH_COMPONENT16;
				glFormat = GL_DEPTH_COMPONENT;
				return;
			}
			case GeneratePixelType1<'d', 24>::ID:
			{
				glType = GL_UNSIGNED_INT;
				glTypeSize = 3;
				glInternalFormat = GL_DEPTH_COMPONENT24;
				glFormat = GL_DEPTH_COMPONENT;
				return;
			}
			case GeneratePixelType2<'d', 's', 24, 8>::ID:
			{
				glType = GL_UNSIGNED_INT_24_8;
				glTypeSize = 4;
				glInternalFormat = GL_DEPTH24_STENCIL8;
				glFormat = GL_DEPTH_STENCIL;
				return;
			}
			case GeneratePixelType2<'d', 's', 32, 8>::ID:
			{
				glType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
				glTypeSize = 5;
				glInternalFormat = GL_DEPTH32F_STENCIL8;
				glFormat = GL_DEPTH_STENCIL;
				return;
			}
			case GeneratePixelType1<'d', 32>::ID:
			{
				glType = GL_FLOAT;
				glTypeSize = 4;
				glInternalFormat = GL_DEPTH_COMPONENT32F;
				glFormat = GL_DEPTH_COMPONENT;
				return;
			}
			case GeneratePixelType1<'s', 8>::ID:
			{
				glTypeSize = 4;
				glInternalFormat = GL_STENCIL_INDEX8;
				glFormat = GL_DEPTH_STENCIL;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_SRGB8_ALPHA8; }
				else
				{
					glInternalFormat = GL_RGBA8;
				}
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: { glFormat = glInternalFormat = GL_RGB;
#ifdef GL_SRGB8
				if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_SRGB8; }
				else
				{
					glInternalFormat = GL_RGB8;
				}
#endif
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8;
				return;
			}
			case GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 8>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA;
				return;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_BGRA_EXT;
				glInternalFormat = GL_BGRA_EXT;
				return;
			}
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA8_SNORM;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB8_SNORM;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG8_SNORM;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R8_SNORM;
				return;
			}
			case GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 8>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			glType = GL_UNSIGNED_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8UI;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8UI;
				return;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			glType = GL_BYTE;
			glTypeSize = 1;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA8I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB8I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG8I;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R8I;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				glType = GL_UNSIGNED_SHORT_4_4_4_4;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA4;
				return;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_5_5_1;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB5_A1;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				glType = GL_UNSIGNED_SHORT_5_6_5;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB565;
				return;
			}
#ifdef GL_RGBA16_EXT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16_EXT;
				return;
			}
#endif
#ifdef GL_RGBA16_EXT
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16_EXT;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RGB16_EXT;
				return;
			}
#endif
#ifdef GL_R16_EXT
			case GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R16_EXT;
				return;
			}
#endif
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 16>::ID:
			{
				glFormat = GL_ALPHA;
				glInternalFormat = GL_ALPHA16F_EXT;
				return;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			glTypeSize = 2;
			glType = GL_SHORT;
			switch (pixelFormat.getPixelTypeId())
			{
#ifdef GL_RGBA16_SNORM_EXT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGBA16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_RGB16_SNORM_EXT
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_RG16_SNORM_EXT
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG;
				glInternalFormat = GL_RG16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_R16_SNORM_EXT
			case GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED;
				glInternalFormat = GL_R16_SNORM_EXT;
				return;
			}
#endif
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glFormat = GL_LUMINANCE_ALPHA;
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glFormat = GL_LUMINANCE;
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			glType = GL_UNSIGNED_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16UI;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16UI;
				return;
			}
			case GeneratePixelType1<'d', 16>::ID:
			{
				glFormat = GL_DEPTH_COMPONENT;
				glInternalFormat = GL_DEPTH_COMPONENT16;
				return;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			glType = GL_SHORT;
			glTypeSize = 2;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA16I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB16I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG16I;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R16I;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			glTypeSize = 4;
			if (pixelFormat.getPixelTypeId() == GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA;
				glInternalFormat = GL_RGB10_A2;
				return;
			}
#ifdef GL_RGB10_EXT
			if (pixelFormat.getPixelTypeId() == GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGB;
				glInternalFormat = GL_RGB10_EXT;
				return;
			}
#endif
			break;
		}
		case VariableType::UnsignedInteger:
		{
			glType = GL_UNSIGNED_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32UI;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32UI;
				return;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				glType = GL_UNSIGNED_INT_2_10_10_10_REV;
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGB10_A2UI;
				return;
			}
			case GeneratePixelType1<'d', 24>::ID: { glFormat = GL_DEPTH_COMPONENT;
#if defined(GL_DEPTH_COMPONENT24)
				glInternalFormat = GL_DEPTH_COMPONENT24;
#else
				glInternalFormat = GL_DEPTH_COMPONENT24_OES;
#endif
				return;
			}
			case GeneratePixelType2<'d', 's', 24, 8>::ID: {
#if defined(GL_DEPTH_STENCIL)
				glFormat = GL_DEPTH_STENCIL;
				glInternalFormat = GL_DEPTH24_STENCIL8;
#else
				glFormat = GL_DEPTH_STENCIL_OES;
				glInternalFormat = GL_DEPTH24_STENCIL8_OES;
#endif
				return;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			glType = GL_INT;
			glTypeSize = 4;
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glFormat = GL_RGBA_INTEGER;
				glInternalFormat = GL_RGBA32I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glFormat = GL_RGB_INTEGER;
				glInternalFormat = GL_RGB32I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glFormat = GL_RG_INTEGER;
				glInternalFormat = GL_RG32I;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glFormat = GL_RED_INTEGER;
				glInternalFormat = GL_R32I;
				return;
			}
			}
			break;
		}
		default: {
		}
		}
	}
	// Default (erroneous) return values.
	glTypeSize = glType = glFormat = glInternalFormat = 0;
	throw InvalidOperationError("[getOpenGLFormat]: Attempted to convert format not supported on OpenGL ES");
}

void getOpenGLStorageFormat(PixelFormat pixelFormat, pvr::ColorSpace colorSpace, VariableType dataType, GLenum& glInternalFormat)
{
	if (pixelFormat.getPart().High == 0)
	{
		// Format and type == 0 for compressed textures.
		switch (pixelFormat.getPixelTypeId())
		{
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB): {
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
#endif
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA): {
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
#endif
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB): {
#ifdef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
			glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
#endif
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA): {
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
#endif
			return;
		}
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG;
			return;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
		{
			glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
			return;
		}
#endif
#ifdef GL_ETC1_RGB8_OES
		case static_cast<uint64_t>(CompressedPixelFormat::ETC1):
		{
			glInternalFormat = GL_ETC1_RGB8_OES;
			return;
		}
#endif
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		case static_cast<uint64_t>(CompressedPixelFormat::DXT1):
		{
			glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			return;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE
		case static_cast<uint64_t>(CompressedPixelFormat::DXT2):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT3):
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE;
			return;
		}
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
		case static_cast<uint64_t>(CompressedPixelFormat::DXT4):
		case static_cast<uint64_t>(CompressedPixelFormat::DXT5):
		{
			glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE;
			return;
		}
#endif
		case static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5):
		{
			// Not technically a compressed format by OpenGL ES standards.
			glInternalFormat = GL_RGB9_E5;
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_ETC2; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_ETC2;
			};
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1):
		{
			if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2; }
			else
			{
				glInternalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11):
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm || dataType == VariableType::SignedShort ||
				dataType == VariableType::SignedShortNorm || dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm || dataType == VariableType::SignedFloat)
			{ glInternalFormat = GL_COMPRESSED_SIGNED_R11_EAC; } else
			{
				glInternalFormat = GL_COMPRESSED_R11_EAC;
			}
			return;
		}
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11):
		{
			if (dataType == VariableType::SignedInteger || dataType == VariableType::SignedIntegerNorm || dataType == VariableType::SignedShort ||
				dataType == VariableType::SignedShortNorm || dataType == VariableType::SignedByte || dataType == VariableType::SignedByteNorm || dataType == VariableType::SignedFloat)
			{ glInternalFormat = GL_COMPRESSED_SIGNED_RG11_EAC; } else
			{
				glInternalFormat = GL_COMPRESSED_RG11_EAC;
			}
			return;
		}
		// Formats not supported by opengl/opengles
		case static_cast<uint64_t>(CompressedPixelFormat::BC4):
		case static_cast<uint64_t>(CompressedPixelFormat::BC5):
		case static_cast<uint64_t>(CompressedPixelFormat::BC6):
		case static_cast<uint64_t>(CompressedPixelFormat::BC7):
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888):
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888):
		case static_cast<uint64_t>(CompressedPixelFormat::UYVY):
		case static_cast<uint64_t>(CompressedPixelFormat::YUY2):
		case static_cast<uint64_t>(CompressedPixelFormat::BW1bpp):
		case static_cast<uint64_t>(CompressedPixelFormat::NumCompressedPFs):
			throw InvalidOperationError("[getOpenGLStorageFormat]: Attempted to convert compressed format not supported on OpenGL ES");
		}
	}
	else
	{
		switch (dataType)
		{
		case VariableType::UnsignedFloat:
			if (pixelFormat.getPixelTypeId() == GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID || pixelFormat.getPixelTypeId() == GeneratePixelType3<'b', 'g', 'r', 10, 11, 10>::ID)
			{
				glInternalFormat = GL_R11F_G11F_B10F;
				return;
			}
			break;
		case VariableType::SignedFloat:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			// HALF_FLOAT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16F;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16F;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16F;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16F;
				return;
			}
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 16>::ID:
			{
				glInternalFormat = GL_ALPHA16F_EXT;
				return;
			}
			// FLOAT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32F;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32F;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32F;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32F;
				return;
			}
			case GeneratePixelType2<'l', 'a', 32, 32>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 32>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 32>::ID:
			{
				glInternalFormat = GL_ALPHA32F_EXT;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedByteNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_SRGB8_ALPHA8; }
				else
				{
					glInternalFormat = GL_RGBA8;
				}
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				if (colorSpace == ColorSpace::sRGB) { glInternalFormat = GL_SRGB8; }
				else
				{
					glInternalFormat = GL_RGB8;
				}
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8;
				return;
			}
			case GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 8>::ID:
			{
				glInternalFormat = GL_ALPHA8_EXT;
				return;
			}
			case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_BGRA8_EXT;
				return;
			}
			}
			break;
		}
		case VariableType::SignedByteNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8_SNORM;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8_SNORM;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8_SNORM;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8_SNORM;
				return;
			}
			case GeneratePixelType2<'l', 'a', 8, 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 8>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedByte:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8UI;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8UI;
				return;
			}
			}
			break;
		}
		case VariableType::SignedByte:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGBA8I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID:
			{
				glInternalFormat = GL_RGB8I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 8, 8>::ID:
			{
				glInternalFormat = GL_RG8I;
				return;
			}
			case GeneratePixelType1<'r', 8>::ID:
			{
				glInternalFormat = GL_R8I;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedShortNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID:
			{
				glInternalFormat = GL_RGBA4;
				return;
			}
			case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID:
			{
				glInternalFormat = GL_RGB5_A1;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID:
			{
				glInternalFormat = GL_RGB565;
				return;
			}
#ifdef GL_RGBA16_EXT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16_EXT;
				return;
			}
#endif
#ifdef GL_RGB16_EXT
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16_EXT;
				return;
			}
#endif
#ifdef GL_RG16_EXT
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16_EXT;
				return;
			}
#endif
#ifdef GL_R16_EXT
			case GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16_EXT;
				return;
			}
#endif
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			case GeneratePixelType1<'a', 16>::ID:
			{
				glInternalFormat = GL_ALPHA;
				return;
			}
			}
			break;
		}
		case VariableType::SignedShortNorm:
		{
			switch (pixelFormat.getPixelTypeId())
			{
#ifdef GL_RGBA16_SNORM_EXT
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_RGB16_SNORM_EXT
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_RG16_SNORM_EXT
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16_SNORM_EXT;
				return;
			}
#endif
#ifdef GL_R16_SNORM_EXT
			case GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16_SNORM_EXT;
				return;
			}
#endif
			case GeneratePixelType2<'l', 'a', 16, 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE_ALPHA;
				return;
			}
			case GeneratePixelType1<'l', 16>::ID:
			{
				glInternalFormat = GL_LUMINANCE;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedShort:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16UI;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16UI;
				return;
			}
			}
			break;
		}
		case VariableType::SignedShort:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGBA16I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID:
			{
				glInternalFormat = GL_RGB16I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 16, 16>::ID:
			{
				glInternalFormat = GL_RG16I;
				return;
			}
			case GeneratePixelType1<'r', 16>::ID:
			{
				glInternalFormat = GL_R16I;
				return;
			}
			}
			break;
		}
		case VariableType::UnsignedIntegerNorm:
		{
			if (pixelFormat.getPixelTypeId() == GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glInternalFormat = GL_RGB10_A2;
				return;
			}
#ifdef GL_RGB10_EXT
			if (pixelFormat.getPixelTypeId() == GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID)
			{
				glInternalFormat = GL_RGB10_EXT;
				return;
			}
			break;
#endif
		}
		case VariableType::UnsignedInteger:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32UI;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32UI;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32UI;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32UI;
				return;
			}
			case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
			{
				glInternalFormat = GL_RGB10_A2UI;
				return;
			}
			}
			break;
		}
		case VariableType::SignedInteger:
		{
			switch (pixelFormat.getPixelTypeId())
			{
			case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGBA32I;
				return;
			}
			case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID:
			{
				glInternalFormat = GL_RGB32I;
				return;
			}
			case GeneratePixelType2<'r', 'g', 32, 32>::ID:
			{
				glInternalFormat = GL_RG32I;
				return;
			}
			case GeneratePixelType1<'r', 32>::ID:
			{
				glInternalFormat = GL_R32I;
				return;
			}
			}
			break;
		}
		default: {
		}
		}
	}

	// Default (erroneous) return values.
	glInternalFormat = 0;
	throw InvalidOperationError("[getOpenGLStorageFormat]: Attempted to convert format not supported on OpenGL ES");
}

GLenum convertToGles(Face face)
{
	static GLenum glFace[] = { GL_NONE, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK };
	return glFace[static_cast<uint32_t>(face)];
}

GLenum convertToGles(PolygonWindingOrder order)
{
	static GLenum glWindingOrder[] = { GL_CCW, GL_CW };
	return glWindingOrder[static_cast<uint32_t>(order)];
}

GLenum convertToGles(CompareOp func)
{
	static GLenum glCompareMode[] = { GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
	return glCompareMode[static_cast<uint32_t>(func)];
}

GLenum convertToGles(ImageAspectFlags type)
{
	switch (type)
	{
	case ImageAspectFlags::Color: return GL_COLOR_ATTACHMENT0;
	case ImageAspectFlags::Depth: return GL_DEPTH_ATTACHMENT;
	case ImageAspectFlags::Stencil: return GL_STENCIL_ATTACHMENT;
	case ImageAspectFlags::DepthAndStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
	default: assertion(0, "Invalid image aspect type"); return GL_COLOR_ATTACHMENT0;
	}
}

GLenum convertToGles(ImageViewType texType)
{
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D GL_TEXTURE_3D_OES
#endif

	static GLenum glTextureType[] = { GL_NONE, GL_NONE, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_NONE, GL_TEXTURE_2D_ARRAY, GL_NONE, GL_NONE,
#ifdef GL_TEXTURE_EXTERNAL_OES
		GL_TEXTURE_EXTERNAL_OES
#else
		GL_NONE
#endif
	};

	return glTextureType[static_cast<uint32_t>(texType)];
}

GLenum convertToGles(DataType dataType)
{
	static const GLenum map[] = { GL_NONE, GL_FLOAT, GL_INT, GL_UNSIGNED_SHORT, GL_RGBA, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_FIXED, GL_UNSIGNED_BYTE, GL_SHORT, GL_SHORT,
		GL_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_NONE, GL_HALF_FLOAT };
	return map[static_cast<uint32_t>(dataType)];
}

GLenum convertToGles(SamplerAddressMode addressMode)
{
	if (addressMode > SamplerAddressMode::ClampToEdge)
	{
		static const char* wrapNames[] = { "Border", "MirrorClamp" };
		Log("%s SamplerAddressMode '%s' not support falling back to default", wrapNames[static_cast<uint32_t>(addressMode) - static_cast<uint32_t>(SamplerAddressMode::ClampToBorder)]);
		addressMode = SamplerAddressMode::Default;
	}
	const static GLenum glSampler[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE };
	return glSampler[static_cast<uint32_t>(addressMode)];
}

GLenum convertToGles(StencilOp stencilOp)
{
	static GLenum glStencilOpValue[] = { GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT, GL_INCR_WRAP, GL_DECR_WRAP };
	return glStencilOpValue[static_cast<uint32_t>(stencilOp)];
}

GLenum convertToGles(BlendOp blendOp)
{
	static GLenum glOp[] = { GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX };
	return glOp[static_cast<uint32_t>(blendOp)];
}

GLenum convertToGles(BlendFactor blendFactor)
{
	static GLenum glBlendFact[] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE };
	return glBlendFact[static_cast<uint32_t>(blendFactor)];
}

GLenum convertToGles(PrimitiveTopology primitiveType)
{
#if BUILD_API_MAX < 31
	static GLenum glPrimtiveType[] = {
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
	};
	if (primitiveType > PrimitiveTopology::TriangleFan)
	{ Log(LogLevel::Error, "drawPrimitiveType: Primitive type not supported at this API level (BUILD_API_MAX is defined and BUILD_API_MAX<31)"); }
	#else

	static GLenum glPrimtiveType[] = { GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_LINES_ADJACENCY_OES, GL_LINE_STRIP_ADJACENCY_OES,
		GL_TRIANGLES_ADJACENCY_OES, GL_TRIANGLE_STRIP_ADJACENCY_OES, GL_PATCHES };
#endif

	return glPrimtiveType[static_cast<uint32_t>(primitiveType)];
}
} // namespace utils
} // namespace pvr
//!\endcond
