/*!
\brief Basic file used in the PowerVR Framework. Defines several types used throughout the Framework (sized
arithmetic types, enumerations, character types).
\file PVRCore/types/Types.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

//!\cond NO_DOXYGEN
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef GLM_FORCE_SSE2
#define GLM_FORCE_SSE2
#endif
#endif
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
//!\endcond

#if defined(X11)
// undef these macros from the xlib files, they are breaking the framework types.
#undef Success
#undef Enum
#undef None
#undef Always
#undef byte
#undef char8
#undef ShaderStageFlags
#undef capability
#endif

#include <vector>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <memory>
#include <algorithm>
#include <functional>
#include <string>
#include <stdint.h>
#include <limits>
#include <assert.h>
#include <type_traits>

/*! \brief Macro that defines all common bitwise operators for an enum-class */
#define DEFINE_ENUM_OPERATORS(type_) \
\
	inline type_ operator|(type_ lhs, type_ rhs) \
\
	{ \
		return static_cast<type_>(static_cast<std::underlying_type<type_>::type /**/>(lhs) | static_cast<std::underlying_type<type_>::type /**/>(rhs)); \
	} \
\
	inline void operator|=(type_& lhs, type_ rhs) \
\
	{ \
		lhs = static_cast<type_>(static_cast<std::underlying_type<type_>::type /**/>(lhs) | static_cast<std::underlying_type<type_>::type /**/>(rhs)); \
	} \
\
	inline type_ operator&(type_ lhs, type_ rhs) \
\
	{ \
		return static_cast<type_>(static_cast<std::underlying_type<type_>::type /**/>(lhs) & static_cast<std::underlying_type<type_>::type /**/>(rhs)); \
	} \
\
	inline void operator&=(type_& lhs, type_ rhs) \
\
	{ \
		lhs = static_cast<type_>(static_cast<std::underlying_type<type_>::type /**/>(lhs) & static_cast<std::underlying_type<type_>::type /**/>(rhs)); \
	}

