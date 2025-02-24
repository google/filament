#pragma once
#include <cstdint>
#include "PVRCore/types/Types.h"

namespace pvr {
/// <summary>Groups functionality that has to do with bit calculations/sizes/offsets of glsl types</summary>
namespace GpuDatatypesHelper {
/// <summary>A bit representing if a type is basically of integer or floating point format</summary>
enum class BaseType
{
	Integer = 0,
	Float = 1
};
/// <summary>Two bits, representing the number of vector components (from scalar up to 4)</summary>
enum class VectorWidth
{
	Scalar = 0,
	Vec2 = 1,
	Vec3 = 2,
	Vec4 = 3,
};
/// <summary>Three bits, representing the number of matrix columns (from not a matrix to 4)</summary>
enum class MatrixColumns
{
	OneCol = 0,
	Mat2x = 1,
	Mat3x = 2,
	Mat4x = 3
};

/// <summary>Contains bit enums for the expressiveness of the GpuDatatypes class' definition</summary>
enum class Bits : uint32_t
{
	Integer = 0,
	Float = 1,
	BitScalar = 0,
	BitVec2 = 2,
	BitVec3 = 4,
	BitVec4 = 6,
	BitOneCol = 0,
	BitMat2x = 8,
	BitMat3x = 16,
	BitMat4x = 24,
	ShiftType = 0,
	MaskType = 1,
	NotMaskType = static_cast<uint32_t>(~MaskType),
	ShiftVec = 1,
	MaskVec = (3 << ShiftVec),
	NotMaskVec = static_cast<uint32_t>(~MaskVec),
	ShiftCols = 3,
	MaskCols = (3 << ShiftCols),
	NotMaskCols = static_cast<uint32_t>(~MaskCols)
};

/// <summary>Operator| for a set of bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>The bits after applying the operator</returns>
inline Bits operator|(Bits lhs, Bits rhs)
{
	return static_cast<Bits>(static_cast<std::underlying_type<Bits>::type /**/>(lhs) | static_cast<std::underlying_type<Bits>::type /**/>(rhs));
}

/// <summary>Operator|= for a set of bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
inline void operator|=(Bits& lhs, Bits rhs)
{
	lhs = static_cast<Bits>(static_cast<std::underlying_type<Bits>::type /**/>(lhs) | static_cast<std::underlying_type<Bits>::type /**/>(rhs));
}

/// <summary>Operator& for a set of bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>The bits after applying the operator</returns>
inline Bits operator&(Bits lhs, Bits rhs)
{
	return static_cast<Bits>(static_cast<std::underlying_type<Bits>::type /**/>(lhs) & static_cast<std::underlying_type<Bits>::type /**/>(rhs));
}

/// <summary>Operator&= for a set of bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
inline void operator&=(Bits& lhs, Bits rhs)
{
	lhs = static_cast<Bits>(static_cast<std::underlying_type<Bits>::type /**/>(lhs) & static_cast<std::underlying_type<Bits>::type /**/>(rhs));
}
} // namespace GpuDatatypesHelper

/// <summary>A (normally hardware-supported) GPU datatype (e.g. vec4 etc.)</summary>
enum class GpuDatatypes : uint32_t
{
	Integer = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) |
		static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	uinteger = Integer,
	boolean = Integer,
	Float = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) |
		static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	ivec2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) |
		static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	uvec2 = ivec2,
	bvec2 = ivec2,
	ivec3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) |
		static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	uvec3 = ivec3,
	bvec3 = ivec3,
	ivec4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Integer) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) |
		static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	uvec4 = ivec4,
	bvec4 = ivec4,
	vec2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	vec4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitOneCol),
	mat2x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat2x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat2x),
	mat3x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat3x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat3x),
	mat4x2 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x3 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec3) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	mat4x4 = static_cast<uint32_t>(GpuDatatypesHelper::Bits::Float) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec4) | static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitMat4x),
	none = 0xFFFFFFFF,
	structure = none
};
/// <summary>Bitwise operator AND. Typical semantics. Allows AND between GpuDatatypes and Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs AND rhs</returns>
inline GpuDatatypes operator&(GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs) { return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

/// <summary>Bitwise operator RIGHT SHIFT. Typical semantics. Allows RIGHT SHIFT of GpuDatatypes by Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs RIGHT SHIFT rhs</returns>
inline GpuDatatypes operator>>(GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs) { return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) >> static_cast<uint32_t>(rhs)); }

