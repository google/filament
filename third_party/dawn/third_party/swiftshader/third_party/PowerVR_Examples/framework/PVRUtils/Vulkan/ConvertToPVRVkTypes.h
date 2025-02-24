/*!
\brief Contains conversions of pvr Enumerations to Vulkan types.
\file PVRUtils/Vulkan/ConvertToPVRVkTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
///**********************************************************************
///     NOTE
/// ThisFile has functions to:
/// convert from enums in pvr namespace to Vulkan Enums
/// Convert struct/ class from pvr type to struct/class
///**********************************************************************
#include "PVRVk/TypesVk.h"
#include "PVRCore/types/Types.h"
#include "PVRUtils/PVRUtilsTypes.h"
#include "PVRCore/texture/Texture.h"
#include "PVRVk/HeadersVk.h"
namespace pvr {
namespace utils {
/// <summary>Contain functions to convert several PowerVR Framework types to their Native, Vulkan representations,
/// usually, from an enumeration to a vulkan type.</summary>
#define PVR_DECLARE_DIRECT_MAPPING(_pvrvktype_, _frameworktype_) \
	inline _pvrvktype_ convertToPVRVk(_frameworktype_ item) { return (_pvrvktype_)item; }
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::PrimitiveTopology, ::pvr::PrimitiveTopology)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::BufferUsageFlags, ::pvr::BufferUsageFlags)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::BlendOp, ::pvr::BlendOp)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::ColorComponentFlags, ::pvr::ColorChannelFlags)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::BlendFactor, ::pvr::BlendFactor)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::StencilOp, ::pvr::StencilOp)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::SamplerAddressMode, ::pvr::SamplerAddressMode)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::Filter, ::pvr::Filter)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::SamplerMipmapMode, ::pvr::SamplerMipmapMode)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::CompareOp, ::pvr::CompareOp)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::ImageAspectFlags, ::pvr::ImageAspectFlags)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::ImageType, ::pvr::ImageType)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::DescriptorType, ::pvr::DescriptorType)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::CullModeFlags, ::pvr::Face)
PVR_DECLARE_DIRECT_MAPPING(::pvrvk::FrontFace, ::pvr::PolygonWindingOrder)

/// <summary>Specifies that the format is unsupported.</summary>
/// <param name="fmt">Pixel format</param>
#define UNSUPPORTED_FORMAT(fmt) \
	case static_cast<uint64_t>(CompressedPixelFormat::fmt): return pvrvk::Format::e_UNDEFINED;

/// <summary>Convert a pvr::IndexType to its Native, Vulkan representation.</summary>
/// <param name="type">A pvr::IndexType to convert to its Vulkan representation</param>
/// <returns>A pvrvk::IndexType corresponding to the pvr::IndexType</returns>
inline pvrvk::IndexType convertToPVRVk(IndexType type) { return (type == IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32); }

/// <summary>Convert to pvrvk image view type</summary>
/// <param name="texDimemsion">Texture dimension</param>
/// <returns>A pvrvk::ImageViewType (pvrvk::ImageViewType::e_1D, pvrvk::ImageViewType::e_2D etc)</returns>
inline pvrvk::ImageViewType convertToPVRVk(ImageViewType texDimemsion)
{
	switch (texDimemsion)
	{
	case ImageViewType::ImageView1D: return pvrvk::ImageViewType::e_1D;

	case ImageViewType::ImageView2D: return pvrvk::ImageViewType::e_2D;
	case ImageViewType::ImageView2DCube: return pvrvk::ImageViewType::e_CUBE;
	case ImageViewType::ImageView2DArray: return pvrvk::ImageViewType::e_2D_ARRAY;
	case ImageViewType::ImageView3D: return pvrvk::ImageViewType::e_3D;
	default: assertion(false, "Invalid texture dimension"); return pvrvk::ImageViewType::e_MAX_ENUM;
	}
}

/// <summary>Convert to pvrvk vertex input rate</summary>
/// <param name="stepRate">The step rate of the vertex input(Vertex, Instance)</param>
/// <returns>A pvrvk::VertexInputRate (pvrvk::VertexInputRate::e_VERTEX, pvrvk::VertexInputRate::e_INSTANCE)</returns>
inline pvrvk::VertexInputRate convertToPVRVk(StepRate stepRate) { return (stepRate == StepRate::Vertex ? pvrvk::VertexInputRate::e_VERTEX : pvrvk::VertexInputRate::e_INSTANCE); }

/// <summary>Convert to pvrvk Data type</summary>
/// <param name="dataType">The pvr::DataType to convert to Vulkan type</param>
/// <returns>A pvrvk::DataType</returns>
inline pvrvk::DataType convertToPVRVk(DataType dataType) { return static_cast<pvrvk::DataType>(dataType); }

/// <summary>Convert to pvrvk sample count</summary>
/// <param name="numSamples">Number of samples</param>
/// <returns>A pvrvk::SampleCountFlags (pvrvk::SampleCountFlags::e_1_BIT, pvrvk::SampleCountFlags::e_2_BIT, etc)</returns>
inline pvrvk::SampleCountFlags convertToPVRVkNumSamples(uint8_t numSamples)
{
	return (numSamples < 8
			? (numSamples < 2 ? pvrvk::SampleCountFlags::e_1_BIT : numSamples < 4 ? pvrvk::SampleCountFlags::e_2_BIT : pvrvk::SampleCountFlags::e_4_BIT)
			: (numSamples < 16 ? pvrvk::SampleCountFlags::e_8_BIT
							   : numSamples < 32 ? pvrvk::SampleCountFlags::e_16_BIT : numSamples < 64 ? pvrvk::SampleCountFlags::e_32_BIT : pvrvk::SampleCountFlags::e_64_BIT));
}

/// <summary>Convert to pvrvk sampler mip-map mode</summary>
/// <param name="filter">Mip map sampler filter</param>
/// <returns>A pvrvk::SamplerMipmapMode (pvrvk::SamplerMipmapMode::e_NEAREST, pvrvk::SamplerMipmapMode::e_LINEAR)</returns>
inline pvrvk::SamplerMipmapMode convertToPVRVkMipmapMode(Filter filter)
{
	return pvrvk::SamplerMipmapMode(static_cast<uint32_t>(filter) & 1); // Nearest = Nearest, Linear = Linear, None = Nearest, Cubic = linear
}

/// <summary>Convert to pvrvk format</summary>
/// <param name="dataType">Type of the data(Float32, Int32 etc)</param>
/// <param name="width">The Width of the data type</param>
/// <returns>A pvrvk::Format (pvrvk::Format::e_R32_SFLOAT, pvrvk::Format::e_R32G32_SFLOAT etc)</returns>
inline pvrvk::Format convertToPVRVkVertexInputFormat(DataType dataType, uint8_t width)
{
	static const pvrvk::Format Float32[] = { pvrvk::Format::e_R32_SFLOAT, pvrvk::Format::e_R32G32_SFLOAT, pvrvk::Format::e_R32G32B32_SFLOAT, pvrvk::Format::e_R32G32B32A32_SFLOAT };
	static const pvrvk::Format Int32[] = { pvrvk::Format::e_R32_SINT, pvrvk::Format::e_R32G32_SINT, pvrvk::Format::e_R32G32B32_SINT, pvrvk::Format::e_R32G32B32A32_SINT };
	static const pvrvk::Format UInt32[] = { pvrvk::Format::e_R32_UINT, pvrvk::Format::e_R32G32_UINT, pvrvk::Format::e_R32G32B32_UINT, pvrvk::Format::e_R32G32B32A32_UINT };
	static const pvrvk::Format Int8[] = { pvrvk::Format::e_R8_SINT, pvrvk::Format::e_R8G8_SINT, pvrvk::Format::e_R8G8B8_SINT, pvrvk::Format::e_R8G8B8A8_SINT };
	static const pvrvk::Format Int8Norm[] = { pvrvk::Format::e_R8_SNORM, pvrvk::Format::e_R8G8_SNORM, pvrvk::Format::e_R8G8B8_SNORM, pvrvk::Format::e_R8G8B8A8_SNORM };
	static const pvrvk::Format Int16[] = { pvrvk::Format::e_R16_SINT, pvrvk::Format::e_R16G16_SINT, pvrvk::Format::e_R16G16B16_SINT, pvrvk::Format::e_R16G16B16A16_SINT };
	static const pvrvk::Format Int16Norm[] = { pvrvk::Format::e_R16_SNORM, pvrvk::Format::e_R16G16_SNORM, pvrvk::Format::e_R16G16B16_SNORM, pvrvk::Format::e_R16G16B16A16_SNORM };
	static const pvrvk::Format UInt8[] = { pvrvk::Format::e_R8_UINT, pvrvk::Format::e_R8G8_UINT, pvrvk::Format::e_R8G8B8_UINT, pvrvk::Format::e_R8G8B8A8_UINT };
	static const pvrvk::Format UInt8Norm[] = { pvrvk::Format::e_R8_UNORM, pvrvk::Format::e_R8G8_UNORM, pvrvk::Format::e_R8G8B8_UNORM, pvrvk::Format::e_R8G8B8A8_UNORM };
	static const pvrvk::Format UInt16[] = { pvrvk::Format::e_R16_UINT, pvrvk::Format::e_R16G16_UINT, pvrvk::Format::e_R16G16B16_UINT, pvrvk::Format::e_R16G16B16A16_UINT };
	static const pvrvk::Format UInt16Norm[] = { pvrvk::Format::e_R16_UNORM, pvrvk::Format::e_R16G16_UNORM, pvrvk::Format::e_R16G16B16_UNORM, pvrvk::Format::e_R16G16B16A16_UNORM };
	switch (dataType)
	{
	case DataType::Float32: return Float32[width - 1];
	case DataType::Int16: return Int16[width - 1];
	case DataType::Int16Norm: return Int16Norm[width - 1];
	case DataType::Int8: return Int8[width - 1];
	case DataType::Int8Norm: return Int8Norm[width - 1];
	case DataType::UInt8: return UInt8[width - 1];
	case DataType::UInt8Norm: return UInt8Norm[width - 1];
	case DataType::UInt16: return UInt16[width - 1];
	case DataType::UInt16Norm: return UInt16Norm[width - 1];
	case DataType::Int32: return Int32[width - 1];
	case DataType::UInt32: return UInt32[width - 1];
	case DataType::RGBA: return pvrvk::Format::e_R8G8B8A8_UNORM;
	case DataType::UBYTE4: return pvrvk::Format::e_R8G8B8A8_UINT;
	case DataType::DEC3N: return pvrvk::Format::e_A2R10G10B10_UNORM_PACK32;
	case DataType::Fixed16_16: return pvrvk::Format::e_R16G16_SNORM;
	case DataType::ABGR: return pvrvk::Format::e_A8B8G8R8_UNORM_PACK32;
	case DataType::Custom:
	case DataType::None:
	default: return pvrvk::Format::e_UNDEFINED;
	}
}

/// <summary>Convert to pvrvk pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">Type of the data (SignedByte, SignedInteger etc)</param>
/// <returns>A pvrvk::Format representing the pixel format</returns>
inline pvrvk::Format convertToPVRVkPixelFormat(PixelFormat format, ColorSpace colorSpace, VariableType dataType)
{
	bool isSrgb = (colorSpace == ColorSpace::sRGB);
	bool isSigned = isVariableTypeSigned(dataType);
	if (format.getPart().High == 0) // IS COMPRESSED FORMAT!
	{
		// pvrvk::Format and type == 0 for compressed textures.
		switch (format.getPixelTypeId())
		{
			// PVRTC

		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGB): // fall through
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_2bpp_RGBA):
			return (isSrgb ? pvrvk::Format::e_PVRTC1_2BPP_SRGB_BLOCK_IMG : pvrvk::Format::e_PVRTC1_2BPP_UNORM_BLOCK_IMG);
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_2bpp):
			return (isSrgb ? pvrvk::Format::e_PVRTC2_2BPP_SRGB_BLOCK_IMG : pvrvk::Format::e_PVRTC2_2BPP_UNORM_BLOCK_IMG);
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCII_4bpp):
			return (isSrgb ? pvrvk::Format::e_PVRTC2_4BPP_SRGB_BLOCK_IMG : pvrvk::Format::e_PVRTC2_4BPP_UNORM_BLOCK_IMG);

		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGB):
			return isSrgb ? pvrvk::Format::e_PVRTC1_4BPP_SRGB_BLOCK_IMG : pvrvk::Format::e_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		case static_cast<uint64_t>(CompressedPixelFormat::PVRTCI_4bpp_RGBA):
			return isSrgb ? pvrvk::Format::e_PVRTC1_4BPP_SRGB_BLOCK_IMG : pvrvk::Format::e_PVRTC1_4BPP_UNORM_BLOCK_IMG;

		// OTHER COMPRESSED
		case static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5): return pvrvk::Format::e_E5B9G9R9_UFLOAT_PACK32;
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB): return (isSrgb ? pvrvk::Format::e_ETC2_R8G8B8_SRGB_BLOCK : pvrvk::Format::e_ETC2_R8G8B8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGBA): return (isSrgb ? pvrvk::Format::e_ETC2_R8G8B8A8_SRGB_BLOCK : pvrvk::Format::e_ETC2_R8G8B8A8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ETC2_RGB_A1): return (isSrgb ? pvrvk::Format::e_ETC2_R8G8B8A1_SRGB_BLOCK : pvrvk::Format::e_ETC2_R8G8B8A1_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_R11): return (isSigned ? pvrvk::Format::e_EAC_R11_SNORM_BLOCK : pvrvk::Format::e_EAC_R11_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::EAC_RG11): return (isSigned ? pvrvk::Format::e_EAC_R11G11_SNORM_BLOCK : pvrvk::Format::e_EAC_R11G11_UNORM_BLOCK);

		case static_cast<uint64_t>(CompressedPixelFormat::BC1): return (isSrgb ? pvrvk::Format::e_BC1_RGBA_SRGB_BLOCK : pvrvk::Format::e_BC1_RGBA_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC2): return (isSrgb ? pvrvk::Format::e_BC2_SRGB_BLOCK : pvrvk::Format::e_BC2_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC3): return (isSrgb ? pvrvk::Format::e_BC3_SRGB_BLOCK : pvrvk::Format::e_BC3_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC4): return (isSigned ? pvrvk::Format::e_BC4_SNORM_BLOCK : pvrvk::Format::e_BC4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC5): return (isSigned ? pvrvk::Format::e_BC5_SNORM_BLOCK : pvrvk::Format::e_BC5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC6): return (isSigned ? pvrvk::Format::e_BC6H_SFLOAT_BLOCK : pvrvk::Format::e_BC6H_UFLOAT_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::BC7): return (isSrgb ? pvrvk::Format::e_BC7_SRGB_BLOCK : pvrvk::Format::e_BC7_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x10): return (isSrgb ? pvrvk::Format::e_ASTC_10x10_SRGB_BLOCK : pvrvk::Format::e_ASTC_10x10_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x5): return (isSrgb ? pvrvk::Format::e_ASTC_10x5_SRGB_BLOCK : pvrvk::Format::e_ASTC_10x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x6): return (isSrgb ? pvrvk::Format::e_ASTC_10x6_SRGB_BLOCK : pvrvk::Format::e_ASTC_10x6_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_10x8): return (isSrgb ? pvrvk::Format::e_ASTC_10x8_SRGB_BLOCK : pvrvk::Format::e_ASTC_10x8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x10): return (isSrgb ? pvrvk::Format::e_ASTC_12x10_SRGB_BLOCK : pvrvk::Format::e_ASTC_12x10_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_12x12): return (isSrgb ? pvrvk::Format::e_ASTC_12x12_SRGB_BLOCK : pvrvk::Format::e_ASTC_12x12_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_4x4): return (isSrgb ? pvrvk::Format::e_ASTC_4x4_SRGB_BLOCK : pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x4): return (isSrgb ? pvrvk::Format::e_ASTC_5x4_SRGB_BLOCK : pvrvk::Format::e_ASTC_5x4_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_5x5): return (isSrgb ? pvrvk::Format::e_ASTC_5x5_SRGB_BLOCK : pvrvk::Format::e_ASTC_5x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_6x5): return (isSrgb ? pvrvk::Format::e_ASTC_6x5_SRGB_BLOCK : pvrvk::Format::e_ASTC_6x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x5): return (isSrgb ? pvrvk::Format::e_ASTC_8x5_SRGB_BLOCK : pvrvk::Format::e_ASTC_8x5_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x6): return (isSrgb ? pvrvk::Format::e_ASTC_8x6_SRGB_BLOCK : pvrvk::Format::e_ASTC_8x6_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::ASTC_8x8): return (isSrgb ? pvrvk::Format::e_ASTC_8x8_SRGB_BLOCK : pvrvk::Format::e_ASTC_8x8_UNORM_BLOCK);
		case static_cast<uint64_t>(CompressedPixelFormat::RGBG8888): return (isSrgb ? pvrvk::Format::e_UNDEFINED : pvrvk::Format::e_G8B8G8R8_422_UNORM);
		case static_cast<uint64_t>(CompressedPixelFormat::GRGB8888):
			return (isSrgb ? pvrvk::Format::e_UNDEFINED : pvrvk::Format::e_B8G8R8G8_422_UNORM);

			///////// UNSUPPORTED FORMATS
			UNSUPPORTED_FORMAT(ETC1);
			UNSUPPORTED_FORMAT(DXT2)
			UNSUPPORTED_FORMAT(DXT4)
			UNSUPPORTED_FORMAT(UYVY)
			UNSUPPORTED_FORMAT(YUY2)
			UNSUPPORTED_FORMAT(BW1bpp)
			UNSUPPORTED_FORMAT(ASTC_3x3x3)
			UNSUPPORTED_FORMAT(ASTC_4x3x3)
			UNSUPPORTED_FORMAT(ASTC_4x4x3)
			UNSUPPORTED_FORMAT(ASTC_4x4x4)
			UNSUPPORTED_FORMAT(ASTC_5x4x4)
			UNSUPPORTED_FORMAT(ASTC_5x5x4)
			UNSUPPORTED_FORMAT(ASTC_5x5x5)
			UNSUPPORTED_FORMAT(ASTC_6x5x5)
			UNSUPPORTED_FORMAT(ASTC_6x6x5)
			UNSUPPORTED_FORMAT(ASTC_6x6x6)
		}
	}
	else
	{
		bool depthOrStencil = (format.getChannelContent(0) == 'd' || format.getChannelContent(0) == 's' || format.getChannelContent(1) == 'd');
		if (depthOrStencil)
		{
			switch (format.getPixelTypeId())
			{
			case GeneratePixelType1<'d', 32>::ID: return pvrvk::Format::e_D32_SFLOAT;
			case GeneratePixelType1<'d', 24>::ID:
			case GeneratePixelType2<'x', 8, 'd', 24>::ID:
			case GeneratePixelType2<'d', 24, 'x', 8>::ID: return pvrvk::Format::e_D32_SFLOAT;
			case GeneratePixelType1<'d', 16>::ID: return pvrvk::Format::e_D16_UNORM;
			case GeneratePixelType2<'d', 's', 32, 8>::ID: return pvrvk::Format::e_D32_SFLOAT_S8_UINT;
			case GeneratePixelType2<'d', 's', 24, 8>::ID: return pvrvk::Format::e_D24_UNORM_S8_UINT;
			case GeneratePixelType2<'d', 's', 16, 8>::ID: return pvrvk::Format::e_D16_UNORM_S8_UINT;
			case GeneratePixelType1<'s', 8>::ID: return pvrvk::Format::e_S8_UINT;
			}
		}
		else
		{
			switch (dataType)
			{
			case VariableType::UnsignedFloat:
				if (format.getPixelTypeId() == GeneratePixelType3<'b', 'g', 'r', 10, 11, 11>::ID) { return pvrvk::Format::e_B10G11R11_UFLOAT_PACK32; }
				break;
			case VariableType::SignedFloat:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16A16_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16_SFLOAT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return pvrvk::Format::e_R16G16_SFLOAT;
				case GeneratePixelType1<'r', 16>::ID: return pvrvk::Format::e_R16_SFLOAT;
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return pvrvk::Format::e_R16G16_SFLOAT;
				case GeneratePixelType1<'l', 16>::ID: return pvrvk::Format::e_R16_SFLOAT;
				case GeneratePixelType1<'a', 16>::ID: return pvrvk::Format::e_R16_SFLOAT;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32A32_SFLOAT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32_SFLOAT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return pvrvk::Format::e_R32G32_SFLOAT;
				case GeneratePixelType1<'r', 32>::ID: return pvrvk::Format::e_R32_SFLOAT;
				case GeneratePixelType2<'l', 'a', 32, 32>::ID: return pvrvk::Format::e_R32G32_SFLOAT;
				case GeneratePixelType1<'l', 32>::ID: return pvrvk::Format::e_R32_SFLOAT;
				case GeneratePixelType1<'a', 32>::ID: return pvrvk::Format::e_R32_SFLOAT;
				}
				break;
			}
			case VariableType::UnsignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_R8G8B8A8_SRGB : pvrvk::Format::e_R8G8B8A8_UNORM);
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_R8G8B8_SRGB : pvrvk::Format::e_R8G8B8_UNORM);
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return pvrvk::Format::e_R8G8_UNORM;
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return (isSrgb ? pvrvk::Format::e_R8_SRGB : pvrvk::Format::e_R8_UNORM);
				case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_B8G8R8A8_SRGB : pvrvk::Format::e_B8G8R8A8_UNORM);
				case GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_B8G8R8_SRGB : pvrvk::Format::e_B8G8R8_UNORM);
				case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return pvrvk::Format::e_R4G4B4A4_UNORM_PACK16;
				}
			}
			case VariableType::SignedByteNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8A8_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8_SNORM;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID:
				case GeneratePixelType2<'l', 'a', 8, 8>::ID: return pvrvk::Format::e_R8G8B8_SNORM;
				case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_B8G8R8A8_SRGB : pvrvk::Format::e_B8G8R8A8_SNORM);
				case GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID: return (isSrgb ? pvrvk::Format::e_B8G8R8_SRGB : pvrvk::Format::e_B8G8R8_SNORM);
				case GeneratePixelType1<'r', 8>::ID:
				case GeneratePixelType1<'l', 8>::ID:
				case GeneratePixelType1<'a', 8>::ID: return pvrvk::Format::e_R8_SNORM; break;
				}
			}
			case VariableType::UnsignedByte:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8A8_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8_UINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return pvrvk::Format::e_R8G8_UINT;
				case GeneratePixelType1<'r', 8>::ID: return pvrvk::Format::e_R8_UINT;
				case GeneratePixelType4<'b', 'g', 'r', 'a', 8, 8, 8, 8>::ID: return pvrvk::Format::e_B8G8R8A8_UINT;
				case GeneratePixelType3<'b', 'g', 'r', 8, 8, 8>::ID: return pvrvk::Format::e_B8G8R8_UINT;
				}
			}
			case VariableType::SignedByte:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 8, 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8A8_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 8, 8, 8>::ID: return pvrvk::Format::e_R8G8B8_SINT;
				case GeneratePixelType2<'r', 'g', 8, 8>::ID: return pvrvk::Format::e_R8G8_SINT;
				case GeneratePixelType1<'r', 8>::ID: return pvrvk::Format::e_R8_SINT;
				}
				break;
			}
			case VariableType::UnsignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 4, 4, 4, 4>::ID: return pvrvk::Format::e_R4G4B4A4_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 5, 5, 5, 1>::ID: return pvrvk::Format::e_R5G5B5A1_UNORM_PACK16;
				case GeneratePixelType3<'r', 'g', 'b', 5, 6, 5>::ID: return pvrvk::Format::e_R5G6B5_UNORM_PACK16;
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16A16_UNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16_UNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return pvrvk::Format::e_R16G16_UNORM;
				case GeneratePixelType2<'d', 16, 's', 8>::ID: return pvrvk::Format::e_D16_UNORM_S8_UINT;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID: return pvrvk::Format::e_R16_UNORM;
				}
				break;
			}
			case VariableType::SignedShortNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16A16_SNORM;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16_SNORM;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID:
				case GeneratePixelType2<'l', 'a', 16, 16>::ID: return pvrvk::Format::e_R16G16_SNORM;
				case GeneratePixelType1<'r', 16>::ID:
				case GeneratePixelType1<'l', 16>::ID:
				case GeneratePixelType1<'a', 16>::ID: return pvrvk::Format::e_R16_SNORM;
				}
				break;
			}
			case VariableType::UnsignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16A16_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16_UINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return pvrvk::Format::e_R16G16_UINT;
				case GeneratePixelType1<'r', 16>::ID: return pvrvk::Format::e_R16_UINT;
				}
				break;
			}
			case VariableType::SignedShort:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 16, 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16A16_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 16, 16, 16>::ID: return pvrvk::Format::e_R16G16B16_SINT;
				case GeneratePixelType2<'r', 'g', 16, 16>::ID: return pvrvk::Format::e_R16G16_SINT;
				case GeneratePixelType1<'r', 16>::ID: return pvrvk::Format::e_R16_SINT;
				}
				break;
			}
			case VariableType::UnsignedIntegerNorm:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID:
				case GeneratePixelType4<'x', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return pvrvk::Format::e_A2B10G10R10_UNORM_PACK32;
				}
				break;
			}
			case VariableType::UnsignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32A32_UINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32_UINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return pvrvk::Format::e_R32G32_UINT;
				case GeneratePixelType1<'r', 32>::ID: return pvrvk::Format::e_R32_UINT;
				case GeneratePixelType4<'a', 'b', 'g', 'r', 2, 10, 10, 10>::ID: return pvrvk::Format::e_A2B10G10R10_UINT_PACK32;
				}
				break;
			}
			case VariableType::SignedInteger:
			{
				switch (format.getPixelTypeId())
				{
				case GeneratePixelType4<'r', 'g', 'b', 'a', 32, 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32A32_SINT;
				case GeneratePixelType3<'r', 'g', 'b', 32, 32, 32>::ID: return pvrvk::Format::e_R32G32B32_SINT;
				case GeneratePixelType2<'r', 'g', 32, 32>::ID: return pvrvk::Format::e_R32G32_SINT;
				case GeneratePixelType1<'r', 32>::ID: return pvrvk::Format::e_R32_SINT;
				}
				break;
			}
			default:
			{
				break;
			}
			}
		}
	}

	return pvrvk::Format::e_UNDEFINED;
}

/// <summary>Created a packed sampler filter</summary>
/// <param name="mini">The minification filter</param>
/// <param name="magni">The magnification filter</param>
/// <param name="mip">The sampler mipmap mode</param>
/// <returns>The packed sampler filter</returns>
inline PackedSamplerFilter packSamplerFilter(pvrvk::Filter mini, pvrvk::Filter magni, pvrvk::SamplerMipmapMode mip)
{
	return PackedSamplerFilter((PackedSamplerFilter)mini + ((PackedSamplerFilter)magni << 2) + ((PackedSamplerFilter)mip << 4));
}

/// <summary>Unpack a packed sampler filter</summary>
/// <param name="packed">The packed sampler filter</param>
/// <param name="mini">The minification filter</param>
/// <param name="magni">The magnification filter</param>
/// <param name="mip">The sampler mipmap mode</param>
inline void unpackSamplerFilter(PackedSamplerFilter packed, pvrvk::Filter& mini, pvrvk::Filter& magni, pvrvk::SamplerMipmapMode& mip)
{
	mini = (pvrvk::Filter)(packed & 3);
	magni = (pvrvk::Filter)((packed >> 2) & 3);
	mip = (pvrvk::SamplerMipmapMode)(packed >> 4);
}

/// <summary>Convert to pvrvk pixel format</summary>
/// <param name="format">Image Data format</param>
/// <returns>A pvrvk::Format representing the pixel format</returns>
inline pvrvk::Format convertToPVRVk(const ImageDataFormat& format) { return convertToPVRVkPixelFormat(format.format, format.colorSpace, format.dataType); }

/// <summary>Convert to pvrvk pixel format</summary>
/// <param name="format">Pixel format</param>
/// <param name="colorSpace">Color space of the format (lRGB, sRGB)</param>
/// <param name="dataType">TYpe of the data (SignedByte, SignedInteger etc)</param>
/// <param name="outIsCompressedFormat">Return if its a compressed format</param>
/// <returns>A pvrvk::Format representing the pixel format</returns>
inline pvrvk::Format convertToPVRVkPixelFormat(PixelFormat format, ColorSpace colorSpace, VariableType dataType, bool& outIsCompressedFormat)
{
	outIsCompressedFormat = (format.getPart().High == 0) && (format.getPixelTypeId() != static_cast<uint64_t>(CompressedPixelFormat::SharedExponentR9G9B9E5));
	return convertToPVRVkPixelFormat(format, colorSpace, dataType);
}

/// <summary>Convert to pvrvk StencilOpState</summary>
/// <param name="op">The pvr::StencilState to convert</param>
/// <returns>A pvrvk::StencilOpState representing the StencilState</returns>
inline pvrvk::StencilOpState convertToPVRVk(const StencilState& op)
{
	return pvrvk::StencilOpState(
		convertToPVRVk(op.opStencilFail), convertToPVRVk(op.opDepthPass), convertToPVRVk(op.opDepthFail), convertToPVRVk(op.compareOp), op.compareMask, op.writeMask, op.reference);
}

/// <summary>Convert to pvrvk PipelineColorBlendAttachmentState</summary>
/// <param name="config">The pvr::BlendingConfig to convert</param>
/// <returns>A pvrvk::PipelineColorBlendAttachmentState representing the BlendingConfig</returns>
inline pvrvk::PipelineColorBlendAttachmentState convertToPVRVk(const BlendingConfig& config)
{
	return pvrvk::PipelineColorBlendAttachmentState(config.blendEnable, convertToPVRVk(config.srcBlendColor), convertToPVRVk(config.dstBlendColor), convertToPVRVk(config.blendOpColor),
		convertToPVRVk(config.srcBlendAlpha), convertToPVRVk(config.dstBlendAlpha), convertToPVRVk(config.blendOpAlpha), convertToPVRVk(config.channelWriteMask));
}

/// <summary>Convert to pvrvk VertexInputAttributeDescription</summary>
/// <param name="info">The pvr::VertexAttributeInfo to convert</param>
/// <param name="binding">The binding index to use as part of the pvrvk::VertexInputAttributeDescription</param>
/// <returns>A pvrvk::VertexInputAttributeDescription representing the VertexAttributeInfo with corresponding binding index</returns>
inline pvrvk::VertexInputAttributeDescription convertToPVRVk(const VertexAttributeInfo& info, uint32_t binding)
{
	return pvrvk::VertexInputAttributeDescription(info.index, binding, convertToPVRVkVertexInputFormat(info.format, info.width), info.offsetInBytes);
}

/// <summary>Convert to pvrvk VertexInputBindingDescription</summary>
/// <param name="info">The pvr::VertexInputBindingInfo to convert</param>
/// <returns>A pvrvk::VertexInputBindingDescription representing the VertexInputBindingInfo</returns>
inline pvrvk::VertexInputBindingDescription convertToPVRVk(const VertexInputBindingInfo& info)
{
	return pvrvk::VertexInputBindingDescription(info.bindingId, info.strideInBytes, convertToPVRVk(info.stepRate));
}

/// <summary>Convert to pvrvk Extent3D</summary>
/// <param name="extent">The pvr::Extent3D to convert</param>
/// <returns>A pvrvk::Extent3D representing the Extent3D</returns>
inline pvrvk::Extent3D convertToPVRVk(const Extent3D& extent) { return pvrvk::Extent3D{ extent.width, extent.height, extent.depth }; }

/// <summary>Convert to pvrvk Extent2D</summary>
/// <param name="extent">The pvr::Extent2D to convert</param>
/// <returns>A pvrvk::Extent2D representing the Extent2D</returns>
inline pvrvk::Extent2D convertToPVRVk(const Extent2D& extent) { return pvrvk::Extent2D{ extent.width, extent.height }; }

/// <summary>Convert to pvrvk Offset3D</summary>
/// <param name="offset">The pvr::Offset3D to convert</param>
/// <returns>A pvrvk::Offset3D representing the Offset3D</returns>
inline pvrvk::Offset3D convertToPVRVk(const Offset3D& offset) { return pvrvk::Offset3D{ offset.x, offset.y, offset.z }; }

/// <summary>Convert to pvrvk Offset2D</summary>
/// <param name="offset">The pvr::Offset2D to convert</param>
/// <returns>A pvrvk::Offset2D representing the Offset2D</returns>
inline pvrvk::Offset2D convertToPVRVk(const Offset2D& offset) { return pvrvk::Offset2D{ offset.x, offset.y }; }
#undef UNSUPPORTED_FORMAT
} // namespace utils
} // namespace pvr