namespace pvr {

/// <summary>Enumeration of all API types supported by this implementation</summary>
enum class Api
{
	Unspecified = 0,
	// OpenGL,
	OpenGLES2,
	OpenGLES3,
	OpenGLES31,
	OpenGLESMaxVersion = OpenGLES31,
	Vulkan,
	NumApis
};

/// <summary>Get a string of the specific api enum</summary>
/// <param name="api">The api</param>
/// <returns>Api code</returns>
inline const char* apiCode(Api api)
{
	static const char* ApiCodes[] = {
		"",
		"ES2",
		"ES3",
		"ES31",
		"vk",
	};
	return ApiCodes[static_cast<int>(api)];
}

/// <summary>Get the api name std::string of the given Enumeration</summary>
/// <param name="api">The api</param>
/// <returns>Api name std::string</returns>
inline const char* apiName(Api api)
{
	static const char* ApiCodes[] = {
		"Unknown",
		"OpenGL ES 2.0",
		"OpenGL ES 3.0",
		"OpenGL ES 3.1",
		"Vulkan",
	};
	return ApiCodes[static_cast<int>(api)];
}

/// <summary>Enumeration of all the different descriptor types.</summary>
enum class DescriptorType : uint32_t
{
	// DO NOT RE-ARRANGE THIS
	Sampler, //!< A Sampler object
	CombinedImageSampler, //!< A descriptor that contains both and image and its sampler
	SampledImage, //!< Aka "Texture"
	StorageImage, //!< Aka "Image for Image Load Store"
	UniformTexelBuffer, //!< Aka Texture Buffer
	StorageTexelBuffer, //!< Also known as TextureBuffer
	UniformBuffer, //!< Aka UBO
	StorageBuffer, //!< Aka SSBO
	UniformBufferDynamic, //!< A UBO that can be bound one piece at a time
	StorageBufferDynamic, //!< A SSBO that can be bound one piece at a time
	InputAttachment, //!< An intermediate attachment that can be used between subpasses
	Count = 12,
	NumBits = 4
};

/// <summary>Enumeration of all supported buffer use types.</summary>
enum class BufferUsageFlags : uint32_t
{
	TransferSrc = 0x00000001, //!< Transfer Source
	TransferDest = 0x00000002, //!< Transfer Destination
	UniformTexelBuffer = 0x00000004, //!< Uniform Texel Buffer
	StorageTexelBuffer = 0x00000008, //!< Storage Texel Buffer
	UniformBuffer = 0x00000010, //!< UBO
	StorageBuffer = 0x00000020, //!< SSBO
	IndexBuffer = 0x00000040, //!< IBO
	VertexBuffer = 0x00000080, //!< VBO
	IndirectBuffer = 0x00000100, //!< A buffer that contains Draw Indirect commands
	Count = 10
};
DEFINE_ENUM_OPERATORS(BufferUsageFlags)

/// <summary>Infer the BufferUsageFlags that are suitable for the typical use of an object</summary>
/// <param name="descType">A descriptor type</param>
/// <returns>The typical usage flags for <paramref name="descType/></returns>
inline BufferUsageFlags descriptorTypeToBufferUsage(DescriptorType descType)
{
	if (descType == DescriptorType::UniformBuffer || descType == DescriptorType::UniformBufferDynamic) { return BufferUsageFlags::UniformBuffer; }
	return BufferUsageFlags::StorageBuffer;
}

/// <summary>Checks if a descriptor type is dynamic (a dynamic UBO or dynamic SSBO)</summary>
/// <param name="descType">A descriptor type</param>
/// <returns>True if descType is UniformBufferDynamic or StorageBufferDynamic, otherwise false</returns>
inline bool isDescriptorTypeDynamic(DescriptorType descType) { return (descType == DescriptorType::UniformBufferDynamic || descType == DescriptorType::StorageBufferDynamic); }

/// <summary>Aligns a given number based on the given alignment</summary>
/// <param name="numberToAlign">A number ot align based alignment</param>
/// <param name="alignment">The value to which the numberToAlign will be aligned</param>
/// <returns>An aligned value</returns>
template<typename t1, typename t2>
inline t1 align(t1 numberToAlign, t2 alignment)
{
	if (alignment)
	{
		t1 align1 = numberToAlign % (t1)alignment;
		if (!align1) { align1 += (t1)alignment; }
		numberToAlign += t1(alignment) - align1;
	}
	return numberToAlign;
}

/// <summary>An enumeration that defines data types used throughout the Framework. Commonly used in places where
/// raw data are used to define the types actually contained.</summary>
enum class DataType
{
	None, //!< None, or unknown
	Float32, //!< 32 bit floating point number
	Int32, //!< 32 bit Integer
	UInt16, //!< 16 bit Unsigned Integer (aka Unsigned Short)
	RGBA, //!< 32 bit (4 channels x 8bpc), in Red,Green,Blue,Alpha order
	ARGB, //!< 32 bit (4 channels x 8bpc), in Alpha,Red,Green,Blue order
	D3DCOLOR, //!< Direct3D color format
	UBYTE4, //!< Direct3D UBYTE4 format
	DEC3N, //!< Direct3D DEC3N format
	Fixed16_16, //!< 32 bit Fixed Point (16 + 16)
	UInt8, //!< Unsigned 8 bit integer (aka unsigned char/byte)
	Int16, //!< Signed 16 bit integer (aka short)
	Int16Norm, //!< Signed 16 bit integer scaled to a value from -1..1 (aka normalized short)
	Int8, //!< Signed 8 bit integer (aka char / byte)
	Int8Norm, //!< Signed 8 bit integer, interpreted by scaling to -1..1 (aka normalized byte)
	UInt8Norm, //!< Unsigned 8 bit integer,  interpreted by scaling to 0..1 (aka unsigned normalized byte)
	UInt16Norm, //!< Unsigned 16 bit integer,  interpreted by scaling to 0..1 (aka unsigned normalized short)
	UInt32, //!< Unsigned 32 bit integer (aka Unsigned Int)
	ABGR, //!< 32 bit (4 channels x 8 bpc), in Alpha,Blue,Green,Red order
	Float16, //!< 16 bit  IEEE 754-2008 floating point number (aka Half)
	Custom = 1000
};

/// <summary>Return the Size of a DataType.</summary>
/// <param name="type">The Data type</param>
/// <returns>The size of the Datatype in bytes.</returns>
inline uint32_t dataTypeSize(DataType type)
{
	switch (type)
	{
	default: assert(false); return 0;
	case DataType::Float32:
	case DataType::Int32:
	case DataType::UInt32:
	case DataType::RGBA:
	case DataType::ABGR:
	case DataType::ARGB:
	case DataType::D3DCOLOR:
	case DataType::UBYTE4:
	case DataType::DEC3N: //(1D/2D/3D).
	case DataType::Fixed16_16: return 4;
	case DataType::Int16:
	case DataType::Int16Norm:
	case DataType::UInt16: return 2;
	case DataType::UInt8:
	case DataType::UInt8Norm:
	case DataType::Int8:
	case DataType::Int8Norm: return 1;
	}
}
/// <summary>Return the number of components in a datatype.</summary>
/// <param name="type">The datatype</param>
/// <returns>The number of components (e.g. float is 1, vec3 is 3)</returns>
inline uint32_t numDataTypeComponents(DataType type)
{
	switch (type)
	{
	default: assert(false); return 0;
	case DataType::Float32:
	case DataType::Int32:
	case DataType::UInt32:
	case DataType::Int16:
	case DataType::Int16Norm:
	case DataType::UInt16:
	case DataType::Fixed16_16:
	case DataType::Int8:
	case DataType::Int8Norm:
	case DataType::UInt8:
	case DataType::UInt8Norm: return 1;
	case DataType::DEC3N: return 3;
	case DataType::RGBA:
	case DataType::ABGR:
	case DataType::ARGB:
	case DataType::D3DCOLOR:
	case DataType::UBYTE4: return 4;
	}
}

/// <summary>Return if the format is Normalized (represents a range between 0..1 for unsigned types or between -1..1
/// for signed types)</summary>
/// <param name="type">The format to test.</param>
/// <returns>True if the format is Normalised.</returns>
/// <remarks>A Normalised format is a value that is stored as an Integer, but that actually represents a value
/// from 0..1 or -1..1 instead of the numeric value of the Integer. For example, for a normalised unsigned char
/// value, the value 0 represents 0.0, the value 127 represents 0.5 and the value 255 represents 1.0.</remarks>
inline bool dataTypeIsNormalised(DataType type)
{
	return (type == DataType::Int8Norm || type == DataType::UInt8Norm || type == DataType::Int16Norm || type == DataType::UInt16Norm);
}

/// <summary>Enumeration containing all possible Primitive topologies (Point, line trianglelist etc.).</summary>
enum class PrimitiveTopology : uint32_t
{
	// POSITION-SENSITIVE. Do not renumber unless also refactoring ConvertToPVRVkTypes.
	PointList, //!< Renders poins
	LineList, //!< Each two items render a separate line segment
	LineStrip, //!< Renders one continuous polyline (n vertices represent n-1 lines)
	TriangleList, //!< Each 3 vertices render one triangle
	TriangleStrip, //!< Renders one continuous triangle strip, (n vertices represent n-2 triangles in a strip configuration)
	TriangleFan, //!< Renders one continuous triangle fan (n vertices represent n-2 triangles in a fan configuration)
	LineListWithAdjacency, //!< Represents a list of lines, but contains adjacency info (2 additional vertices per 2 vertices: 4 vertices per line segment)
	LineStripWithAdjacency, //!< Represents a continuous strip of lines, but contains adjacency info (2 additional vertices: the vertex before the first and the vertex after the
							//!< last line segment)
	TriangleListWithAdjacency, //!< Represents a triangle list with adjacency info (6 vertices per primitive).
	TriangleStripWithAdjacency, //!< Represents a triangle strip with adjacency info (1 additional adjacency vertex per triangle, plus the adjacent vertices of the first and last
								//!< triangle sides of the list).
	PatchList, //!< A list of Patches, intended for tessellation
	Count
};

/// <summary>ChannelWriteMask enable/ disable writting to channel bits.</summary>
enum class ColorChannelFlags : uint32_t
{
	// DO NOT REARRANGE - Direct mapping to Vulkan
	R = 0x01, //!< write to red channel
	G = 0x02, //!< write to green channel
	B = 0x04, //!< write to blue channel
	A = 0x08, //!< write to alpha channel
	None = 0, //!< don't write to any channel
	All = R | G | B | A //< write to all channel
};
DEFINE_ENUM_OPERATORS(ColorChannelFlags)

/// <summary>Step rate for a vertex attribute when drawing: Per vertex, per instance, per draw.</summary>
enum class StepRate : uint32_t
{
	Vertex, //!< Step rate Per vertex
	Instance, //!< Step rate per instance
	Default = Vertex
};

/// <summary>Enumeration of Face facing (front, back...).</summary>
enum class Face : uint32_t
{
	// DO NOT REARRANGE - DIRECT TO VULKAN
	None = 0, //!< No faces
	Front = 1, //!< The front face
	Back = 2, //!< The back face
	FrontAndBack = 3, //!< Both faces
	Default = None
};

/// <summary>Enumeration of the blend operations (determine how a new pixel (source color) is combined with a pixel
/// already in the framebuffer (destination color).</summary>
enum class BlendOp : uint32_t
{
	// DO NOT REARRANGE - Direct mapping to Vulkan. See convertToPVRVk
	Add, //!< Addition
	Subtract, //!< Subtraction second from first
	ReverseSubtract, //!< Subtract first from second
	Min, //!< Minimum of the two
	Max, //!< Maximum of the two
	NumBlendFunc,
	Default = Add
};

/// <summary>Specfies how the rgba blending facors are computed for source and destination fragments.</summary>
enum class BlendFactor : uint8_t
{
	Zero, //!< Zero
	One, //!< One
	SrcColor, //!< The color of the incoming fragment
	OneMinusSrcColor, //!< 1 - (SourceColor)
	DstColor, //!< The color of the pixel already in the framebuffer
	OneMinusDstColor, //!< 1 - (Destination Color)
	SrcAlpha, //!< The alpha of the incoming fragment
	OneMinusSrcAlpha, //!< 1- (Source Alpha)
	DstAlpha, //!< The alpha of the pixel already in the framebuffer (requires an alpha channel)
	OneMinusDstAlpha, //!< 1- (Destination Alpha)
	ConstantColor, //!< A constant color provided by the api
	OneMinusConstantColor, //!< 1- (Constant Color)
	ConstantAlpha, //!< A constant alpha value provided by the api
	OneMinusConstantAlpha, //!< 1- (ConstantAlpha)
	Src1Color, //!< Source Color 1
	OneMinusSrc1Color, //!< 1 - (Source Color 1)
	Src1Alpha, //!< Source Alpha 1
	OneMinusSrc1Alpha, //!< 1 - (Source Alpha 1)
	NumBlendFactor,
	DefaultSrcRgba = One,
	DefaultDestRgba = Zero
};

/// <summary>Enumeration of the different front face to winding order correlations.</summary>
enum class PolygonWindingOrder : uint8_t
{
	// DO NOT REARRANGE - VULKAN DIRECT MAPPING
	FrontFaceCCW, //!< Front face is the Counter Clockwise face
	FrontFaceCW, //!< Front face is the Clockwise face
	Default = FrontFaceCCW
};

/// <summary>Enumeration of the different stencil operations.</summary>
enum class StencilOp : uint8_t
{
	// DO NOT REARRANGE - VULKAN DIRECT MAPPING
	Keep, //!< Keep existing value
	Zero, //!< Set to zero
	Replace, //!< Replace value with Ref
	IncrementClamp, //!< Increment until max value
	DecrementClamp, //!< Decrement until min value
	Invert, //!< Bitwise-not the existing value
	IncrementWrap, //!< Increment the existing value, wrap if >max
	DecrementWrap, //!< Decrement the existing value, wrap if <min
	NumStencilOp,

