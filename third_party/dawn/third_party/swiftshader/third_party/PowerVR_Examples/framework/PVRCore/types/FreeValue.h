/*!
\brief Two classes designed to carry values of arbitrary datatypes along with their "reflective" data (datatypes
etc.) FreeValue is statically allocated but has a fixed (max) size of 64 bytes, while TypedMem stores arbitrary
sized data.
\file PVRCore/types/FreeValue.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/GpuDataTypes.h"
#include "PVRCore/Log.h"
#include "PVRCore/math/MathUtils.h"
namespace pvr {
namespace GpuDatatypesHelper {

/// <summary>Metadata for mapping GpuDatatypes to actual types. Specializations provide the following members: storagetype, dataType, gpuSize, .</summary>
template<typename>
struct Metadata;

//!\cond NO_DOXYGEN
template<>
struct Metadata<char*>
{
	typedef std::array<char, 64> storagetype;
	static const GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static const size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<unsigned char*>
{
	typedef std::array<char, 64> storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<const char*>
{
	typedef std::array<char, 64> storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<const unsigned char*>
{
	typedef std::array<char, 64> storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<double>
{
	typedef float storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<float>
{
	typedef float storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Float; }
	static size_t gpuSizeOf() { return 4; }
};
template<>
struct Metadata<int64_t>
{
	typedef int32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Integer; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<int32_t>
{
	typedef int32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Integer; }
	static size_t gpuSizeOf() { return 4; }
};
template<>
struct Metadata<int16_t>
{
	typedef int32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Integer; }
	static size_t gpuSizeOf() { return 2; }
};
template<>
struct Metadata<int8_t>
{
	typedef int32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::Integer; }
	static size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<uint64_t>
{
	typedef uint32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uinteger; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<uint32_t>
{
	typedef uint32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uinteger; }
	static size_t gpuSizeOf() { return 4; }
};
template<>
struct Metadata<uint16_t>
{
	typedef uint32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uinteger; }
	static size_t gpuSizeOf() { return 2; }
};
template<>
struct Metadata<uint8_t>
{
	typedef uint32_t storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uinteger; }
	static size_t gpuSizeOf() { return 1; }
};
template<>
struct Metadata<glm::vec2>
{
	typedef glm::vec2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::vec2; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<glm::vec3>
{
	typedef glm::vec3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::vec3; }
	static size_t gpuSizeOf() { return 12; }
};
template<>
struct Metadata<glm::vec4>
{
	typedef glm::vec4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::vec4; }
	static size_t gpuSizeOf() { return 16; }
};
template<>
struct Metadata<glm::ivec2>
{
	typedef glm::ivec2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::ivec2; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<glm::ivec3>
{
	typedef glm::ivec3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::ivec3; }
	static size_t gpuSizeOf() { return 12; }
};
template<>
struct Metadata<glm::ivec4>
{
	typedef glm::ivec4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::ivec4; }
	static size_t gpuSizeOf() { return 16; }
};
template<>
struct Metadata<glm::uvec2>
{
	typedef glm::uvec2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uvec2; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<glm::uvec3>
{
	typedef glm::uvec3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uvec3; }
	static size_t gpuSizeOf() { return 12; }
};
template<>
struct Metadata<glm::uvec4>
{
	typedef glm::uvec4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::uvec4; }
	static size_t gpuSizeOf() { return 16; }
};
template<>
struct Metadata<glm::bvec2>
{
	typedef glm::bvec2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::bvec2; }
	static size_t gpuSizeOf() { return 8; }
};
template<>
struct Metadata<glm::bvec3>
{
	typedef glm::bvec3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::bvec3; }
	static size_t gpuSizeOf() { return 12; }
};
template<>
struct Metadata<glm::bvec4>
{
	typedef glm::bvec4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::bvec4; }
	static size_t gpuSizeOf() { return 16; }
};
template<>
struct Metadata<glm::mat2x2>
{
	typedef glm::mat2x2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat2x2; }
	static size_t gpuSizeOf() { return 32; }
};
template<>
struct Metadata<glm::mat2x3>
{
	typedef glm::mat2x3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat2x3; }
	static size_t gpuSizeOf() { return 32; }
};
template<>
struct Metadata<glm::mat2x4>
{
	typedef glm::mat2x4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat2x4; }
	static size_t gpuSizeOf() { return 32; }
};
template<>
struct Metadata<glm::mat3x2>
{
	typedef glm::mat3x2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat3x2; }
	static size_t gpuSizeOf() { return 48; }
};
template<>
struct Metadata<glm::mat3x3>
{
	typedef glm::mat3x3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat3x3; }
	static size_t gpuSizeOf() { return 48; }
};
template<>
struct Metadata<glm::mat3x4>
{
	typedef glm::mat3x4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat3x4; }
	static size_t gpuSizeOf() { return 48; }
};
template<>
struct Metadata<glm::mat4x2>
{
	typedef glm::mat4x2 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat4x2; }
	static size_t gpuSizeOf() { return 64; }
};
template<>
struct Metadata<glm::mat4x3>
{
	typedef glm::mat4x3 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat4x3; }
	static size_t gpuSizeOf() { return 64; }
};
template<>
struct Metadata<glm::mat4x4>
{
	typedef glm::mat4x4 storagetype;
	static GpuDatatypes dataTypeOf() { return GpuDatatypes::mat4x4; }
	static size_t gpuSizeOf() { return 64; }
};
//!\endcond
} // namespace GpuDatatypesHelper

/// <summary>Represents a runtime-known number of elements of a runtime-known type, with functions to handle and convert them.
/// Very commonly used to dynamically represent data that will eventually need to be used by the GPU, such as uniforms, vectors
/// and matrices. Does not contain methods to actually be populated, as this should be done through subclasses.</summary>
class FreeValueView
{
protected:
	unsigned char* value_; //!< Dynamically allocated, data is carried here.
	uint32_t arrayElements_; //!< Number of array elements
	GpuDatatypes dataType_; //!< Semantic data type
public:
	/// <summary>Constructor. Constructs and empty value.</summary>
	FreeValueView() : value_(0), arrayElements_(0), dataType_(GpuDatatypes::none) {}
	/// <summary>Get the datatype of the value contained. "None" means unformatted data, in which case ArraySize represents
	/// the number of bytes</summary>
	/// <returns>The datatype of this value.</returns>
	GpuDatatypes dataType() const { return dataType_; }

	/// <summary>Check if two free values only differ in value (i.e. they are of the same type and elements).</summary>
	/// <param name="rhs">The value against which to compare</param>
	/// <returns>True if datatype and arrayelements are the same between this and rhs, otherwise false.</returns>
	bool isDataCompatible(const FreeValueView& rhs) const { return (dataType_ == rhs.dataType_) && (arrayElements_ == rhs.arrayElements_); }

	/// <summary>Size of the data contained.</summary>
	/// <returns>The size of data contained.</returns>
	uint64_t dataSize() const
	{
		uint32_t isnone = (dataType_ == GpuDatatypes::none);
		return isnone ? arrayElements_ : getCpuPackedSize(dataType_, arrayElements_);
	}

	/// <summary>Size of the data contained. For unformatted data (Datatype==None) this is the total bytes contained.</summary>
	/// <returns>The size of data contained.</returns>
	uint32_t arrayElements() const { return arrayElements_; }

	/// <summary>Get a pointer to the raw data value at the specified index.</summary>
	/// <param name="arrayIndex">The array index of which to take the pointer</param>
	/// <returns>Pointer to the raw value.</returns>
	void* raw(uint32_t arrayIndex)
	{
		size_t offset = (size_t)(arrayIndex * getCpuPackedSize(dataType_));
		return value_ + offset;
	}

	/// <summary>Get a pointer to the raw data value at the specified index.</summary>
	/// <param name="arrayIndex">The array index of which to take the pointer</param>
	/// <returns>Pointer to the raw value.</returns>
	const void* raw(uint32_t arrayIndex) const
	{
		size_t offset = (size_t)(arrayIndex * getCpuPackedSize(dataType_));
		return value_ + offset;
	}

	/// <summary>Get a pointer to the raw data value.</summary>
	/// <returns>Pointer to the raw value.</returns>
	void* raw() { return value_; }
	/// <summary>Get a pointer to the raw data value.</summary>
	/// <returns>Pointer to the raw value.</returns>
	const void* raw() const { return value_; }

	/// <summary>Get a pointer to the raw data value as a specified type</summary>
	/// <typeparam name="Type_"> The type as which the data are interpreted</typeparam>
	/// <returns>Pointer to the raw value.</returns>
	template<typename Type_>
	Type_* rawAs()
	{
		return reinterpret_cast<Type_*>(value_);
	}

	/// <summary>Get a pointer to the data value as a specified type</summary>
	/// <typeparam name="Type_"> The type as which the data are interpreted</typeparam>
	/// <returns>Pointer to the raw value.</returns>
	template<typename Type_>
	const Type_* rawAs() const
	{
		return reinterpret_cast<Type_*>(value_);
	}

	/// <summary>Get a pointer to the data value as (an array of) 32 bit floating point numbers</summary>
	/// <returns>Pointer to the data as float*.</returns>
	float* rawFloats() { return reinterpret_cast<float*>(value_); }
	/// <summary>Get a pointer to the data value as (an array of) 32 bit floating point numbers</summary>
	/// <returns>Pointer to the data as float*.</returns>
	const float* rawFloats() const { return reinterpret_cast<const float*>(value_); }
	/// <summary>Get a pointer to the data value as (an array of) 32 bit integers</summary>
	/// <returns>Pointer to the data as int*.</returns>
	int32_t* rawInts() { return reinterpret_cast<int32_t*>(value_); }
	/// <summary>Get a pointer to the data value as (an array of) 32 bit integers</summary>
	/// <returns>Pointer to the data as int*.</returns>
	const int32_t* rawInts() const { return reinterpret_cast<const int32_t*>(value_); }
	/// <summary>Get a pointer to the data value as (an array of) unsigned chars</summary>
	/// <returns>Pointer to the data as unsigned char*.</returns>
	unsigned char* rawChars() { return value_; }
	/// <summary>Get a pointer to the data value as (an array of) unsigned chars</summary>
	/// <returns>Pointer to the data as unsigned char*.</returns>
	const unsigned char* rawChars() const { return value_; }

	/// <summary>Interpret the value as (an array of) a specified type and retrieve the item at a specific position.</summary>
	/// <typeparam name="Type_">The type as which the value is interpreted</typeparam>
	/// <param name="entryIndex">If an array, which item to retrieve (default 0)</typeparam>
	/// <returns>A value of type <paramRef name="Type_"/> at index <paramRef name="entryIndex"/></returns>
	template<typename Type_>
	Type_& interpretValueAs(uint32_t entryIndex = 0)
	{
		return reinterpret_cast<Type_*>(value_)[entryIndex];
	}

	/// <summary>Interpret the value as (an array of) a specified type and retrieve the item at a specific position.</summary>
	/// <typeparam name="Type_">The type as which the value is interpreted</typeparam>
	/// <param name="entryIndex">If an array, which item to retrieve (default 0)</typeparam>
	/// <returns>A value of type <paramRef name="Type_"/> at index <paramRef name="entryIndex"/></returns>
	template<typename Type_>
	const Type_& interpretValueAs(uint32_t entryIndex = 0) const
	{
		return reinterpret_cast<const Type_*>(value_)[entryIndex];
	}
};

/// <summary>Represents Free Value View that is backed by a (usually small) dynamically allocated block.</summary>
struct TypedMem : public FreeValueView
{
private:
	uint64_t currentSize_;

public:
	/// <summary>Constructor.</summary>
	TypedMem() : currentSize_(0) {}
	/// <summary>Destructor. Frees any allocated memory.</summary>
	~TypedMem() { free(FreeValueView::value_); }

	/// <summary>Copy constructor from another TypedMem.</summary>
	/// <param name="rhs">The object to copy from</param>
	void assign(const TypedMem& rhs)
	{
		uint64_t dataTypeSize = rhs.dataSize();
		allocate(rhs.dataType_, rhs.arrayElements_);
		memcpy(value_, rhs.value_, (size_t)dataTypeSize);
	}

	/// <summary>Copy assignment from another TypedMem.</summary>
	/// <param name="rhs">The object to copy from</param>
	/// <returns>This object</returns>
	const TypedMem& operator=(const TypedMem& rhs) const
	{
		uint64_t dataTypeSize = rhs.dataSize();
		debug_assertion(dataTypeSize <= dataSize(), "TypedMem operator=: MISMATCHED SIZE");
		memcpy(value_, rhs.value_, (size_t)dataTypeSize);
		return *this;
	}

	/// <summary>Copy assignment from another TypedMem.</summary>
	/// <param name="rhs">The object to copy from</param>
	/// <returns>This object</returns>
	TypedMem(const TypedMem& rhs) : currentSize_(0)
	{
		uint64_t dataTypeSize = rhs.dataSize();
		allocate(rhs.dataType_, rhs.arrayElements_);
		memcpy(value_, rhs.value_, (size_t)dataTypeSize);
	}

	/// <summary>Return the size of this value in bytes.</summary>
	/// <returns>The size of this value in bytes</returns>
	uint64_t totalSize() const { return currentSize_; }

	/// <summary>Set this TypedMen to the specified number of items, reallocating as needed.
	/// Use this method instead of clear() if you actually need to physically free the memory.</summary>
	/// <param name="arrayElements">Number of elemets</param>
	void shrink(uint32_t arrayElements)
	{
		uint64_t dataTypeSize = (dataType_ == GpuDatatypes::none ? arrayElements : getCpuPackedSize(dataType_) * arrayElements);
		arrayElements_ = arrayElements;
		if (!arrayElements)
		{
			free(value_);
			value_ = 0;
		}
		else if (dataTypeSize != currentSize_)
		{
			value_ = static_cast<unsigned char*>(realloc(value_, (size_t)dataTypeSize));
		}
		currentSize_ = dataTypeSize;
	}

	/// <summary>Empty this object. Does NOT free the memory.</summary>
	void clear()
	{
		dataType_ = GpuDatatypes::none;
		arrayElements_ = 0;
	}

	/// <summary>Allocate the TypedMem to contain the specified type and number of items. Grows only, does not shrink memory.</summary>
	/// <param name="dataType">The type of the data. Use "none" for raw bytes</param>
	/// <param name="arrayElements">The number of array elements.</param>
	void allocate(GpuDatatypes dataType, uint32_t arrayElements = 1)
	{
		uint32_t isnone = (dataType == GpuDatatypes::none);
		uint64_t dataTypeSize = isnone * arrayElements + (1 - isnone) * getCpuPackedSize(dataType, arrayElements);

		dataType_ = dataType;
		arrayElements_ = arrayElements;
		if (dataTypeSize > currentSize_)
		{
			value_ = static_cast<unsigned char*>(realloc(value_, (size_t)dataTypeSize));
			currentSize_ = dataTypeSize;
		}
	}

	/// <summary>Set this object to contain the specified item, allocating as needed.
	/// Sets the type to the type of rawvalue, and array elements to 1.</summary>
	/// <param name="rawvalue">The value to set</param>
	template<typename Type_>
	void setValue(const Type_& rawvalue)
	{
		allocate(GpuDatatypesHelper::Metadata<Type_>::dataTypeOf(), 1);
		memcpy(value_, &rawvalue, sizeof(Type_));
	}

	/// <summary>**Before calling this function, allocate the required number of items
	/// of the type you need, as this function does NOT allocate**
	/// Sets this object to contain the specified item at the specified index. Does not
	/// allocate, so if the type is wrong or the array index is wrong, the behaviour is
	/// undefined.</summary>
	/// <param name="rawvalue">The value to set</param>
	/// <param name="arrayIndex">The index where to set the value</param>
	template<typename Type_>
	void setValue(const Type_& rawvalue, uint32_t arrayIndex)
	{
		assertion(arrayElements_ > arrayIndex,
			"TypedMemory:: If array values "
			"are used with this class, they must be pre-allocated");
		memcpy(value_ + (arrayIndex * sizeof(Type_)), &rawvalue, sizeof(Type_));
	}

	/// <summary>**Before calling this function, allocate the required number of items
	/// of the type you need, as this function does NOT allocate**
	/// Sets the memory this object to contain the specified item at the specified index. Does not
	/// allocate, so if the type is wrong or the array index is wrong, the behaviour is
	/// undefined.</summary>
	/// <param name="rawvalues">The values to set</param>
	/// <param name="numElements">The number of elmeents to set</param>
	/// <param name="startArrayIndex">The array index at which to start setting values for</param>
	template<typename Type_>
	void setValues(const Type_* rawvalues, uint32_t numElements, uint32_t startArrayIndex = 0)
	{
		assertion(numElements > startArrayIndex, "TypedMemory:: If array values are used with this class, they must be pre-allocated");
		uint32_t elementsSize = static_cast<uint32_t>(startArrayIndex * sizeof(Type_) + sizeof(Type_) * numElements);
		assertion(currentSize_ >= elementsSize, "TypedMemory:: If array values are used with this class, they must be pre-allocated");
		memcpy(value_ + (startArrayIndex * sizeof(Type_)), &rawvalues, sizeof(Type_) * numElements);
	}

	/// <summary>Sets the value from a null-terminated c-style string. Allocates as
	/// necessary.</summary>
	/// <param name="c_string_value">A null-terminated c-string</param>
	void setValue(const char* c_string_value)
	{
		this->dataType_ = GpuDatatypes::none;
		uint32_t sz = static_cast<uint32_t>(strlen(c_string_value));
		allocate(GpuDatatypes::none, sz + 1);
		memcpy(value_, c_string_value, sz);
		value_[sz] = 0;
	}
	/// <summary>Sets the value from a c++ string. Allocates as
	/// necessary.</summary>
	/// <param name="rawvalue">A c++ string</param>
	void setValue(const std::string& rawvalue) { setValue(rawvalue.c_str()); }
};

/// <summary>A Free Value View that is backed by a 64-byte statically allocated array: Enough to hold
/// one item of any GPU-datatype, up to a mat4x4</summary>
struct FreeValue : public FreeValueView
{
private:
	union
	{
		double _alignment_[8];
		unsigned char chars_[64];
		int32_t integer32_[16];
		float float32_[16];
		int16_t int16_[16];
		int64_t int64_[8];
	};

public:
	/// <summary>Constructor. Non-initializing (undefined value)</summary>
	FreeValue()
	{
		value_ = chars_;
		arrayElements_ = 1;
	}

	/// <summary>Constructor. Set to the value of the passed object.</summary>
	/// <typeparam name="Type_">The type of the value. Used to interpret the object
	/// and also set the datatype.</typeparam>
	/// <param name="rawvalue">Copy this value</param>
	template<typename Type_>
	FreeValue(const Type_& rawvalue) : FreeValue()
	{
		setValue(rawvalue);
	}

	/// <summary>Define the datatype of this FreeValue</summary>
	/// <param name="datatype">The datatype to define this FreeValue.</param>
	void setDataType(GpuDatatypes datatype) { dataType_ = datatype; }

	/// <summary>Set the value of this object</summary>
	/// <typeparam name="Type_">The type of the value. Used to interpret the object
	/// and alos set the datatype.</typeparam>
	/// <param name="rawvalue">Copy this value</param>
	template<typename Type_>
	void setValue(const Type_& rawvalue)
	{
		this->dataType_ = GpuDatatypesHelper::Metadata<Type_>::dataTypeOf();

		memcpy(this->chars_, &rawvalue, sizeof(Type_));
	}

	/// <summary>Set the value of this object from any FreeValue object</summary>
	/// <param name="other">Copy this item's value. If an array, copy the first item only.</param>
	void setValue(FreeValueView& other)
	{
		dataType_ = other.dataType();

		memcpy(chars_, other.raw(), (size_t)std::min(other.dataSize(), static_cast<uint64_t>(sizeof(chars_))));
	}

	/// <summary>Set the value of this object from a null-terminated c-style string. Truncates to 64 bytes.</summary>
	/// <param name="c_string_value">The value to copy</param>
	void setValue(const char* c_string_value)
	{
		this->dataType_ = GpuDatatypes::none;
		size_t sz = strlen(c_string_value);

		memcpy(chars_, c_string_value, (size_t)std::min(sz, sizeof(chars_)));
		chars_[63] = 0;
	}

	/// <summary>Set the value of this object from a c++ string. Truncates to 64 bytes.</summary>
	/// <param name="rawvalue">The value to copy</param>
	void setValue(const std::string& rawvalue)
	{
		this->dataType_ = GpuDatatypes::none;

		memcpy(chars_, rawvalue.c_str(), std::min(sizeof(chars_), (size_t)rawvalue.length()));
		chars_[63] = 0;
	}

	/// <summary>Copy a value from a given untyped pointer. The type of the value is ONLY used for the size to copy -
	/// no conversion is performed - only a bit-for-bit copy. If the value is incompatible, the behaviour is undefined/</summary>
	/// <param name="type">The type of the value to copy.</param>
	/// <param name="value">The value to copy.</param>
	void fastSet(GpuDatatypes type, char* value)
	{
		this->dataType_ = type;
		memcpy(this->chars_, value, (size_t)std::min(static_cast<uint64_t>(64), getSize(type)));
	}

	/// <summary>Read the value as if it was of the specified type</summary>
	/// <typeparam name="Type_">Interpret the value as an object of this type and return a reference to it</typeparam>
	/// <returns> A reference to the object, interpreted as of the specified Type</returns>
	template<typename Type_>
	Type_& interpretValueAs()
	{
		return *reinterpret_cast<Type_*>(chars_);
	}

	/// <summary>Read the value as if it was of the specified type</summary>
	/// <typeparam name="Type_">Interpret the value as an object of this type and return a reference to it</typeparam>
	/// <returns> A reference to the object, interpreted as of the specified Type</returns>
	template<typename Type_>
	const Type_& interpretValueAs() const
	{
		return *reinterpret_cast<const Type_*>(chars_);
	}

	/// <summary>Assuming the value is a scalar, cast it to the specified type and return it. If the type parameter
	/// is different than the actual parameter contained, will perform a normal type conversion before returning
	/// the value. Can only be used if the type contained is not a vector or matrix, otherwise logs error and returns
	/// an empty value</summary>
	/// <typeparam name="Type_">The type of the return value. If different than the exact type, a simple conversion is done.</typeparam>
	/// <returns> A reference to the object, interpreted as of the specified Type. If the datatype is not scaler, returns
	/// (Type_)0</returns>
	template<typename Type_>
	Type_ castValueScalarToScalar() const
	{
		switch (dataType_)
		{
		case GpuDatatypes::Float: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<float>::storagetype>());
		case GpuDatatypes::Integer: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<int32_t>::storagetype>());
		default: Log("FreeValue: Tried to interpret matrix, std::string or vector value as scalar."); return Type_();
		}
	}

	/// <summary>Assuming the contained value is a vector, cast it to the specified type and return it. If the type parameter
	/// is different than the actual parameter contained, will perform a normal type conversion before returning
	/// the value. Can only be used if the type contained is not a scalar or matrix, otherwise logs error and returns
	/// an empty value.</summary>
	/// <typeparam name="Type_">The type of the return value. If different than the exact type, a simple conversion is done.</typeparam>
	/// <returns> A reference to the object, interpreted as of the specified Type. If the datatype is not vector, returns
	/// (Type_)0</returns>
	template<typename Type_>
	Type_ castValueVectorToVector() const
	{
		switch (dataType_)
		{
		case GpuDatatypes::vec2: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::vec2>::storagetype>());
		case GpuDatatypes::vec3: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::vec3>::storagetype>());
		case GpuDatatypes::vec4: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::vec4>::storagetype>());
		case GpuDatatypes::ivec2: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::ivec2>::storagetype>());
		case GpuDatatypes::ivec3: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::ivec3>::storagetype>());
		case GpuDatatypes::ivec4: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::ivec4>::storagetype>());
		default: Log("FreeValue: Tried to interpret matrix, std::string or scalar value as vector."); return Type_();
		}
	}

	/// <summary>Assuming the contained value is a matrix, cast it to the specified type and return it. If the type parameter
	/// is different than the actual parameter contained, will perform a normal type conversion before returning
	/// the value. Can only be used if the type contained is not a scalar or vector, otherwise logs error and returns
	/// an empty value.</summary>
	/// <typeparam name="Type_">The type of the return value. If different than the exact type, a simple conversion is done.</typeparam>
	/// <returns> A reference to the object, interpreted as of the specified Type. If the datatype is not matrix, returns
	/// (Type_)0</returns>
	template<typename Type_>
	Type_ castValueMatrixToMatrix() const
	{
		switch (dataType_)
		{
		case GpuDatatypes::mat2x2: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat2x2>::storagetype>());
		case GpuDatatypes::mat2x3: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat2x3>::storagetype>());
		case GpuDatatypes::mat2x4: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat2x4>::storagetype>());
		case GpuDatatypes::mat3x2: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat3x2>::storagetype>());
		case GpuDatatypes::mat3x3: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat3x3>::storagetype>());
		case GpuDatatypes::mat3x4: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat3x4>::storagetype>());
		case GpuDatatypes::mat4x2: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat4x2>::storagetype>());
		case GpuDatatypes::mat4x3: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat4x3>::storagetype>());
		case GpuDatatypes::mat4x4: return Type_(interpretValueAs<GpuDatatypesHelper::Metadata<glm::mat4x4>::storagetype>());
		default: Log("FreeValue: Tried to interpret vector, std::string or scalar value as matrix."); return Type_();
		}
	}

	/// <summary>Assuming the contained value is a string, return it, otherwise logs error and returns an empty string.</summary>
	/// <returns> The contained string. If the datatype is not a string, returns ""</returns>
	const char* getValueAsString() const
	{
		switch (dataType_)
		{
		case GpuDatatypes::none: return reinterpret_cast<const char*>(chars_);
		default: Log("FreeValue: Tried to interpret vector, matrix or scalar value as std::string."); return "";
		}
	}
};
} // namespace pvr