/// <summary>Bitwise operator LEFT SHIFT. Typical semantics. Allows LEFT SHIFT of GpuDatatypes by Bits</summary>
/// <param name="lhs">Left hand side</param>
/// <param name="rhs">Right hand side</param>
/// <returns>lhs LEFT SHIFT rhs</returns>
inline GpuDatatypes operator<<(GpuDatatypes lhs, GpuDatatypesHelper::Bits rhs) { return static_cast<GpuDatatypes>(static_cast<uint32_t>(lhs) << static_cast<uint32_t>(rhs)); }

/// <summary>Get the number of colums (1..4) of the type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The number of matrix colums (1..4) of the type. 1 implies not a matrix</returns>
inline uint32_t getNumMatrixColumns(GpuDatatypes type)
{
	return static_cast<uint32_t>(GpuDatatypesHelper::MatrixColumns(static_cast<uint32_t>((type & GpuDatatypesHelper::Bits::MaskCols) >> GpuDatatypesHelper::Bits::ShiftCols) + 1));
}

/// <summary>Get required alignment of this type as demanded by std140 rules</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The required alignment of the type based on std140 (see the GLSL spec)</returns>
inline uint32_t getAlignment(GpuDatatypes type)
{
	uint32_t vectype = static_cast<uint32_t>(type & GpuDatatypesHelper::Bits::MaskVec);
	return (vectype == static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitScalar) ? 4u : vectype == static_cast<uint32_t>(GpuDatatypesHelper::Bits::BitVec2) ? 8u : 16u);
}

/// <summary>Get the size of a type, including padding, assuming the next item is of the same type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size plus padding of this type</returns>
inline uint32_t getVectorSelfAlignedSize(GpuDatatypes type) { return getAlignment(type); }

/// <summary>Get the number of vector elements (i.e. Rows) of a type. (e.g. vec2=>2)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The number of vector elements.</returns>
inline uint32_t getNumVecElements(GpuDatatypes type)
{
	return static_cast<uint32_t>(GpuDatatypesHelper::VectorWidth(static_cast<uint32_t>((type & GpuDatatypesHelper::Bits::MaskVec) >> GpuDatatypesHelper::Bits::ShiftVec) + 1));
}

/// <summary>Get the cpu-packed size of each vector element a type (disregarding matrix columns if they exist)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size that a single column of <paramRef name="type"/> would take on the CPU</returns>
inline uint32_t getVectorUnalignedSize(GpuDatatypes type) { return 4 * getNumVecElements(type); }

/// <summary>Get the underlying element of a type (integer or float)</summary>
/// <param name="type">The datatype to test</param>
/// <returns>A BaseType enum (integer or float)</returns>
inline GpuDatatypesHelper::BaseType getBaseType(GpuDatatypes type) { return GpuDatatypesHelper::BaseType(static_cast<uint32_t>(type) & 1); }

/// <summary>Returns a datatype that is larger or equal to both of two types:
/// 1) Has the most permissive base type (float>int)
/// 2) Has the largest of the two vector widths
/// 3) Has the most of the two matrix colums heights</summary>
/// <param name="type1">The first type</param>
/// <param name="type2">The second type</param>
/// <returns>A type that can fit either of type1 or type1</returns>
inline GpuDatatypes mergeDatatypesBigger(GpuDatatypes type1, GpuDatatypes type2)
{
	uint32_t baseTypeBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskType, type2 & GpuDatatypesHelper::Bits::MaskType));
	uint32_t vectorWidthBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskVec, type2 & GpuDatatypesHelper::Bits::MaskVec));
	uint32_t matrixColBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskCols, type2 & GpuDatatypesHelper::Bits::MaskCols));
	return GpuDatatypes(baseTypeBits | vectorWidthBits | matrixColBits);
}