	// Defaults
	Default = Keep,
};

/// <summary>Capability supported values.</summary>
enum class Capability : uint8_t
{
	Unsupported, //!< The capability is unsupported
	Immutable, //!< The capability exists but cannot be changed
	Mutable //!< The capability is supported and can be changed
};

/// <summary>An enumeration that defines a type that can use as an index, typically 16 or 32 bit int. Especially
/// used in Model classes.</summary>
enum class IndexType : uint32_t
{
	IndexType16Bit = static_cast<uint32_t>(DataType::UInt16), //!< 16 bit index
	IndexType32Bit = static_cast<uint32_t>(DataType::UInt32) //!< 32 bit index
};

/// <summary>Return the Size of an IndexType in bytes.</summary>
/// <param name="type">The Index type</param>
/// <returns>The number of bytes in an index type</returns>
inline uint32_t indexTypeSizeInBytes(const IndexType type)
{
	switch (type)
	{
	default: assert(false); return false;
	case IndexType::IndexType16Bit: return 2;
	case IndexType::IndexType32Bit: return 4;
	}
}

/// <summary>An enumeration that defines Comparison operations (equal, less or equal etc.). Especially used in
/// API classes for functions like depth testing.</summary>
enum class CompareOp : uint32_t
{
	// DIRECT MAPPING FOR VULKAN - DO NOT REARRANGE
	Never = 0, //!< Always false
	Less = 1, //!< True if lhs<rhs
	Equal = 2, //!< True if lhs==rhs
	LessEqual = 3, //!< True if lhs<=rhs
	Greater = 4, //!< True if lhs>rhs
	NotEqual = 5, //!< True if lhs!=rhs
	GreaterEqual = 6, //!< True if lhs>=rhs
	Always = 7, //!< Always true
	NumComparisonMode,
	DefaultDepthFunc = Less,
	DefaultStencilFunc = Always,
};

/// <summary>Enumeration describing a filtering type of a specific dimension. In order to describe the filtering mode
/// properly, you would have to define a Minification filter, a Magnification filter and a Mipmapping minification
/// filter. Possible values: Nearest, Linear, Cubic, None.</summary>
enum class Filter : uint8_t
{
	Nearest, //!< Nearest neighbour
	Linear, //< Linear (average weighted by distance)
	None, //!< No filtering
	Cubic, //!< Bicubic filtering (IMG extension)
	Default = Linear,
	MipDefault = Linear,
	Size = 4
};

/// <summary>Enumeration for defining texture wrapping mode: Repeat, Mirror, Clamp, Border.</summary>
enum class SamplerAddressMode : uint8_t
{
	Repeat, //!< repeat
	MirrorRepeat, //!< mirror repeat
	ClampToEdge, //!< clamp
	ClampToBorder, //!< border
	MirrorClampToEdge, //!< mirror clamp
	Size,
	Default = Repeat
};

/// <summary>Enumeration of mipmap modes supported for a sampler</summary>
enum class SamplerMipmapMode : uint8_t
{
	Nearest, //!< Nearest neighbour
	Linear, //!< Linear
	Count
};

/// <summary>This enum is made to pack all sampler filtering info in 8 bits for specific uses. Use "packSamplerFilter" and "unpackSamplerFilter".
/// NOTE: The defined values are only the most common cases - other 8 bit values are also valid (for example, different minification and magnification filters)</summary>
enum PackedSamplerFilter : int8_t
{
	PackNone, //< no filter
	PackNearestMipNone = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) |
		(static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //<Nearest Neighbour, no mipmap use
	PackNearestMipNearest = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) |
		(static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //<Nearest Neighbour per mipmap, nearest neighbour between mipmaps
	PackNearestMipLinear = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Nearest) | (static_cast<uint8_t>(Filter::Nearest) << 2) |
		(static_cast<uint8_t>(SamplerMipmapMode::Linear) << 4)), //<Nearest Neighbour, linearly interpolate between mipmaps (OpenGL Default)
	PackLinearMipNone = static_cast<uint8_t>(
		static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) | (static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //< Bilinear, no mipmap use
	PackLinearMipNearest = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) |
		(static_cast<uint8_t>(SamplerMipmapMode::Nearest) << 4)), //< Bilinear, nearest neighbour between mipmaps
	PackTrilinear = static_cast<uint8_t>(static_cast<uint8_t>(Filter::Linear) | (static_cast<uint8_t>(Filter::Linear) << 2) |
		(static_cast<uint8_t>(SamplerMipmapMode::Linear) << 4)), //< Full Trilinear (bilinear, linearly interpolate between mipmaps)
	Size, //< number of supported filter
	PackDefault = PackTrilinear //< default filter
};

