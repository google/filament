/*!
\brief Contains structures, classes and enums used throughout PVRUtils.
\file PVRUtils/PVRUtilsTypes.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"

namespace pvr {
namespace utils {
/// <summary>Contains a full description of a Vertex Attribute: Index, format, number of elements, offset in the
/// buffer, optionally name. All values (except attributeName) must be set explicitly.</summary>
struct VertexAttributeInfo
{
	uint16_t index; //!< Attribute index
	DataType format; //!< Data type of each element of the attribute
	uint8_t width; //!< Number of elements in attribute, e.g 1,2,3,4
	uint32_t offsetInBytes; //!< Offset of the first element in the buffer
	std::string attribName; //!< Optional: Name(in the shader) of the attribute

	/// <summary>Default constructor. Uninitialized values, except for AttributeName.</summary>
	VertexAttributeInfo() : index(0), format(DataType::None), width(0), offsetInBytes(0), attribName("") {}

	/// <summary>Create a new VertexAttributeInfo object.</summary>
	/// <param name="index">Attribute binding index</param>
	/// <param name="format">Attribute data type</param>
	/// <param name="width">Number of elements in attribute</param>
	/// <param name="offsetInBytes">Interleaved: offset of the attribute from the start of data of each vertex</param>
	/// <param name="attribName">Name of the attribute in the shader.</param>
	VertexAttributeInfo(uint16_t index, DataType format, uint8_t width, uint32_t offsetInBytes, const char* attribName = "")
		: index(index), format(format), width(width), offsetInBytes(offsetInBytes), attribName(attribName)
	{}

	/// <summary>Return true if the right hand object is equal to this</summary>
	/// <param name=rhs>The right hand side of the operator</param>
	/// <returns>True if index, format, width and offset are all equal, otherwise false</returns>
	bool operator==(VertexAttributeInfo const& rhs) const
	{
		return ((index == rhs.index) && (format == rhs.format) && (width == rhs.width) && (offsetInBytes == rhs.offsetInBytes));
	}

	/// <summary>Return true if the right hand object is not equal to this</summary>
	/// <param name=rhs>The right hand side of the operator</param>
	/// <returns>True if at least one of index, format, width and offset is not equal,
	/// otherwise false</returns>
	bool operator!=(VertexAttributeInfo const& rhs) const { return !((*this) == rhs); }
};

/// <summary>Information about a Buffer binding: Binding index, stride, (instance) step rate.</summary>
struct VertexInputBindingInfo
{
	uint16_t bindingId; //!< buffer binding index
	uint32_t strideInBytes; //!< buffer stride in bytes
	StepRate stepRate; //!< buffer step rate

	/// <summary>Construct with Uninitialized values.</summary>
	VertexInputBindingInfo() : bindingId(static_cast<uint16_t>(-1)), strideInBytes(static_cast<uint32_t>(-1)), stepRate(StepRate::Default) {}

	/// <summary>Add a buffer binding.</summary>
	/// <param name="bindId">Buffer binding point</param>
	/// <param name="strideInBytes">Buffer stride of each vertex attribute to the next</param>
	/// <param name="stepRate">Vertex Attribute Step Rate</param>
	VertexInputBindingInfo(uint16_t bindId, uint32_t strideInBytes, StepRate stepRate = StepRate::Vertex) : bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};

/// <summary>A container struct carrying Vertex Attribute information (vertex layout, plus binding point)</summary>
struct VertexAttributeInfoWithBinding : public VertexAttributeInfo
{
	/// <summary>The Vertex Buffer binding point this attribute is bound to</summary>
	uint16_t binding;

	/// <summary>Constructor</summary>
	VertexAttributeInfoWithBinding() : binding(static_cast<uint16_t>(-1)) {}

	/// <summary>Constructor from VertexAttributeInfo and Binding</summary>
	/// <param name="nfo">A vertexAttributeInfo</param>
	/// <param name="binding">The VBO binding index from where this vertex attribute will be sourced</param>
	VertexAttributeInfoWithBinding(const VertexAttributeInfo& nfo, uint16_t binding) : VertexAttributeInfo(nfo), binding(binding) {}

	/// <summary>Constructor from individual values</summary>
	/// <param name="index">The index of the vertex attribute</param>
	/// <param name="format">The vertex attribute format</param>
	/// <param name="width">The number of elements in the vertex attribute (e.g. 4 for a vec4)</param>
	/// <param name="offsetInBytes">The offset of the vertex attribute from the start of the buffer</param>
	/// <param name="binding">The VBO binding index from where this vertex attribute will be sourced</param>
	/// <param name="attribName">The Attribute name (optional, only required/supported in some apis)</param>
	VertexAttributeInfoWithBinding(uint16_t index, DataType format, uint8_t width, uint32_t offsetInBytes, uint16_t binding, const char* attribName = "")
		: VertexAttributeInfo(index, format, width, offsetInBytes, attribName), binding(binding)
	{}
};
} // namespace utils
} // namespace pvr