/// <summary>Returns a datatype that is smaller or equal to both of two types:
/// 1) Has the most permissive base type (float>int)
/// 2) Has the smaller of the two vector widths
/// 3) Has the least of the two matrix colums heights</summary>
/// <param name="type1">The first type</param>
/// <param name="type2">The second type</param>
/// <returns>A type that will truncate everything the two types don't share</returns>
inline GpuDatatypes mergeDatatypesSmaller(GpuDatatypes type1, GpuDatatypes type2)
{
	uint32_t baseTypeBits = static_cast<uint32_t>((::std::max)(type1 & GpuDatatypesHelper::Bits::MaskType, type2 & GpuDatatypesHelper::Bits::MaskType));
	uint32_t vectorWidthBits = static_cast<uint32_t>((::std::min)(type1 & GpuDatatypesHelper::Bits::MaskVec, type2 & GpuDatatypesHelper::Bits::MaskVec));
	uint32_t matrixColBits = static_cast<uint32_t>((::std::min)(type1 & GpuDatatypesHelper::Bits::MaskCols, type2 & GpuDatatypesHelper::Bits::MaskCols));
	return GpuDatatypes(baseTypeBits | vectorWidthBits | matrixColBits);
}

/// <summary>Returns "how many bytes will an object of this type take", if not an array.</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size of this type, aligned to its own alignment restrictions</returns>
inline uint32_t getSelfAlignedSize(GpuDatatypes type)
{
	uint32_t isMatrix = (getNumMatrixColumns(type) > 1);

	return (::std::max)(getVectorSelfAlignedSize(type), static_cast<uint32_t>(16) * isMatrix) * getNumMatrixColumns(type);
}

/// <summary>Returns "how many bytes will an object of this type take", if it is an array member (arrays have potentially stricter requirements).</summary>
/// <param name="type">The datatype to test</param>
/// <returns>The size of this type, aligned to max array alignment restrictions</returns>
inline uint32_t getSelfAlignedArraySize(GpuDatatypes type) { return (::std::max)(getVectorSelfAlignedSize(type), static_cast<uint32_t>(16)) * getNumMatrixColumns(type); }

/// <summary>Returns how many bytes an array of n objects of this type take, but arrayElements = 1
/// is NOT considered an array (is aligned as a single object, NOT an array of 1)</summary>
/// <param name="type">The datatype to test</param>
/// <param name="arrayElements">The number of array elements. 1 is NOT considered an array.</param>
/// <returns>The size of X elements takes</returns>
inline uint64_t getSize(GpuDatatypes type, uint32_t arrayElements = 1)
{
	uint64_t numElements = getNumMatrixColumns(type) * arrayElements;

	uint64_t selfAlign = (::std::max)(static_cast<uint64_t>(getVectorSelfAlignedSize(type)), static_cast<uint64_t>(16)) * numElements *
		(numElements > 1); // Equivalent to: if (arrayElements!=0) selfAlign = getVectorSelfAlignedSize(...) else selfAlign=0;
	uint64_t unaligned = getVectorUnalignedSize(type) * (numElements == 1); // Equivalent to: if (arrayElements==0) unaligned = getVectorUnalignedSize(...) else unaligned = 0;
	return selfAlign + unaligned;

	// *** THIS IS A BRANCHLESS EQUIVALENT OF THE FOLLOWING CODE ***
	// if (numElements > 1)
	//{
	// return (std::max)(getVectorSelfAlignedSize(type), 16u) * numElements;
	//}
	// else
	//{
	// return getVectorUnalignedSize(type);
	//}
}