/// <summary>Pack a minification filter, a magnification filter and a mipmap filter into an 8 bit value</summary>
/// <param name="mini">The filtering mode that should be used for minification</param>
/// <param name="magni">The filtering mode that should be used for magnification</param>
/// <param name="mip">The filtering mode that should be used for mipmapping</param>
/// <returns>An 8 bit value representing the described sampler</returns>
inline PackedSamplerFilter packSamplerFilter(Filter mini, Filter magni, SamplerMipmapMode mip)
{
	return PackedSamplerFilter(static_cast<PackedSamplerFilter>(mini) + (static_cast<PackedSamplerFilter>(magni) << 2) + (static_cast<PackedSamplerFilter>(mip) << 4));
}

/// <summary>Unpack a 8 bit PackedSamplerFilter value into a minification, magnification and mip filter mode</summary>
/// <param name="packed">The packed sampler filter to unpack</param>
/// <param name="mini">The filtering mode that should be used for minification</param>
/// <param name="magni">The filtering mode that should be used for magnification</param>
/// <param name="mip">The filtering mode that should be used for mipmapping</param>
/// <returns>An 8 bit value representing the described sampler</returns>
inline void unpackSamplerFilter(PackedSamplerFilter packed, Filter& mini, Filter& magni, SamplerMipmapMode& mip)
{
	mini = static_cast<Filter>(packed & 3);
	magni = static_cast<Filter>((packed >> 2) & 3);
	mip = static_cast<SamplerMipmapMode>(packed >> 4);
}

