/*!
\brief Contains conversions of pvr Enumerations to OpenGL ES types.
\file PVRUtils/OpenGLES/ConvertToGlesTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/texture/Texture.h"
#include "PVRUtils/OpenGLES/ErrorsGles.h"

namespace pvr {
namespace utils {
/// <summary>Retrieves the OpenGL ES texture formats for a texture with pvr::PixelFormat pixelFormat, pvr::ColorSpace colorSpace and pvr::VariableType dataType.</summary>
/// <param name="pixelFormat">The pvr::PixelFormat to retrieve a corresponding set of OpenGL ES texture formats for.</param>
/// <param name="colorSpace">The pvr::ColorSpace to retrieve a corresponding set of OpenGL ES texture formats for.</param>
/// <param name="dataType">The pvr::VariableTypeto retrieve a corresponding set of OpenGL ES texture formats for.</param>
/// <param name="glInternalFormat">The internal OpenGL ES format corresponding to the provided pvr::PixelFormat pixelFormat, pvr::ColorSpace
/// colorSpace and pvr::VariableType dataType.</param>
/// <param name="glFormat">The OpenGL ES format corresponding to the provided pvr::PixelFormat pixelFormat, pvr::ColorSpace
/// colorSpace and pvr::VariableType dataType.</param>
/// <param name="glType">The OpenGL ES type corresponding to the provided pvr::PixelFormat pixelFormat, pvr::ColorSpace
/// colorSpace and pvr::VariableType dataType.</param>
/// <param name="glTypeSize">The OpenGL ES type size corresponding to the provided pvr::PixelFormat pixelFormat, pvr::ColorSpace
/// colorSpace and pvr::VariableType dataType.</param>
/// <param name="isCompressedFormat">Specifies whether the OpenGL ES format retrieved is a compressed format.</param>
void getOpenGLFormat(PixelFormat pixelFormat, ColorSpace colorSpace, VariableType dataType, uint32_t& glInternalFormat, uint32_t& glFormat, uint32_t& glType, uint32_t& glTypeSize,
	bool& isCompressedFormat);

/// <summary>Retrieves the OpenGL ES texture formats for a texture with the provided ImageStorageFormat.</summary>
/// <param name="storageFormat">The pvr::ImageStorageFormat to retrieve a corresponding set of OpenGL ES texture formats for.</param>
/// <param name="glInternalFormat">The internal OpenGL ES format corresponding to the provided pvr::ImageStorageFormat storageFormat</param>
/// <param name="glFormat">The OpenGL ES format corresponding to the provided pvr::ImageStorageFormat storageFormat</param>
/// <param name="glType">The OpenGL ES type corresponding to the provided pvr::ImageStorageFormat storageFormat</param>
/// <param name="glTypeSize">The OpenGL ES type size corresponding to the provided pvr::ImageStorageFormat storageFormat</param>
/// <param name="isCompressedFormat">Specifies whether the OpenGL ES format retrieved is a compressed format.</param>
inline void getOpenGLFormat(ImageStorageFormat storageFormat, uint32_t& glInternalFormat, uint32_t& glFormat, uint32_t& glType, uint32_t& glTypeSize, bool& isCompressedFormat)
{
	getOpenGLFormat(storageFormat.format, storageFormat.colorSpace, storageFormat.dataType, glInternalFormat, glFormat, glType, glTypeSize, isCompressedFormat);
}

/// <summary>Retrieves the internal OpenGL ES texture format for a texture with pvr::PixelFormat pixelFormat, pvr::ColorSpace colorSpace and pvr::VariableType dataType.</summary>
/// <param name="pixelFormat">The pvr::PixelFormat to retrieve a corresponding internal OpenGL ES texture format for.</param>
/// <param name="colorSpace">The pvr::ColorSpace to retrieve a corresponding internal OpenGL ES texture format for.</param>
/// <param name="dataType">The pvr::VariableTypeto retrieve a corresponding internal OpenGL ES texture format for.</param>
/// <param name="glInternalFormat">The internal OpenGL ES format corresponding to the provided pvr::PixelFormat pixelFormat, pvr::ColorSpace
/// colorSpace and pvr::VariableType dataType</param>
void getOpenGLStorageFormat(PixelFormat pixelFormat, ColorSpace colorSpace, VariableType dataType, GLenum& glInternalFormat);

/// <summary>Retrieves the internal OpenGL ES texture formats for a texture with the provided ImageStorageFormat.</summary>
/// <param name="storageFormat">The pvr::ImageStorageFormat to retrieve a the internal OpenGL ES texture format for.</param>
/// <param name="glInternalFormat">The internal OpenGL ES format corresponding to the provided pvr::ImageStorageFormat storageFormat</param>
/// <returns>Returns 'True' if the utility function was able to successfully determin the internal OpenGL ES format corresponding to the provided pvr::ImageStorageFormat
/// storageFormat.</returns>
inline void getOpenGLStorageFormat(ImageStorageFormat storageFormat, GLenum& glInternalFormat)
{
	getOpenGLStorageFormat(storageFormat.format, storageFormat.colorSpace, storageFormat.dataType, glInternalFormat);
}

/// <summary>Converts from a pvr::IndexType to its OpenGL ES GLenum counterpart.</summary>
/// <param name="type">The pvr::IndexType to convert.</param>
/// <returns>The OpenGL ES GLenum counterpart to a pvr::IndexType.</returns>
inline GLenum convertToGles(IndexType type) { return static_cast<GLenum>((type == pvr::IndexType::IndexType16Bit ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT)); }

/// <summary>Convert to opengl face.</summary>
/// <param name="face">A Face enum</param>
/// <returns>A GLenum representing a face (GL_FRONT, GL_BACK, GL_FRONT_AND_BACK, GL_NONE)</returns>
GLenum convertToGles(Face face);

/// <summary>Convert to opengl winding-order.</summary>
/// <param name="windingOrder">A PolygonWindingOrder enum</param>
/// <returns>A GLenum representing a winding order (GL_CW, GL_CCW)</returns>
GLenum convertToGles(PolygonWindingOrder windingOrder);

/// <summary>Convert to opengl comparison mode.</summary>
/// <param name="func">A ComparisonMode enum</param>
/// <returns>A GLenum representing a ComparisonMode (GL_LESS, GL_EQUAL etc)</returns>
GLenum convertToGles(CompareOp func);

/// <summary>Convert to an opengl image aspect type.</summary>
/// <param name="type">An ImageAspectFlags enum</param>
/// <returns>A GLenum representing the ImageAspectFlags</returns>
GLenum convertToGles(ImageAspectFlags type);

/// <summary>Convert to opengl texture type.</summary>
/// <param name="texType">A TextureDimension enum</param>
/// <returns>A GLenum representing a texture dimension (GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP,
/// GL_TEXTURE_2D_ARRAY)</returns>
GLenum convertToGles(ImageViewType texType);

/// <summary>Convert to opengl data type.</summary>
/// <param name="dataType">A DataType enum</param>
/// <returns>A GLenum representing a DataType (GL_FLOAT, GL_UNSIGNED_BYTE etc)</returns>
GLenum convertToGles(DataType dataType);

/// <summary>Convert to opengl priitive type.</summary>
/// <param name="primitiveType">a PrimitiveTopology enum</param>
/// <returns>A GLenum representing a primitive type (GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_POINTS etc)</returns>
GLenum convertToGles(PrimitiveTopology primitiveType);

/// <summary>Convert to opengl sampler wrap.</summary>
/// <param name="addressMode">A SamplerAddressMode enum</param>
/// <returns>A GLenum representing a Sampler Wrap mode (GL_CLAMP_TO_EDGE, GL_REPEAT etc)</returns>
GLenum convertToGles(SamplerAddressMode addressMode);

/// <summary>Convert to opengl stencil op output.</summary>
/// <param name="stencilOp">A StencilOp enum</param>
/// <returns>A GLenum representing a Stencil Operation (GL_INC_WRAP, GL_ZERO etc)</returns>
GLenum convertToGles(StencilOp stencilOp);

/// <summary>Convert to opengl blend op output.</summary>
/// <param name="blendOp">A BlendOp enum</param>
/// <returns>A GLenum representing a Blend Operation (GL_FUNC_ADD, GL_MIN etc)</returns>
GLenum convertToGles(BlendOp blendOp);

/// <summary>Convert to opengl blend factor output.</summary>
/// <param name="blendFactor">A BlendFactor enum</param>
/// <returns>A GLenum representing a BlendFactor (GL_ZERO, GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA etc)</returns>
GLenum convertToGles(BlendFactor blendFactor);

} // namespace utils
} // namespace pvr