/// <summary>Get a string with the glsl variable name of a type</summary>
/// <param name="type">The datatype to test</param>
/// <returns>A c-style string with the glsl variable keyword of <paramRef name="type"/></returns>
inline const char* toString(GpuDatatypes type)
{
	switch (type)
	{
	case GpuDatatypes::Integer: return "int";
	case GpuDatatypes::ivec2: return "ivec2";
	case GpuDatatypes::ivec3: return "ivec3";
	case GpuDatatypes::ivec4: return "ivec4";
	case GpuDatatypes::Float: return "float";
	case GpuDatatypes::vec2: return "vec2";
	case GpuDatatypes::vec3: return "vec3";
	case GpuDatatypes::vec4: return "vec4";
	case GpuDatatypes::mat2x2: return "mat2x2";
	case GpuDatatypes::mat2x3: return "mat2x3";
	case GpuDatatypes::mat2x4: return "mat2x4";
	case GpuDatatypes::mat3x2: return "mat3x2";
	case GpuDatatypes::mat3x3: return "mat3x3";
	case GpuDatatypes::mat3x4: return "mat3x4";
	case GpuDatatypes::mat4x2: return "mat4x2";
	case GpuDatatypes::mat4x3: return "mat4x3";
	case GpuDatatypes::mat4x4: return "mat4x4";
	case GpuDatatypes::none: return "NONE";
	default: return "UNKNOWN";
	}
}

/// <summary>Get the size of n array members of a type, packed in CPU</summary>
/// <param name="type">The datatype to test</param>
/// <param name="arrayElements">The number of array elements</param>
/// <returns>The base size of the type multiplied by arrayElements</returns>
inline uint64_t getCpuPackedSize(GpuDatatypes type, uint32_t arrayElements = 1) { return getVectorUnalignedSize(type) * getNumMatrixColumns(type) * arrayElements; }

/// <summary>Aligns an address/offset with the alignment of a type -- equivalently,
/// assuming you want to place a type after a known offset (i.e. calculating the
/// offset of an item inside a struct having already calculated its previous element)
/// (i.e. aligning a vec4 after an item that ends at 30 bytes returns 32 bytes...)</summary>
/// <param name="type">The datatype to test</param>
/// <param name="previousTotalSize">The address/offset to align for that type</param>
/// <returns><paramRef name="previousTotalSize"/> aligned to the requirements of
/// <paramRef name="type"/></returns>
inline uint32_t getOffsetAfter(GpuDatatypes type, uint64_t previousTotalSize)
{
	uint32_t align = getAlignment(type);

	uint32_t diff = previousTotalSize % align;
	diff += (align * (diff == 0)); // REMOVE BRANCHING -- equal to : if(diff==0) { diff+=8 }
	return static_cast<uint32_t>(previousTotalSize) - diff + align;
}

/// <summary>Returns the new size of a hypothetical struct whose old size was previousTotalSize,
/// and to which "arrayElement" new items of type "type" are added</summary>
/// <param name="type">The datatype to add</param>
/// <param name="arrayElements">The number of items of type <paramRef name="type"/> to add</param>
/// <param name="previousTotalSize">The address/offset to align for that type</param>
/// <returns>The new size</returns>
inline uint64_t getTotalSizeAfter(GpuDatatypes type, uint32_t arrayElements, uint64_t previousTotalSize)
{
	// Arrays pads their last element to their alignments. Standalone objects do not. vec3[3] is NOT the same as vec3;vec3;vec3;
	uint64_t selfAlignedSize = getSelfAlignedArraySize(type) * arrayElements * (arrayElements != 1); // Equivalent to: if (arrayElements!=1) selfAlign = getSelfAl... else selfAlign=0
	// Other elements do not.
	uint64_t unalignedSize = getSize(type) * (arrayElements == 1); // Equivalent to: if (arrayElements==1) unaligned = getUnaligned.... else unaligned=0

	return getOffsetAfter(type, previousTotalSize) + selfAlignedSize + unalignedSize;
}

/// <summary>Get the Cpu Datatype <paramRef name="type"/> refers to (i.e. which CPU datatype must you
/// load in the data you upload to the GPU to correctly upload the same value  in the shader).</summary>
/// <param name="type">The type to convert</param>
/// <returns>A CPU type that has the same bit representation as one scalar element of type (i.e. mat4x4 returns "float")</returns>
inline DataType toDataType(GpuDatatypes type) { return getBaseType(type) == GpuDatatypesHelper::BaseType::Float ? DataType::Float32 : DataType::Int32; }

namespace GpuDatatypesHelper {
template<typename T>
struct Metadata; // Template specializations in FreeValue.h
}
} // namespace pvr