/// <summary>Enumeration of all supported shader types.</summary>
enum class ShaderType
{
	UnknownShader = 0, //!< unknown shader type
	VertexShader, //!< vertex shader
	FragmentShader, //!< fragment shader
	ComputeShader, //!< compute shader
	TessControlShader,
	TessEvaluationShader,
	GeometryShader,
	RayShader,
	FrameShader,
	Count
};

/// <summary>Converts a pvr::ShaderType to string</summary>
/// <param name="shaderType">The type of the shader</param>
/// <returns>A stringified version of pvr::ShaderType</returns>
inline std::string to_string(ShaderType shaderType)
{
	switch (shaderType)
	{
	case ShaderType::VertexShader: return "Vertex";
	case ShaderType::FragmentShader: return "Fragment";
	case ShaderType::ComputeShader: return "Compute";
	case ShaderType::TessControlShader: return "Tessellation Control";
	case ShaderType::TessEvaluationShader: return "Tessellation Evaluation";
	case ShaderType::GeometryShader: return "Geometry";
	case ShaderType::RayShader: return "Ray";
	case ShaderType::FrameShader: return "Frame";
	default: return "Unknown";
	}
}

/// <summary>Pre-defined Result codes (success and generic errors).</summary>
enum class Result
{
	Success,
	UnknownError,
	NotInitialized,
	// Shell Error
	InitializationError,
	UnsupportedRequest,

