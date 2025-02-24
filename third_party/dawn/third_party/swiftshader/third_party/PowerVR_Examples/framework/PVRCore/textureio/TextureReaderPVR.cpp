/*!
\brief Implementation of methods of the TextureReaderPVR class.
\file PVRCore/textureio/TextureReaderPVR.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/textureio/TextureReaderPVR.h"
#include "PVRCore/Log.h"
using std::vector;
namespace pvr {
namespace assetReaders {

namespace {
/// <summary>Load the this texture meta data from a stream</summary>
/// <param name="stream">Stream to load the meta data from</param>
inline TextureMetaData loadTextureMetadataFromStream(const Stream& stream)
{
	size_t dataRead = 0;

	uint32_t _fourCC;
	uint32_t _key;
	uint32_t _dataSize;

	stream.read(sizeof(_fourCC), 1, &_fourCC, dataRead);
	stream.read(sizeof(_key), 1, &_key, dataRead);
	stream.read(sizeof(_dataSize), 1, &_dataSize, dataRead);
	if (_dataSize <= 0) { throw FileIOError("TextureMetadata::loadFromStream: Could not read header from stream."); }
	TextureMetaData ret(_fourCC, _key, _dataSize, NULL);

	stream.read(1, _dataSize, ret.getData(), dataRead);
	return ret;
}

void mapLegacyEnumToNewFormat(const texture_legacy::PixelFormat legacyPixelType, PixelFormat& pixelType, ColorSpace& colorSpace, VariableType& channelType, bool& isPremultiplied)
{
	// Default value.
	isPremultiplied = false;

	switch (legacyPixelType)
	{
	case texture_legacy::MGL_ARGB_4444:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_ARGB_1555:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_RGB_565:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_RGB_555:
	{
		pixelType = GeneratePixelType4<'x', 'r', 'g', 'b', 1, 5, 5, 5>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_RGB_888:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::MGL_ARGB_8888:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::MGL_ARGB_8332:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 3, 3, 2>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_I_8:
	{
		pixelType = GeneratePixelType1<'i', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::MGL_AI_88:
	{
		pixelType = GeneratePixelType2<'a', 'i', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::MGL_1_BPP:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::BW1bpp);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::MGL_VY1UY0:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::YUY2);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::MGL_Y1VY0U:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::UYVY);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::MGL_PVRTC2:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::MGL_PVRTC4:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_RGBA_4444:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::GL_RGBA_5551:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::GL_RGBA_8888:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_RGB_565:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::GL_RGB_555:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'x', 5, 5, 5, 1>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::GL_RGB_888:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_I_8:
	{
		pixelType = GeneratePixelType1<'l', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_AI_88:
	{
		pixelType = GeneratePixelType2<'l', 'a', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_PVRTC2:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_PVRTC4:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_BGRA_8888:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_A_8:
	{
		pixelType = GeneratePixelType1<'a', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_PVRTCII4:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::GL_PVRTCII2:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_DXT1:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT1);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_DXT2:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT2);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::D3D_DXT3:
	{
		pixelType = CompressedPixelFormat::DXT3;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_DXT4:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT4);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::D3D_DXT5:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT5);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_RGB_332:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 3, 3, 2>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_AL_44:
	{
		pixelType = GeneratePixelType2<'a', 'l', 4, 4>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_LVU_655:
	{
		pixelType = GeneratePixelType3<'l', 'g', 'r', 6, 5, 5>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_XLVU_8888:
	{
		pixelType = GeneratePixelType4<'x', 'l', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_QWVU_8888:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_ABGR_2101010:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_ARGB_2101010:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_AWVU_2101010:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 2, 10, 10, 10>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_GR_1616:
	{
		pixelType = GeneratePixelType2<'g', 'r', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_VU_1616:
	{
		pixelType = GeneratePixelType2<'g', 'r', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_ABGR_16161616:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_R16F:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::D3D_GR_1616F:
	{
		pixelType = GeneratePixelType2<'g', 'r', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::D3D_ABGR_16161616F:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::D3D_R32F:
	{
		pixelType = GeneratePixelType1<'r', 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::D3D_GR_3232F:
	{
		pixelType = GeneratePixelType2<'g', 'r', 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::D3D_ABGR_32323232F:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 32, 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::e_etc_RGB_4BPP:
	{
		pixelType = CompressedPixelFormat::ETC1;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_A8:
	{
		pixelType = GeneratePixelType1<'a', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_V8U8:
	{
		pixelType = GeneratePixelType2<'g', 'r', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_L16:
	{
		pixelType = GeneratePixelType1<'l', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_L8:
	{
		pixelType = GeneratePixelType1<'l', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_AL_88:
	{
		pixelType = GeneratePixelType2<'a', 'l', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::D3D_UYVY:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::UYVY);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::D3D_YUY2:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::YUY2);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R32G32B32A32_FLOAT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R32G32B32A32_UINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedInteger;
		break;
	}

	case texture_legacy::DXGI_R32G32B32A32_SINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedInteger;
		break;
	}

	case texture_legacy::DXGI_R32G32B32_FLOAT:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R32G32B32_UINT:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedInteger;
		break;
	}

	case texture_legacy::DXGI_R32G32B32_SINT:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedInteger;
		break;
	}

	case texture_legacy::DXGI_R16G16B16A16_FLOAT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R16G16B16A16_UNORM:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16G16B16A16_UINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShort;
		break;
	}

	case texture_legacy::DXGI_R16G16B16A16_SNORM:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16G16B16A16_SINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShort;
		break;
	}

	case texture_legacy::DXGI_R32G32_FLOAT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R32G32_UINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedInteger;
		break;
	}

	case texture_legacy::DXGI_R32G32_SINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 32, 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedInteger;
		break;
	}

	case texture_legacy::DXGI_R10G10B10A2_UNORM:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_R10G10B10A2_UINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 10, 10, 10, 2>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedInteger;
		break;
	}

	case texture_legacy::DXGI_R11G11B10_FLOAT:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 11, 11, 10>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R8G8B8A8_UNORM:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8G8B8A8_UNORM_SRGB:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8G8B8A8_UINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByte;
		break;
	}

	case texture_legacy::DXGI_R8G8B8A8_SNORM:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8G8B8A8_SINT:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByte;
		break;
	}

	case texture_legacy::DXGI_R16G16_FLOAT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R16G16_UNORM:
	{
		pixelType = GeneratePixelType2<'r', 'g', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16G16_UINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShort;
		break;
	}

	case texture_legacy::DXGI_R16G16_SNORM:
	{
		pixelType = GeneratePixelType2<'r', 'g', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16G16_SINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 16, 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShort;
		break;
	}

	case texture_legacy::DXGI_R32_FLOAT:
	{
		pixelType = GeneratePixelType1<'r', 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R32_UINT:
	{
		pixelType = GeneratePixelType1<'r', 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedInteger;
		break;
	}

	case texture_legacy::DXGI_R32_SINT:
	{
		pixelType = GeneratePixelType1<'r', 32>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedInteger;
		break;
	}

	case texture_legacy::DXGI_R8G8_UNORM:
	{
		pixelType = GeneratePixelType2<'r', 'g', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8G8_UINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByte;
		break;
	}

	case texture_legacy::DXGI_R8G8_SNORM:
	{
		pixelType = GeneratePixelType2<'r', 'g', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8G8_SINT:
	{
		pixelType = GeneratePixelType2<'r', 'g', 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByte;
		break;
	}

	case texture_legacy::DXGI_R16_FLOAT:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R16_UNORM:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16_UINT:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedShort;
		break;
	}

	case texture_legacy::DXGI_R16_SNORM:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShortNorm;
		break;
	}

	case texture_legacy::DXGI_R16_SINT:
	{
		pixelType = GeneratePixelType1<'r', 16>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedShort;
		break;
	}

	case texture_legacy::DXGI_R8_UNORM:
	{
		pixelType = GeneratePixelType1<'r', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8_UINT:
	{
		pixelType = GeneratePixelType1<'r', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByte;
		break;
	}

	case texture_legacy::DXGI_R8_SNORM:
	{
		pixelType = GeneratePixelType1<'r', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R8_SINT:
	{
		pixelType = GeneratePixelType1<'r', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedByte;
		break;
	}

	case texture_legacy::DXGI_A8_UNORM:
	{
		pixelType = GeneratePixelType1<'r', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R1_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::BW1bpp);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::DXGI_R9G9B9E5_SHAREDEXP:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedFloat;
		break;
	}

	case texture_legacy::DXGI_R8G8_B8G8_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::RGBG8888);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}
	case texture_legacy::DXGI_G8R8_G8B8_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::GRGB8888);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}
	case texture_legacy::DXGI_BC1_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT1);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC1_UNORM_SRGB:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT1);
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC2_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT3);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC2_UNORM_SRGB:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT1);
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC3_UNORM:
	{
		pixelType = static_cast<uint64_t>(CompressedPixelFormat::DXT5);
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC3_UNORM_SRGB:
	{
		pixelType = CompressedPixelFormat::DXT1;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC4_UNORM:
	{
		pixelType = CompressedPixelFormat::BC4;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC4_SNORM:
	{
		pixelType = CompressedPixelFormat::BC4;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC5_UNORM:
	{
		pixelType = CompressedPixelFormat::BC5;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedIntegerNorm;
		break;
	}

	case texture_legacy::DXGI_BC5_SNORM:
	{
		pixelType = CompressedPixelFormat::BC5;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::SignedIntegerNorm;
		break;
	}
	case texture_legacy::VG_sRGBX_8888:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sRGBA_8888:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sRGBA_8888_PRE:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sRGB_565:
	{
		pixelType = GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sRGBA_5551:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sRGBA_4444:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sL_8:
	{
		pixelType = GeneratePixelType1<'l', 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lRGBX_8888:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'x', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lRGBA_8888:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lRGBA_8888_PRE:
	{
		pixelType = GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_lL_8:
	{
		pixelType = GeneratePixelType1<'l', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_A_8:
	{
		pixelType = GeneratePixelType1<'a', 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_BW_1:
	{
		pixelType = CompressedPixelFormat::BW1bpp;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sXRGB_8888:
	{
		pixelType = GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sARGB_8888:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sARGB_8888_PRE:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sARGB_1555:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 1, 5, 5, 5>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sARGB_4444:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_lXRGB_8888:
	{
		pixelType = GeneratePixelType4<'x', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lARGB_8888:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lARGB_8888_PRE:
	{
		pixelType = GeneratePixelType4<'a', 'r', 'g', 'b', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sBGRX_8888:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sBGRA_8888:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sBGRA_8888_PRE:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sBGR_565:
	{
		pixelType = GeneratePixelType3<'b', 'g', 'r', 5, 6, 5>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sBGRA_5551:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 5, 5, 5, 1>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sBGRA_4444:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'x', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_lBGRX_8888:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'x', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lBGRA_8888:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lBGRA_8888_PRE:
	{
		pixelType = GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sXBGR_8888:
	{
		pixelType = GeneratePixelType4<'x', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sABGR_8888:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_sABGR_8888_PRE:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}

	case texture_legacy::VG_sABGR_1555:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 1, 5, 5, 5>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_sABGR_4444:
	{
		pixelType = GeneratePixelType4<'x', 'b', 'g', 'r', 4, 4, 4, 4>::ID;
		colorSpace = ColorSpace::sRGB;
		channelType = VariableType::UnsignedShortNorm;
		break;
	}

	case texture_legacy::VG_lXBGR_8888:
	{
		pixelType = GeneratePixelType4<'x', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lABGR_8888:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		break;
	}

	case texture_legacy::VG_lABGR_8888_PRE:
	{
		pixelType = GeneratePixelType4<'a', 'b', 'g', 'r', 8, 8, 8, 8>::ID;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::UnsignedByteNorm;
		isPremultiplied = true;
		break;
	}
	default:
	{
		pixelType = CompressedPixelFormat::NumCompressedPFs;
		colorSpace = ColorSpace::lRGB;
		channelType = VariableType::NumVarTypes;
		throw InvalidDataError("[TextureReaderPVR::mapLegacyEnumToNewFormat]: Could not match the old format to a new format");
	}
	}
}
void convertTextureHeader2To3(const texture_legacy::HeaderV2& legacyHeader, TextureHeader& newHeader)
{
	// Pixel type variables to obtain from the old header's information
	bool isPremultiplied;
	PixelFormat pixelType;
	ColorSpace colorSpace;
	VariableType channelType;

	// Map the old enum to the new format.
	mapLegacyEnumToNewFormat((texture_legacy::PixelFormat)(legacyHeader.pixelFormatAndFlags & 0xff), pixelType, colorSpace, channelType, isPremultiplied);

	// Check if this is a cube map.
	bool isCubeMap = (legacyHeader.pixelFormatAndFlags & texture_legacy::c_flagCubeMap) != 0;

	// Setup the new header based on the old values
	TextureHeader pvrTextureHeaderV3;

	// Set the pixel type obtained from the legacy format
	pvrTextureHeaderV3.flags = isPremultiplied ? (TextureHeader::PremultipliedFlag) : (0);
	pvrTextureHeaderV3.pixelFormat = pixelType.getPixelTypeId();
	pvrTextureHeaderV3.colorSpace = colorSpace;
	pvrTextureHeaderV3.channelType = channelType;

	// Set the width and height
	pvrTextureHeaderV3.height = legacyHeader.height;
	pvrTextureHeaderV3.width = legacyHeader.width;

	// Set the number of surfaces and the depth
	if ((legacyHeader.pixelFormatAndFlags & texture_legacy::c_flagVolumeTexture) != 0)
	{
		pvrTextureHeaderV3.depth = legacyHeader.numSurfaces / (isCubeMap ? 6 : 1);
		pvrTextureHeaderV3.numSurfaces = 1;
	}
	else
	{
		pvrTextureHeaderV3.depth = 1;
		pvrTextureHeaderV3.numSurfaces = legacyHeader.numSurfaces / (isCubeMap ? 6 : 1);
	}

	// Check for the elusive "PVR!\0" no surfaces bug, and attempt to correct
	if (pvrTextureHeaderV3.numSurfaces == 0) { pvrTextureHeaderV3.numSurfaces = 1; }

	// Turn the original flag into a number of faces.
	pvrTextureHeaderV3.numFaces = (isCubeMap ? 6 : 1);

	// Legacy headers have a MIP Map count of 0 if there is only the top level. New Headers have a count of 1, so add 1.
	pvrTextureHeaderV3.numMipMaps = (legacyHeader.numMipMaps + 1);

	// Initialize the header with no meta data
	pvrTextureHeaderV3.metaDataSize = 0;

	// Create the new texture header.
	newHeader = TextureHeader(pvrTextureHeaderV3);

	// Check for the texture being a normal map.
	if (legacyHeader.pixelFormatAndFlags & texture_legacy::c_flagBumpMap) { newHeader.setBumpMap(1.0f, "xyz"); }

	// Check for vertical flip orientation.
	if (legacyHeader.pixelFormatAndFlags & texture_legacy::c_flagVerticalFlip) { newHeader.setOrientation(TextureMetaData::AxisOrientationUp); }
}

} // namespace

bool isPVR(const Stream& assetStream)
{
	// Read the identifier
	texture_legacy::HeaderV2 header;
	size_t dataRead;
	assetStream.read(1, texture_legacy::c_headerSizeV2, &header, dataRead);

	// Check that the identifier matches one of the accepted formats.
	switch (header.headerSize)
	{
	case texture_legacy::c_headerSizeV1: return dataRead >= texture_legacy::c_headerSizeV1;

	case texture_legacy::c_headerSizeV2: return dataRead == texture_legacy::c_headerSizeV2 && header.pvrMagic == texture_legacy::c_identifierV2;

	case TextureHeader::PVRv3: return dataRead >= std::min(texture_legacy::c_headerSizeV2, static_cast<uint32_t>(TextureHeader::SizeOfHeader));
	}

	return false;
}
Texture readPVR(const Stream& stream)
{
	if (!stream.isReadable()) { throw InvalidOperationError("[pvr::assetReaders::readPVR] Attempted to read a non-readable assetStream"); }

	Texture asset;
	// Get the file header to Read.
	TextureHeader textureFileHeader;

	try
	{
		// Read the texture header version
		uint32_t version;
		stream.readExact(sizeof(version), 1, &version);

		if (version == TextureHeader::PVRv3)
		{
			// Read the flags
			stream.readExact(sizeof(textureFileHeader.flags), 1, &textureFileHeader.flags);

			// Read the pixel format
			stream.readExact(sizeof(textureFileHeader.pixelFormat), 1, &textureFileHeader.pixelFormat);

			// Read the color space
			stream.readExact(sizeof(textureFileHeader.colorSpace), 1, &textureFileHeader.colorSpace);

			// Read the channel type
			stream.readExact(sizeof(textureFileHeader.channelType), 1, &textureFileHeader.channelType);

			// Read the height
			stream.readExact(sizeof(textureFileHeader.height), 1, &textureFileHeader.height);

			// Read the width
			stream.readExact(sizeof(textureFileHeader.width), 1, &textureFileHeader.width);

			// Read the depth
			stream.readExact(sizeof(textureFileHeader.depth), 1, &textureFileHeader.depth);

			// Read the number of surfaces
			stream.readExact(sizeof(textureFileHeader.numSurfaces), 1, &textureFileHeader.numSurfaces);

			// Read the number of faces
			stream.readExact(sizeof(textureFileHeader.numFaces), 1, &textureFileHeader.numFaces);

			// Read the number of MIP maps
			stream.readExact(sizeof(textureFileHeader.numMipMaps), 1, &textureFileHeader.numMipMaps);

			// Read the meta data size, but store it for now.
			uint32_t tempMetaDataSize = 0;
			stream.readExact(sizeof(tempMetaDataSize), 1, &tempMetaDataSize);
			// Construct a texture header.
			// Set the meta data size to 0
			textureFileHeader.metaDataSize = 0;

			asset.initializeWithHeader(textureFileHeader);
			// Read the meta data
			uint32_t metaDataRead = 0;
			while (metaDataRead < tempMetaDataSize)
			{
				TextureMetaData metaDataBlock = loadTextureMetadataFromStream(stream);

				// Add the meta data
				asset.addMetaData(metaDataBlock);

				// Evaluate the meta data read
				metaDataRead = asset.getMetaDataSize();
			}

			// Make sure the provided data size wasn't wrong. If it was, there are no guarantees about the contents of the texture data.
			if (metaDataRead > tempMetaDataSize) { throw InvalidDataError("[TextureReaderPVR::readAsset_] Metadata seems to be corrupted while reading."); }

			// Read the texture data
			stream.readExact(1, asset.getDataSize(), asset.getDataPointer());
		}
		else if (version == texture_legacy::c_headerSizeV1 || version == texture_legacy::c_headerSizeV2)
		{
			// Read a V2 legacy header
			texture_legacy::HeaderV2 legacyHeader;

			// Read the header size
			legacyHeader.headerSize = version;

			// Read the height
			stream.readExact(sizeof(legacyHeader.height), 1, &legacyHeader.height);

			// Read the width
			stream.readExact(sizeof(legacyHeader.width), 1, &legacyHeader.width);

			// Read the MIP map count
			stream.readExact(sizeof(legacyHeader.numMipMaps), 1, &legacyHeader.numMipMaps);

			// Read the texture flags
			stream.readExact(sizeof(legacyHeader.pixelFormatAndFlags), 1, &legacyHeader.pixelFormatAndFlags);

			// Read the texture data size
			stream.readExact(sizeof(legacyHeader.dataSize), 1, &legacyHeader.dataSize);

			// Read the bit count of the texture format
			stream.readExact(sizeof(legacyHeader.bitCount), 1, &legacyHeader.bitCount);

			// Read the red mask
			stream.readExact(sizeof(legacyHeader.redBitMask), 1, &legacyHeader.redBitMask);

			// Read the green mask
			stream.readExact(sizeof(legacyHeader.greenBitMask), 1, &legacyHeader.greenBitMask);

			// Read the blue mask
			stream.readExact(sizeof(legacyHeader.blueBitMask), 1, &legacyHeader.blueBitMask);

			// Read the alpha mask
			stream.readExact(sizeof(legacyHeader.alphaBitMask), 1, &legacyHeader.alphaBitMask);

			if (version == texture_legacy::c_headerSizeV2)
			{
				// Read the magic number
				stream.readExact(sizeof(legacyHeader.pvrMagic), 1, &legacyHeader.pvrMagic);

				// Read the number of surfaces
				stream.readExact(sizeof(legacyHeader.numSurfaces), 1, &legacyHeader.numSurfaces);
			}
			else
			{
				legacyHeader.pvrMagic = texture_legacy::c_identifierV2;
				legacyHeader.numSurfaces = 1;
			}

			// Construct a texture header by converting the old one
			TextureHeader textureHeader;
			convertTextureHeader2To3(legacyHeader, textureHeader);

			// Copy the texture header to the asset.
			asset.initializeWithHeader(textureHeader);

			// Write the texture data
			for(uint32_t surface = 0; surface < asset.getNumArrayMembers(); ++surface)
			{
				for(uint32_t depth = 0; depth < asset.getDepth(); ++depth)
				{
					for(uint32_t face = 0; face < asset.getNumFaces(); ++face)
					{
						for(uint32_t mipMap = 0; mipMap < asset.getNumMipMapLevels(); ++mipMap)
						{
							uint32_t surfaceSize = asset.getDataSize(mipMap, false, false) / asset.getDepth();
							unsigned char* surfacePointer = asset.getDataPointer(mipMap, surface, face) + depth * surfaceSize;

							// Write each surface, one at a time
							stream.readExact(1, surfaceSize, surfacePointer);
						}
					}
				}
			}
		}
		else
		{
			throw InvalidDataError("[TextureReaderPVR::readAsset_]: Unsupported PVR Version");
		}
	}
	catch (const FileEOFError&)
	{
		throw InvalidDataError("[TextureReaderPVR::readAsset_]: Not a not a valid PVR file.");
	}
	return asset;
}

} // namespace assetReaders
} // namespace pvr
//!\endcond