	ExitRenderFrame, // Used to exit the renderscene loop in the shell
};
/// <summary>Use this function to convert a Result into a std::string that is suitable for outputting.</summary>
/// <param name="result">The result</param>
/// <returns>A std::string suitable for writing out that represents this Result</returns>
inline const char* getResultCodeString(Result result)
{
	switch (result)
	{
	case Result::Success: return "Success";
	case Result::UnknownError: return "Unknown Error";
	case Result::ExitRenderFrame: return "Exit Render Scene";
	case Result::NotInitialized: return "Not initialized";
	case Result::InitializationError: return "Error while initializing";
	default: return "UNRECOGNIZED CODE";
	}
}

/// <summary>Represents a buffer of Unsigned Bytes. Used to store raw data.</summary>
typedef ::std::vector<unsigned char> UInt8Buffer;

/// <summary>Representation of raw data. Used to store raw data that is logically grouped in blocks with a stride.</summary>
class StridedBuffer : public UInt8Buffer
{
public:
	uint16_t stride; //!< The stride of the buffer
};

/// <summary>Get a random Number between min and max</summary>
/// <param name="min">Minimum number (inclusive)</param>
/// <param name="max">Maximum number (inclusive)</param>
/// <returns>Random number</returns>
inline float randomrange(float min, float max)
{
	float zero_to_one = static_cast<float>(double(rand()) / double(RAND_MAX));
	float diff = max - min;
	return zero_to_one * diff + min;
}

/// <summary>The Vertical Synchronization (or lack thereof) modes, A.K.A. Presentation mode.</summary>
enum class VsyncMode
{
	Off, //!< The application does not synchronizes with the vertical sync. If application renders faster than the display refreshes, frames are wasted and tearing may be observed.
		 //!< FPS is uncapped. Maximum power consumption. If unsupported, "ON" value will be used instead. Minimum latency.
	On, //!< The application is always syncrhonized with the vertical sync. Tearing does not happen. FPS is capped to the display's refresh rate. For fast applications, battery
		//!< life is improved. Always supported.
	Relaxed, //!< The application synchronizes with the vertical sync, but only if the application rendering speed is greater than refresh rate. Compared to OFF, there is no
			 //!< tearing. Compared to ON, the FPS will be improved for "slower" applications. If unsupported, "ON" value will be used instead. Recommended for most applications.
			 //!< Default if supported.
	Mailbox, //!< The presentation engine will always use the latest fully rendered image. Compared to OFF, no tearing will be observed. Compared to ON, battery power will be
			 //!< worse, especially for faster applications. If unsupported,  "OFF" will be attempted next.
	Half, //!< The application is capped to using half the vertical sync time. FPS artificially capped to Half the display speed (usually 30fps) to maintain battery. Best possible
		  //!< battery savings. Worst possibly performance. Recommended for specific applications where battery saving is critical.
};

/// <summary>Contains display configuration information (width, height, position, title, bpp etc.).</summary>
class DisplayAttributes
{
public:
	/// <summary>Unnamed enum for constants</summary>
	enum
	{
		PosDefault = -1 //!< Sentinel value for Default position
	};

	std::string windowTitle; //!< Title of the application window

	uint32_t width; //!< Width of the rendering area (default 1280)
	uint32_t height; //!< Height of the rendering (default 800)
	uint32_t x; //!< Horizontal offset of the bottom-left area (default 0)
	uint32_t y; //!< Vertical offset of the bottom-left area (default 0)

	uint32_t depthBPP; //!< Number of bits per pixel in the depth buffer (default 16)
	uint32_t stencilBPP; //!< Number of bits per pixel of the stencil buffer (default 0: no stencil)

	uint32_t redBits; //!< Number of bits of the red channel of the framebuffer (default 8)
	uint32_t greenBits; //!< Number of bits of the green channel of the framebuffer (default 8)
	uint32_t blueBits; //!< Number of bits of the blue channel of the framebuffer (default 8)
	uint32_t alphaBits; //!< Number of bits of the alpha channel of the framebuffer (default 8)

	uint32_t aaSamples; //!< Number of (antialiasing) samples of the framebuffer (default 0)

	uint32_t configID; //!< Deprecated: EGL config id

	VsyncMode vsyncMode; //!< Type of syncrhonization mode (default On: Vsync)
	int32_t contextPriority; //!< Context priority, if supported (default High)
	int32_t swapLength; //!< Swapchain length, AKA number of framebuffer images (default 0: Minimum required for technique)

	bool forceColorBPP; //!< Require that the color channels of the framebuffer are exactly as requested in redBits/blueBits/greenBits/alphaBits (default false)
	bool fullscreen; //!< If true, application will be fullscreen (if supported). If false, application will be windowed (if supported). (default false)
	bool frameBufferSrgb; //!< If true and supported, attempt to use an sRGB framebuffer format (default false)

	// Default constructor
	DisplayAttributes()
		: width(1280), height(800), x(static_cast<uint32_t>(0)), y(static_cast<uint32_t>(0)), depthBPP(32u), stencilBPP(0u), redBits(8u), greenBits(8u), blueBits(8u),
		  alphaBits(8u), aaSamples(0u), configID(0u), vsyncMode(VsyncMode::On), contextPriority(2), swapLength(0), forceColorBPP(false), fullscreen(false), frameBufferSrgb(true)
	{}
	/// <summary>Checks if the screen is rotated.</summary>
	/// <returns>True if the screen is Portrait, otherwise (if landscape) false .</returns>
	bool isDisplayPortrait() const { return height > width; }
	/// <summary>Checks if full screen.</summary>
	/// <returns>True if full screen, otherwise false.</returns>
	bool isFullScreen() const { return fullscreen; }
};

/// <summary>Native connection type. Used as a connection between a client side window system library (Xlib, XCB) and its corresponding server side window system library.</summary>
typedef void* OSConnection;

/// <summary>Native display type.</summary>
typedef void* OSDisplay;

/// <summary>Native window type.</summary>
typedef void* OSWindow;

/// <summary>Native application type.</summary>
typedef void* OSApplication;

/// <summary>Native application data type.</summary>
typedef void* OSDATA;

/// <summary>Enumeration of texture Swizzle mask channels.</summary>
enum class Swizzle : uint8_t // DIRECT VULKAN MAPPING - DO NOT REARRANGE
{
	Identity = 0,
	// Unset = 0,
	Zero = 1,
	One = 2,
	R = 3,
	G = 4,
	B = 5,
	A = 6,
	Red = R,
	Green = G,
	Blue = B,
	Alpha = A,
};

/// <summary>The SwizzleChannels struct</summary>
struct SwizzleChannels
{
	Swizzle r; //!< Swizzle R channel
	Swizzle g; //!< Swizzle G channel
	Swizzle b; //!< Swizzle B channel
	Swizzle a; //!< Swizzle A channel

	/// <summary>SwizzleChannels. Default: All channels are set to identity</summary>
	SwizzleChannels() : r(Swizzle::Identity), g(Swizzle::Identity), b(Swizzle::Identity), a(Swizzle::Identity) {}

	/// <summary>SwizzleChannels</summary>
	/// <param name="r">Swizzle R channel</param>
	/// <param name="g">Swizzle G channel</param>
	/// <param name="b">Swizzle B channel</param>
	/// <param name="a">Swizzle A channel</param>
	SwizzleChannels(Swizzle r, Swizzle g, Swizzle b, Swizzle a) : r(r), g(g), b(b), a(a) {}
};

/// <summary>This class contains all the information of a Vertex Attribute's layout inside a block of memory,
/// typically a Vertex Buffer Object. This informations is normally the DataType of the attribute, the Offset (from
/// the beginning of the array) and the width (how many values of type DataType form an attribute).</summary>
struct VertexAttributeLayout
{
	DataType dataType; //!< Type of data of the vertex data
	uint16_t offset; //!< Offset, in bytes, of this vertex attribute
	uint8_t width; //!< Number of values per vertex

	/// <summary>VertexAttributeLayout</summary>
	VertexAttributeLayout() : dataType(DataType::None), offset(static_cast<uint16_t>(-1)), width(static_cast<uint8_t>(-1)) {}

	/// <summary>VertexAttributeLayout</summary>
	/// <param name="dataType"></param>
	/// <param name="width"></param>
	/// <param name="offset"></param>
	VertexAttributeLayout(DataType dataType, uint8_t width, uint16_t offset) : dataType(dataType), offset(offset), width(width) {}
};

/// <summary>Add blending configuration for a color attachment. Some API's only support one blending state for all
/// attachments, in which case the 1st such configuration will be used for all.</summary>
/// <remarks>--- Defaults --- Blend Enabled:false, Source blend Color factor: false, Destination blend Color
/// factor: Zero, Source blend Alpha factor: Zero, Destination blending Alpha factor :Zero, Blending operation
/// color: Add, Blending operation alpha: Add, Channel writing mask: All</remarks>
struct BlendingConfig
{
	bool blendEnable; //!< Enable blending
	BlendFactor srcBlendColor; //!< Source Blending color factor
	BlendFactor dstBlendColor; //!< Destination blending color factor
	BlendOp blendOpColor; //!< Blending operation color
	BlendFactor srcBlendAlpha; //!< Source blending alpha factor
	BlendFactor dstBlendAlpha; //!< Destination blending alpha factor
	BlendOp blendOpAlpha; //!< Blending operation alpha
	ColorChannelFlags channelWriteMask; //!< Channel writing mask

	/// <summary>Create a blending state. Separate color/alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendColor">Source Blending color factor (default:Zero)</param>
	/// <param name="dstBlendColor">Destination blending color factor (default:Zero)</param>
	/// <param name="srcBlendAlpha">Source blending alpha factor (default:Zero)</param>
	/// <param name="dstBlendAlpha">Destination blending alpha factor (default:Zero)</param>
	/// <param name="blendOpColor">Blending operation color (default:Add)</param>
	/// <param name="blendOpAlpha">Blending operation alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(bool blendEnable = false, BlendFactor srcBlendColor = BlendFactor::One, BlendFactor dstBlendColor = BlendFactor::Zero, BlendOp blendOpColor = BlendOp::Add,
		BlendFactor srcBlendAlpha = BlendFactor::One, BlendFactor dstBlendAlpha = BlendFactor::Zero, BlendOp blendOpAlpha = BlendOp::Add,
		ColorChannelFlags channelWriteMask = ColorChannelFlags::All)
		: blendEnable(blendEnable), srcBlendColor(srcBlendColor), dstBlendColor(dstBlendColor), blendOpColor(blendOpColor), srcBlendAlpha(srcBlendAlpha),
		  dstBlendAlpha(dstBlendAlpha), blendOpAlpha(blendOpAlpha), channelWriteMask(channelWriteMask)
	{}

	/// <summary>Create a blending state. Common color and alpha factors.</summary>
	/// <param name="blendEnable">Enable blending (default false)</param>
	/// <param name="srcBlendFactor">Source Blending factor</param>
	/// <param name="dstBlendFactor">Destination Blending factor</param>
	/// <param name="blendOpColorAlpha">Blending operation color & alpha (default:Add)</param>
	/// <param name="channelWriteMask">Channel writing mask (default:All)</param>
	BlendingConfig(bool blendEnable, BlendFactor srcBlendFactor, BlendFactor dstBlendFactor, BlendOp blendOpColorAlpha, ColorChannelFlags channelWriteMask = ColorChannelFlags::All)
		: blendEnable(blendEnable), srcBlendColor(srcBlendFactor), dstBlendColor(dstBlendFactor), blendOpColor(blendOpColorAlpha), srcBlendAlpha(srcBlendFactor),
		  dstBlendAlpha(dstBlendFactor), blendOpAlpha(blendOpColorAlpha), channelWriteMask(channelWriteMask)
	{}
};

/// <summary>Pipeline Stencil state</summary>
struct StencilState
{
	StencilOp opDepthPass; //!< Action performed on samples that pass both the depth and stencil tests.
	StencilOp opDepthFail; //!< Action performed on samples that pass the stencil test and fail the depth test.
	StencilOp opStencilFail; //!< Action performed on samples that fail the stencil test.
	uint32_t compareMask; //!< Selects the bits of the unsigned Integer stencil values during in the stencil test.
	uint32_t writeMask; //!<  Selects the bits of the unsigned Integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32_t reference; //!< Integer reference value that is used in the unsigned stencil comparison.
	CompareOp compareOp; //!<  Comparison operator used in the stencil test.

	/// <summary>Constructor from all parameters</summary>
	/// <param name="depthPass">Action performed on samples that pass both the depth and stencil tests.</param>
	/// <param name="depthFail">Action performed on samples that pass the stencil test and fail the depth test.</param>
	/// <param name="stencilFail">Action performed on samples that fail the stencil test.</param>
	/// <param name="compareOp">Comparison operator used in the stencil test.</param>
	/// <param name="compareMask">Selects the bits of the unsigned Integer stencil values during in the stencil test.</param>
	/// <param name="writeMask">Selects the bits of the unsigned Integer stencil values updated by the stencil test in the
	/// stencil framebuffer attachment</param>
	/// <param name="reference">Integer reference value that is used in the unsigned stencil comparison.</param>
	StencilState(StencilOp depthPass = StencilOp::Keep, StencilOp depthFail = StencilOp::Keep, StencilOp stencilFail = StencilOp::Keep,
		CompareOp compareOp = CompareOp::DefaultStencilFunc, uint32_t compareMask = 0xff, uint32_t writeMask = 0xff, uint32_t reference = 0)
		: opDepthPass(depthPass), opDepthFail(depthFail), opStencilFail(stencilFail), compareMask(compareMask), writeMask(writeMask), reference(reference), compareOp(compareOp)
	{}
};

//!\cond NO_DOXYGEN
#if defined(_MSC_VER)
#define PVR_ALIGNED __declspec(align(16))
#elif defined(__GNUC__) || defined(__clang__)
#define PVR_ALIGNED __attribute__((aligned(16)))
#else
#define PVR_ALIGNED alignas(16)
#endif
//!\endcond
} // namespace pvr

/// <summary>ARRAY_SIZE(a) is a compile-time constant which represents the number of elements of the given
/// array. ONLY use ARRAY_SIZE for statically allocated arrays.</summary>
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#undef DEFINE_ENUM_OPERATORS
