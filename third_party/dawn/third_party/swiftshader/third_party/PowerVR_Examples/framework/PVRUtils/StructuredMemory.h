/*!
\brief Contains a utility class which allows flexible and easy access and setting of memory that would usually be accessed as raw data.
\file PVRUtils/StructuredMemory.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/types/GpuDataTypes.h"
#include "PVRCore/types/FreeValue.h"
#include "PVRCore/strings/StringHash.h"
#include <sstream>
#include <iomanip>

/// <summary>Main PowerVR Namespace</summary>
namespace pvr {

/// <summary>Main namespace for the PVRUtils Library
namespace utils {
//!\cond NO_DOXYGEN
class StructuredMemoryEntry;
class StructuredBufferView;
//!\endcond

/// <summary>Defines a memory element description. The element will be provided with a name,
/// type and number of array elements. The element itself may also contain child memory elements</summary>
class StructuredMemoryDescription
{
private:
	friend class StructuredMemoryEntry;
	std::string _name;
	std::vector<StructuredMemoryDescription> _children;
	GpuDatatypes _type;
	uint32_t _numArrayElements;

public:
	/// <summary>Copy Constructor.</summary>
	/// <param name="rhs">The StructuredMemoryDescription to copy from.</param>
	StructuredMemoryDescription(const StructuredMemoryDescription& rhs) : _name(rhs._name), _children(rhs._children), _type(rhs._type), _numArrayElements(rhs._numArrayElements) {}

	/// <summary>The constructor for a StructuredMemoryDescription based on the provided name, arraySize and children</summary>
	/// <param name="name">The name of an element to add.</param>
	/// <param name="arraySize">The array size for a new element to add.</param>
	/// <param name="children">A std::initializer_list<StructuredMemoryDescription> containing the description for a set of children to add.</param>
	StructuredMemoryDescription(const std::string& name, uint32_t arraySize, std::initializer_list<StructuredMemoryDescription> children)
		: _name(name), _children(std::move(children)), _numArrayElements(arraySize), _type(GpuDatatypes::none)
	{}

	/// <summary>The constructor for a StructuredMemoryDescription based on the provided name, arraySize and children</summary>
	/// <param name="name">The name of an element to add.</param>
	/// <param name="arraySize">The array size for a new element to add.</param>
	/// <param name="children">A std::initializer_list<StructuredMemoryDescription> containing the description for a set of children to add.</param>
	StructuredMemoryDescription(std::string&& name, uint32_t arraySize, std::initializer_list<StructuredMemoryDescription> children)
		: _name(std::move(name)), _children(std::move(children)), _numArrayElements(arraySize), _type(GpuDatatypes::none)
	{}

	/// <summary>The constructor for a StructuredMemoryDescription based on the provided name and GpuDatatypes</summary>
	/// <param name="name">The name of an element to add.</param>
	/// <param name="type">The type of a new element to add.</param>
	StructuredMemoryDescription(std::string&& name, GpuDatatypes type) : _name(std::move(name)), _type(type), _numArrayElements(1) {}

	/// <summary>The constructor for a StructuredMemoryDescription based on the provided name, arraySize and GpuDatatypes</summary>
	/// <param name="str">The name of an element to add.</param>
	/// <param name="arraySize">The array size for a new element to add.</param>
	/// <param name="type">The type of a new element to add.</param>
	StructuredMemoryDescription(const std::string& str, uint32_t arraySize, pvr::GpuDatatypes type) : _name(std::move(str)), _type(type), _numArrayElements(arraySize) {}

	/// <summary>The default constructor for a StructuredMemoryDescription</summary>
	StructuredMemoryDescription() : _type(GpuDatatypes::none), _numArrayElements(1) {}

	/// <summary>Move constructor for a StructuredMemoryDescription.</summary>
	/// <param name="rhs">The StructuredMemoryDescription to move from.</param>
	StructuredMemoryDescription(StructuredMemoryDescription&& rhs)
		: _name(std::move(rhs._name)), _children(std::move(rhs._children)), _type(std::move(rhs._type)), _numArrayElements(std::move(rhs._numArrayElements))
	{}

	/// <summary>assignment operator for a StructuredMemoryDescription.</summary>
	/// <param name="rhs">The StructuredMemoryDescription to copy from.</param>
	/// <returns>Return this object</returns>
	StructuredMemoryDescription& operator=(const StructuredMemoryDescription& rhs)
	{
		_name = rhs._name;
		_children = rhs._children;
		_type = rhs._type;
		_numArrayElements = rhs._numArrayElements;
		return *this;
	}

	/// <summary>Sets the name of the StructuredMemoryDescription.</summary>
	/// <param name="name">The new name of the StructuredMemoryDescription</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& setName(const std::string& name)
	{
		_name = name;
		return *this;
	}

	/// <summary>Sets the name of the StructuredMemoryDescription.</summary>
	/// <param name="name">The new name of the StructuredMemoryDescription</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& setName(std::string&& name)
	{
		_name = std::move(name);
		return *this;
	}

	/// <summary>Sets the number of array elements of the StructuredMemoryDescription.</summary>
	/// <param name="numArrayElements">The new number of array elements for the StructuredMemoryDescription</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& setNumArrayElements(uint32_t numArrayElements)
	{
		_numArrayElements = numArrayElements;
		return *this;
	}

	/// <summary>Sets the type of the StructuredMemoryDescription.</summary>
	/// <param name="type">The new type of the StructuredMemoryDescription</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& setType(GpuDatatypes type)
	{
		_type = type;
		return *this;
	}

	/// <summary>Adds an element to the list of children for the StructuredMemoryDescription.</summary>
	/// <param name="name">The name of the child element</param>
	/// <param name="type">The type of the child element</param>
	/// <param name="numArrayElements">The number of array elements of the child element</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& addElement(const std::string& name, GpuDatatypes type, uint32_t numArrayElements = 1)
	{
		_children.emplace_back(name, numArrayElements, type);
		return *this;
	}
	/// <summary>Adds an element to the list of children for the StructuredMemoryDescription.</summary>
	/// <param name="name">The name of the child element</param>
	/// <param name="type">The type of the child element</param>
	/// <param name="numArrayElements">The number of array elements of the child element</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& addElement(std::string& name, GpuDatatypes type, uint32_t numArrayElements = 1)
	{
		_children.emplace_back(name, numArrayElements, type);
		return *this;
	}
	/// <summary>Adds an element to the list of children for the StructuredMemoryDescription.</summary>
	/// <param name="smd">A StructuredMemoryDescription defining a child to add to the list of children for the StructuredMemoryDescription</param>
	/// <returns>Returns *this.</returns>
	StructuredMemoryDescription& addElement(const StructuredMemoryDescription& smd)
	{
		_children.emplace_back(smd);
		return *this;
	}

	/// <summary>Gets an element description from the StructuredMemoryDescription.</summary>
	/// <param name="name">the name of a particular element to get the description for</param>
	/// <returns>Returns the StructuredMemoryDescription for a element with name.</returns>
	StructuredMemoryDescription getElement(const std::string& name)
	{
		for (uint32_t i = 0; i < _children.size(); i++)
		{
			if (_children[i].getName() == name) { return _children[i]; }
		}

		return StructuredMemoryDescription();
	}

	/// <summary>Gets an element description from the StructuredMemoryDescription.</summary>
	/// <param name="index">the name of a particular element to get the description for</param>
	/// <returns>Returns the StructuredMemoryDescription for a element at index.</returns>
	StructuredMemoryDescription getElement(uint32_t index)
	{
		if (index < _children.size()) { return _children[index]; }
		return StructuredMemoryDescription();
	}

	/// <summary>Gets an element name for the StructuredMemoryDescription.</summary>
	/// <returns>Returns the name for the StructuredMemoryDescription.</returns>
	const std::string& getName() const { return _name; }

	/// <summary>Gets an element type for the StructuredMemoryDescription.</summary>
	/// <returns>Returns the type for the StructuredMemoryDescription.</returns>
	const GpuDatatypes getType() { return _type; }

	/// <summary>Gets the number of array elements for the StructuredMemoryDescription.</summary>
	/// <returns>Returns the number of array elements for the StructuredMemoryDescription.</returns>
	const uint32_t getNumArrayElements() { return _numArrayElements; }

	/// <summary>Gets the number of child elements for the StructuredMemoryDescription.</summary>
	/// <returns>Returns the number of child elements for the StructuredMemoryDescription.</returns>
	const uint32_t getNumChildren() { return static_cast<uint32_t>(_children.size()); }
};

/// <summary>Defines a StructuredMemoryEntry element. A StructuredMemoryEntry defines an actual element entry in a structured buffer view.
/// WARNING: Arrays of size 1 not supported - they are considered non-arrays.
/// WARNING: Due to pointers to parents, no reallocations must *ever* happen. Once initialized,
/// the lists must be final.
/// WARNING: THE ENTIRE PUBLIC INTERFACE OF THIS CLASS EXCEPT FOR INIT, IS CONST.
/// WARNING: IT IS NOT INTENDED FOR THIS CLASS TO BE EVER MODIFIABLE.</summary>
class StructuredMemoryEntry
{
private:
	friend class StructuredBufferView;
	friend class StructuredBufferViewElement;

	StringHash _name;
	StructuredMemoryEntry* _parent;
	std::vector<StructuredMemoryEntry> _childEntries; // These must never reallocate. Hence the list is non-copyable
	GpuDatatypes _type;
	uint32_t _baseAlignment; // The alignment requirements of this object as defined by std140. Takes into account array etc.
	uint32_t _numArrayElements; // The number of array elements.
	bool _variableArray; // This is the last 1st level element, so can be resized dynamically
	uint64_t _size; // The minimum size of this item, including self-padding IF an arrays or structures, NOT taking into account next item alignment.
	uint64_t _singleElementSize; // The minimum size of this item, including self-padding IF an arrays or structures, NOT taking into account next item alignment.
	uint32_t _arrayMemberSize; // The size of each slice of this item, ALWAYS including self-padding for array or structure.
	uint32_t _offset;
	uint64_t _minDynamicAlignment;
	void* _mappedMemory;
	uint32_t _mappedDynamicSlice;

	struct IsEqual
	{
		const StringHash& _hash;
		IsEqual(const StringHash& name) : _hash(name) {}
		bool operator()(const StructuredMemoryEntry& rhs) const { return _hash == rhs.getName(); }
	};

	void calcBaseAlignment()
	{
		if (isStructure())
		{
			_baseAlignment = 0;
			for (auto& child : _childEntries)
			{
				child.calcBaseAlignment();
				_baseAlignment = std::max(_baseAlignment, child._baseAlignment);
			}
			_baseAlignment = std::max(_baseAlignment, getAlignment(GpuDatatypes::vec4)); // STD140
		}
		else
		{
			_baseAlignment = getAlignment(_type);
			if (_numArrayElements > 1) { _baseAlignment = std::max(_baseAlignment, getAlignment(GpuDatatypes::vec4)); }
		}
	}

	void calcDynamicAlignment(BufferUsageFlags usage = BufferUsageFlags::UniformBuffer, const uint64_t minUboDynamicAlignment = 0, const uint64_t minSsboDynamicAlignment = 0)
	{
		_minDynamicAlignment = 0;

		uint64_t uboAlign = 0;
		uint64_t ssboAlign = 0;

		// if dynamic ubo OR dynamic ssbo take the relevant alignment
		if (static_cast<uint32_t>(usage & BufferUsageFlags::UniformBuffer) != 0 && minUboDynamicAlignment) { uboAlign = minUboDynamicAlignment; }
		if (static_cast<uint32_t>(usage & BufferUsageFlags::StorageBuffer) != 0 && minSsboDynamicAlignment) { ssboAlign = minSsboDynamicAlignment; }
		_minDynamicAlignment = std::max(uboAlign, ssboAlign);

		if (isStructure())
		{
			for (auto& child : _childEntries) { child.calcDynamicAlignment(); }
		}
	}

	// INTERNAL NOTE: CALL THIS ** AFTER ** CALC BASE ALIGNMENT, TO FIX THE _offset AND _size MEMBERS
	void calcSizeAndOffset(const uint32_t& offset)
	{
		_offset = align(offset, _baseAlignment);
		_offset = align(_offset, _minDynamicAlignment);
		if (isStructure())
		{
			uint32_t tmp_offset = 0;
			for (auto& child : _childEntries)
			{
				child.calcSizeAndOffset(tmp_offset);
				tmp_offset = child.getOffset() + static_cast<uint32_t>(child.getSize());
			}
			// STD140: Structures are padded to their alignment so that a[n] ==>   sizeof(a[0]) == sizeof[a] / n
			tmp_offset = align(tmp_offset, _baseAlignment);
			tmp_offset = align(tmp_offset, _minDynamicAlignment);
			_arrayMemberSize = tmp_offset;
			_singleElementSize = _arrayMemberSize;

			tmp_offset *= _numArrayElements;
			_size = tmp_offset;
		}
		else
		{
			_arrayMemberSize = getSelfAlignedArraySize(_type);
			_singleElementSize = pvr::getSize(_type);
			_size = _variableArray ? _arrayMemberSize * _numArrayElements : pvr::getSize(_type, _numArrayElements);
		}
	}

	void privateInit(const StructuredMemoryDescription& desc, StructuredMemoryEntry* parent, bool firstLevel, bool isVariableArray)
	{
		_name = desc._name;
		_numArrayElements = desc._numArrayElements;
		_variableArray = isVariableArray;
		_parent = parent;
		this->_type = desc._type;
		if (!desc._children.empty()) // isStruct
		{
			_childEntries.resize(desc._children.size()); // Called once
			for (uint32_t i = 0; i < desc._children.size(); ++i)
			{
				// PASS "TRUE" TO "VARIABLEARRAY" OF THE LAST ENTRY OF THE FIRST LEVEL.
				// THIS IS TO A) ALLOW SSBO VARIABLE SIZED ARRAYS.
				// B) ALIGN THE SIZE OF THE WHOLE BLOCK TO CONTAIN ITS OWN PADDING
				_childEntries[i].privateInit(desc._children[i], this, false, firstLevel && (i + 1 == desc._children.size()));
			}
			_type = GpuDatatypes::none;
		}
	}

	void layout(BufferUsageFlags usage = BufferUsageFlags::UniformBuffer, const uint64_t minUboDynamicAlignment = 0, const uint64_t minSsboDynamicAlignment = 0)
	{
		calcBaseAlignment();
		calcDynamicAlignment(usage, minUboDynamicAlignment, minSsboDynamicAlignment);
		calcSizeAndOffset(0);
	}

	/// <summary>Assigns memory for this structured buffer view to point towards. Can also set the mapped dynamic
	/// slice which will be used to adjust any offsets retrieved or used when setting buffer values.</summary>
	/// <param name="mappedMemory">The mapped memory to point to</param>
	/// <param name="mappedDynamicSlice">The dynamic slice used when mapping the mapped memory.</param>
	void setMappedMemory(void* mappedMemory, const uint32_t mappedDynamicSlice = 0)
	{
		_mappedMemory = mappedMemory;
		_mappedDynamicSlice = mappedDynamicSlice;
	}

	/// <summary>Get dynamic slice which was mapped when a buffer was mapped which follows the structure set out by this StructuredMemoryEntry. The mapped
	/// dynamic slice is used to adjust the offset value used when setting values. The mapped dynamic slice is set via a call to pointToMappedMemory
	/// on the structured buffer view. This function should only be called on the root</summary>
	/// <returns>Return the mapped dynamic slice</returns>
	uint32_t getMappedDynamicSlice() const { return _mappedDynamicSlice; }

	/// <summary>Get the mapped memory pointed to by this StructuredMemoryEntry.
	/// The mapped memory is set via a call to pointToMappedMemory on the structured buffer view. This function should only be called on the root</summary>
	/// <returns>Return the mapped memory</returns>
	void* getMappedMemory() const { return _mappedMemory; }

	void fixParentPointers(StructuredMemoryEntry* parent)
	{
		_parent = parent;
		for (auto&& child : _childEntries)
		{ //
			child.fixParentPointers(this);
		}
	}
	friend void swap(StructuredBufferView& first, StructuredBufferView& second);

public:
	/// <summary>A default Constructor for a StructuredMemoryEntry.</summary>
	StructuredMemoryEntry()
		: _name(), _parent(0), _type(GpuDatatypes::none), _numArrayElements(0), _size(0), _arrayMemberSize(0), _singleElementSize(0), _offset(0), _mappedMemory(nullptr),
		  _mappedDynamicSlice(0), _baseAlignment(0), _minDynamicAlignment(0), _variableArray(0)
	{}

	/// <summary>A Copy constructor for a StructuredMemoryEntry.</summary>
	/// <param name="other">The StructuredMemoryEntry to copy from</param>
	StructuredMemoryEntry(const StructuredMemoryEntry& other)
	{
		_name = other._name;
		_parent = other._parent;
		_childEntries.reserve(other._childEntries.size());
		std::copy(other._childEntries.begin(), other._childEntries.end(), back_inserter(_childEntries));
		_type = other._type;
		_baseAlignment = other._baseAlignment;
		_numArrayElements = other._numArrayElements;
		_variableArray = other._variableArray;
		_size = other._size;
		_singleElementSize = other._singleElementSize;
		_arrayMemberSize = other._arrayMemberSize;
		_offset = other._offset;
		_minDynamicAlignment = other._minDynamicAlignment;
		_mappedMemory = other._mappedMemory;
		_mappedDynamicSlice = other._mappedDynamicSlice;
	}

	/// <summary>Destructor. Virtual (for polymorphic use).</summary>
	virtual ~StructuredMemoryEntry() {}

	/// <summary>A swap function for a pvr::utils::StructuredMemoryEntry. Swaps all members of each pvr::utils::StructuredMemoryEntry to the other.</summary>
	/// <param name="first">The first StructuredMemoryEntry to swap</param>
	/// <param name="second">The second StructuredMemoryEntry to swap</param>
	friend void swap(StructuredMemoryEntry& first, StructuredMemoryEntry& second)
	{
		std::swap(first._name, second._name);
		std::swap(first._parent, second._parent);
		std::swap(first._childEntries, second._childEntries);
		std::swap(first._type, second._type);
		std::swap(first._baseAlignment, second._baseAlignment);
		std::swap(first._numArrayElements, second._numArrayElements);
		std::swap(first._variableArray, second._variableArray);
		std::swap(first._size, second._size);
		std::swap(first._singleElementSize, second._singleElementSize);
		std::swap(first._arrayMemberSize, second._arrayMemberSize);
		std::swap(first._offset, second._offset);
		std::swap(first._minDynamicAlignment, second._minDynamicAlignment);
		std::swap(first._mappedMemory, second._mappedMemory);
		std::swap(first._mappedDynamicSlice, second._mappedDynamicSlice);
	}

	/// <summary>Copy Assignment operator.</summary>
	/// <param name="other">The StructuredMemoryEntry to assign to this StructuredMemoryEntry</param>
	/// <returns>this object (allows chained calls)</returns>
	StructuredMemoryEntry& operator=(StructuredMemoryEntry other)
	{
		swap(*this, other);
		return *this;
	}

	/// <summary>Returns the number of children.</summary>
	/// <returns>Return the number of child entries.</returns>
	const uint32_t getNumChildren() const { return static_cast<uint32_t>(_childEntries.size()); }

	/// <summary>Returns the child at the given index.</summary>
	/// <param name="index">The index of the child to return</param>
	/// <returns>Return the child StructuredMemoryEntry.</returns>
	const StructuredMemoryEntry& getChild(uint32_t index) const { return _childEntries[index]; }

	/// <summary>Returns the parent.</summary>
	/// <returns>Return the StructuredMemoryEntry parent.</returns>
	const StructuredMemoryEntry* getParent() const { return _parent; }

	/// <summary>Returns the number of array elements.</summary>
	/// <returns>Return the number of array elements.</returns>
	uint32_t getNumArrayElements() const { return _numArrayElements; }

	/// <summary>Check if the StructuredMemoryEntry is a structure.</summary>
	/// <returns>Returns true if the StructuredMemoryEntry is a structure.</returns>
	bool isStructure() const { return _childEntries.size() != 0; }

	/// <summary>Check if the StructuredMemoryEntry has a primitive data type.</summary>
	/// <returns>Returns true if the StructuredMemoryEntry has a primitive data type.</returns>
	bool isPrimitive() const { return !isStructure(); }

	/// <summary>Checks the name of the StructuredMemoryEntry.</summary>
	/// <returns>Returns the given name of the StructuredMemoryEntry.</returns>
	const StringHash& getName() const { return _name; }

	/// <summary>Checks the primitive type of the StructuredMemoryEntry.</summary>
	/// <returns>Returns the typeof the StructuredMemoryEntry.</returns>
	GpuDatatypes getPrimitiveType() const { return _type; }

	/// <summary>Gets the offset for the StructuredMemoryEntry.</summary>
	/// <returns>Returns the offset the StructuredMemoryEntry.</returns>
	uint32_t getOffset() const { return _offset; }

	/// <summary>Returns the particular array element offset for the StructuredMemoryEntry.</summary>
	/// <param name="arrayElement">The array element to retrieve the offset for</param>
	/// <returns>Return the array element offset for the given arrayElement.</returns>
	uint32_t getArrayElementOffset(uint32_t arrayElement) const { return _offset + _arrayMemberSize * arrayElement; }

	/// <summary>Gets the size of the StructuredMemoryEntry.</summary>
	/// <returns>Returns the size the StructuredMemoryEntry.</returns>
	uint64_t getSize() const { return _size; }

	/// <summary>Gets the size of the StructuredMemoryEntry.</summary>
	/// <returns>Returns the size the StructuredMemoryEntry.</returns>
	uint64_t getSingleItemSize() const { return _singleElementSize; }

	/// <summary>Sets the element array size for the last entry in the StructuredMemoryEntry.
	/// Only the last element in the StructuredMemoryEntry may have its size set from the api</summary>
	/// <param name="arraySize">The size of the array</param>
	void setLastElementArraySize(uint32_t arraySize)
	{
		auto& child = _childEntries.back();

		uint64_t oldSize = child._size;
		uint64_t newSize = child._arrayMemberSize * arraySize;
		debug_assertion(child._size == (child._arrayMemberSize * child._numArrayElements), "1");
		child._numArrayElements = arraySize;

		int64_t sizeDiff = static_cast<int64_t>(newSize) - static_cast<int64_t>(oldSize);
		child._size = newSize;
		_size += sizeDiff;
	}

	/// <summary>Returns the index of the child with the given name.</summary>
	/// <param name="name">The name to search for</param>
	/// <returns>Return the index of the element with the given name.</returns>
	uint32_t getIndex(const StringHash& name) const
	{
		auto entry = std::find_if(_childEntries.begin(), _childEntries.end(), IsEqual(name));
		if (entry == _childEntries.end()) { return static_cast<uint32_t>(-1); }
		return static_cast<uint32_t>(entry - _childEntries.begin());
	}

	/// <summary>Initialise the StructuredMemoryEntry using the given description. This function should be used to initialise non-dynamic buffers
	/// //ONLY CALL ON THE ROOT ELEMENT</summary>
	/// <param name="desc">The description used to initialise the StructuredMemoryEntry</param>
	void init(const StructuredMemoryDescription& desc)
	{
		privateInit(desc, nullptr, true, false);
		layout();
	}

	/// <summary>Initialise the StructuredMemoryEntry using the given description. This function should be used to initialise dynamic buffers
	/// //ONLY CALL ON THE ROOT ELEMENT</summary>
	/// <param name="desc">The description used to initialise the StructuredMemoryEntry</param>
	/// <param name="usage">The intended usage of the StructuredMemoryEntry</param>
	/// <param name="minUboDynamicAlignment">The minimum dynamic alignment when used as a uniform buffer</param>
	/// <param name="minSsboDynamicAlignment">The minimum dynamic alignment when used as a storage buffer</param>
	void initDynamic(const StructuredMemoryDescription& desc, BufferUsageFlags usage = BufferUsageFlags::UniformBuffer, const uint64_t minUboDynamicAlignment = 0,
		const uint64_t minSsboDynamicAlignment = 0)
	{
		privateInit(desc, nullptr, true, false);
		layout(usage, minUboDynamicAlignment, minSsboDynamicAlignment);
	}

	/// <summary>Prints a preamble for the current StructuredMemoryEntry level</summary>
	/// <param name="str">The current std::stringstream to print to</param>
	/// <param name="level">The current level to print a preamble for</param>
	inline static void printPreamble(std::stringstream& str, uint32_t level)
	{
		for (uint32_t i = 0; i < level; ++i) { str << " "; }
	}

	/// <summary>Prints a StructuredMemoryEntry to a std::stringstream for the current level</summary>
	/// <param name="str">The current std::stringstream to print to</param>
	/// <param name="level">The current level to print a preamble for</param>
	void printIntoStringStream(std::stringstream& str, uint32_t level) const
	{
		str << "\n" << std::setw(3) << _offset << ": ";
		printPreamble(str, level * 2);
		str << (isStructure() ? "struct" : toString(_type)) << " " << _name.str();
		if (_numArrayElements > 1) { str << "[" << _numArrayElements << "]"; }
		str << ";";
		if (!isStructure()) { str << "\t"; }
		str << "\t baseSz:" << _size / _numArrayElements << "\t size:" << getSize() << "\t baseAlign:" << _baseAlignment << "\t nextOffset:" << _offset + getSize()
			<< "\t arrayMemberSize:" << _arrayMemberSize;
		if (isStructure())
		{
			str << "\n";
			printPreamble(str, level * 2 + 5);
			str << "{";
			for (auto& child : _childEntries) { child.printIntoStringStream(str, level + 1); }
			str << "\n";
			printPreamble(str, level * 2 + 5);
			str << "}";
		}
	}
};

//!\cond NO_DOXYGEN
class StructuredBufferView;
//!\endcond

/// <summary>Defines a StructuredBufferViewElement. A StructuredBufferViewElement handles the public interface used for working with a StructuredMemoryEntry.</summary>
class StructuredBufferViewElement
{
private:
	friend class StructuredBufferView;
	uint32_t _offset;
	void* _mappedMemory;
	uint32_t _level;
	uint32_t _indices[5]; // This, is the array index of each ancestor item up the chain. Is carried to children elements to enable offset calcs.
	const StructuredMemoryEntry& _prototype;
	StructuredBufferViewElement(const StructuredMemoryEntry& entry, uint32_t level, uint32_t elementArrayIndex, const uint32_t* parentIndices, uint32_t dynamicSlice = 0)
		: _mappedMemory(nullptr), _level(level), _prototype(entry)
	{
		_indices[0] = elementArrayIndex;
		if (parentIndices) { memcpy(_indices + 1, parentIndices, sizeof(uint32_t) * (level)); }
		init(dynamicSlice);
	}
	void init(uint32_t dynamicSlice = 0)
	{
		// Get the offset INSIDE CURRENT LEVEL.
		_offset = _prototype.getArrayElementOffset(_indices[0]);
		const StructuredMemoryEntry* parent = _prototype.getParent();
		size_t level = 1; // How many levels up we have gone
		uint64_t dynamicSliceSize = 0;
		uint32_t mappedDynamicSlice = 0;
		while (parent) // Until we reach the root element
		{
			debug_assertion(parent->getNumArrayElements() > _indices[level], "StructuredBufferViewElement: Attempted out-of-bounds access in getOffset");
			_offset += parent->getArrayElementOffset(_indices[level++]);
			dynamicSliceSize = parent->getSize();
			mappedDynamicSlice = parent->getMappedDynamicSlice();
			// store the mapped memory so we only have to do this lookup once rather than each time setValue is called
			_mappedMemory = parent->getMappedMemory();
			parent = parent->getParent();
		}

		// at this point dynamicSliceSize matches the root size
		debug_assertion(dynamicSlice >= mappedDynamicSlice, "StructuredBufferViewElement: Mapped dynamic slice must be greater than or equal to the current dynamic slice");
		uint32_t sliceOffset = (dynamicSlice * static_cast<uint32_t>(dynamicSliceSize)) - (mappedDynamicSlice * static_cast<uint32_t>(dynamicSliceSize));
		_offset += sliceOffset;
	}

	void* getMappedMemory()
	{
		debug_assertion(_mappedMemory != nullptr, "StructuredBufferViewElement: Before getting mapped memory the memory must be set.");
		return _mappedMemory;
	}

public:
	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const StringHash& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const StringHash& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const std::string& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const std::string& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const char* str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const char* str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return getElement(_prototype.getIndex(str), elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElement(uint32_t elementIndex, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return StructuredBufferViewElement(_prototype.getChild(elementIndex), _level + 1, elementArrayIndex, _indices, dynamicSlice);
	}

	/// <summary>Gets an element using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElement(uint32_t elementIndex, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return StructuredBufferViewElement(_prototype.getChild(elementIndex), _level + 1, elementArrayIndex, _indices, dynamicSlice);
	}

	/// <summary>Gets an element using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const std::string& getElementNameByIndex(uint32_t elementIndex) const { return _prototype.getChild(elementIndex).getName(); }

	/// <summary>Gets the index for a given element</summary>
	/// <param name="str">The name of the element to retrieve the index for</param>
	/// <returns>Return the index of the StructuredBufferViewElement.</returns>
	uint32_t getIndex(const StringHash& str) { return _prototype.getIndex(str); }

	/// <summary>Gets the index for a given element</summary>
	/// <param name="str">The name of the element to retrieve the index for</param>
	/// <returns>Return the index of the StructuredBufferViewElement.</returns>
	uint32_t getIndex(const std::string& str) { return _prototype.getIndex(str); }

	/// <summary>Gets the index for a given element</summary>
	/// <param name="str">The name of the element to retrieve the index for</param>
	/// <returns>Return the index of the StructuredBufferViewElement.</returns>
	uint32_t getIndex(const char* str) { return _prototype.getIndex(str); }

	/// <summary>Gets the offset for the StructuredBufferViewElement. This function takes into account any
	/// mapped memory. if the mapped dynamic slice is not equal to zero then the offset returned here
	/// will be adjusted based on the mapped dynamic slice.</summary>
	/// <returns>Return the offset of the StructuredBufferViewElement.</returns>
	uint32_t getOffset() const { return _offset; }

	/// <summary>Gets the size of the underlying structure memory entry</summary>
	/// <returns>Return the size of the underlying structure memory entry.</returns>
	uint64_t getValueSize() const { return _prototype.getSingleItemSize(); }

	/// <summary>Gets the size of the underlying structure memory entry</summary>
	/// <returns>Return the size of the underlying structure memory entry.</returns>
	uint64_t getArrayPaddedSize() const { return _prototype._arrayMemberSize; }

// clang-format off
/// <summary>Contain functions to set values for a number of gpu compatible data types.</summary>
#define DEFINE_SETVALUE_FOR_TYPE(ParamType)\
  void setValue(const ParamType& value)\
  {\
  memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), &value, (size_t)getValueSize()); \
  }\
  \
  void setValue(const ParamType* value)\
  {\
    memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), value, (size_t)getValueSize()); \
  }
	DEFINE_SETVALUE_FOR_TYPE(float)
	DEFINE_SETVALUE_FOR_TYPE(uint32_t)
	DEFINE_SETVALUE_FOR_TYPE(uint64_t)
	DEFINE_SETVALUE_FOR_TYPE(int32_t)
	DEFINE_SETVALUE_FOR_TYPE(int64_t)
	DEFINE_SETVALUE_FOR_TYPE(glm::vec2)
	DEFINE_SETVALUE_FOR_TYPE(glm::vec4)
	DEFINE_SETVALUE_FOR_TYPE(glm::ivec2)
	DEFINE_SETVALUE_FOR_TYPE(glm::ivec4)
	DEFINE_SETVALUE_FOR_TYPE(glm::uvec2)
	DEFINE_SETVALUE_FOR_TYPE(glm::uvec4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat2x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat2x4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat3x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat3x4)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat4x2)
	DEFINE_SETVALUE_FOR_TYPE(glm::mat4x4)

#undef DEFINE_SETVALUE_FOR_TYPE
	// clang-format on

	/// <summary>Sets the value (glm::vec3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::vec3& value) { memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), &value, sizeof(glm::vec3)); }

	/// <summary>Sets the value (glm::ivec3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::ivec3& value) { memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), &value, sizeof(glm::ivec3)); }

	/// <summary>Sets the value (glm::ivec3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::uvec3& value) { memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), &value, sizeof(glm::uvec3)); }

	/// <summary>Sets the value (glm::vec3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::vec3* value)
	{
		for (uint32_t i = 0; i < this->_prototype.getNumArrayElements(); ++i)
		{ memcpy(static_cast<char*>(getMappedMemory()) + getOffset() + sizeof(glm::vec4) * i, value + i, sizeof(glm::vec3)); }
	}

	/// <summary>Sets the value (glm::ivec3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::ivec3* value)
	{
		for (uint32_t i = 0; i < this->_prototype.getNumArrayElements(); ++i)
		{ memcpy(static_cast<char*>(getMappedMemory()) + getOffset() + sizeof(glm::ivec4) * i, value + i, sizeof(glm::ivec3)); }
	}

	/// <summary>Sets the value (glm::uvec3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::uvec3* value)
	{
		for (uint32_t i = 0; i < this->_prototype.getNumArrayElements(); ++i)
		{ memcpy(static_cast<char*>(getMappedMemory()) + getOffset() + sizeof(glm::uvec4) * i, value + i, sizeof(glm::uvec3)); }
	}

	/// <summary>Sets the value (glm::mat2x3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::mat2x3& value)
	{
		glm::mat2x4 newvalue(value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value (glm::mat3x3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::mat3x3& value)
	{
		glm::mat3x4 newvalue(value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value (glm::mat4x3 specific) for this element taking the source by reference</summary>
	/// <param name="value">The value to set by reference</param>
	void setValue(const glm::mat4x3& value)
	{
		glm::mat4x4 newvalue(value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value (glm::mat2x3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::mat2x3* value)
	{
		glm::mat2x4 newvalue(*value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value (glm::mat3x3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::mat3x3* value)
	{
		glm::mat3x4 newvalue(*value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value (glm::mat4x3 specific) for this element taking the source by pointer</summary>
	/// <param name="value">The value to set by pointer</param>
	void setValue(const glm::mat4x3* value)
	{
		glm::mat4x4 newvalue(*value);
		memcpy(static_cast<char*>(getMappedMemory()) + getOffset(), (const void*)&newvalue, (size_t)getValueSize());
	}

	/// <summary>Sets the value of this element using a FreeValue which encapsulates various data types</summary>
	/// <param name="value">The free value to set by reference</param>
	void setValue(const FreeValue& value)
	{
		if (this->_prototype.getPrimitiveType() != value.dataType() && value.dataType() != GpuDatatypes::mat3x3)
		{ throw std::runtime_error("StructuredBufferView: Mismatched FreeValue datatype"); }
		if (value.dataType() == GpuDatatypes::mat3x3)
		{
			glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>());
			memcpy((char*)getMappedMemory() + getOffset(), &tmp[0][0], (size_t)getValueSize());
		}
		else
		{
			memcpy((char*)getMappedMemory() + getOffset(), value.raw(), (size_t)getValueSize());
		}
	}

	/// <summary>Sets the value for this element using typed memory</summary>
	/// <param name="value">The TypedMem to set by reference</param>
	void setValue(const TypedMem& value)
	{
		debug_assertion(value.arrayElements() == 1, "Calling set value would have updated multiple elements here");
		setArrayValuesStartingFromThis(value);
	}

	/// <summary>Sets a number of array values in a single call using typed memory. Without this multiple setValue calls would be necessary.</summary>
	/// <param name="value">The TypedMem to set by reference</param>
	void setArrayValuesStartingFromThis(const TypedMem& value)
	{
		if (this->_prototype.getPrimitiveType() != value.dataType() && value.dataType() != GpuDatatypes::mat3x3)
		{ throw std::runtime_error("StructuredBufferView: Mismatched FreeValue datatype"); }
		if (value.arrayElements() != _prototype.getNumArrayElements()) { throw std::runtime_error("StructuredBufferView: Mismatched number of array elements"); }
		if (value.dataType() == GpuDatatypes::mat3x3)
		{
			size_t startoff = getOffset();
			for (uint32_t i = 0; i < value.arrayElements(); ++i)
			{
				glm::mat3x4 tmp(value.interpretValueAs<glm::mat3x3>(i));

				uint64_t myoff = startoff + _prototype._arrayMemberSize * i;
				uint64_t myvaluesize = getSize(value.dataType());
				memcpy((char*)getMappedMemory() + (size_t)myoff, &tmp[0][0], (size_t)myvaluesize);
			}
		}
		else
		{
			uint64_t myoff = getOffset();
			uint64_t myvaluesize = value.dataSize();
			memcpy((char*)getMappedMemory() + (size_t)myoff, value.raw(), (size_t)myvaluesize);
		}
	}

	/// <summary>Gets the number of elements for a particular level of the structured buffer view</summary>
	/// <returns>Return the number of elements for a particular level of the structured buffer view.</returns>
	uint32_t getNumElements()
	{
		if (_prototype.isStructure()) { return _prototype.getNumChildren(); }
		return 1;
	}
};

/// <summary>A structured buffer view is a class that can be used to define an explicit structure to an object
/// that is usually accessed as raw memory. For example, a GPU-side buffer is mapped to a void pointer, but a
/// StructuredBufferView can be used to create a runtime structure for it, and set its entries one by one.
/// Example usage for accessing a StructuredBufferView defined for the buffer bonesUbo defined below:
/// struct Bone{
///   highp mat4 boneMatrix;
///   highp mat3 boneMatrixIT;
/// };
/// layout(std140, binding = i) uniform bonesUbo
/// {
///   mediump int BoneCount; // a name or index can be used to retrieve a particular element
///   Bone bones[]; // elementArrayIndex is used to index into an arrays of elements
/// } boneBuffer;
/// getElementByName("BoneCount") = boneBuffer.BoneCount
/// getElementByName("bones") = boneBuffer.Bone[0]
/// getElementByName("bones", 1) = boneBuffer.Bone[1]</summary>
class StructuredBufferView
{
private:
	StructuredMemoryEntry _root;
	uint32_t _numDynamicSlices;

public:
	/// <summary>Constructor. Creates an empty StructuredBufferView.</summary>
	StructuredBufferView() : _numDynamicSlices(1) {}

	/// <summary>Constructor. Creates an empty StructuredBufferView.</summary>
	StructuredBufferView(const StructuredBufferView& other) : _numDynamicSlices(other._numDynamicSlices), _root(other._root)
	{ //
		_root.fixParentPointers(0);
	}
	/// <summary>Constructor. Creates an empty StructuredBufferView.</summary>
	StructuredBufferView(const StructuredBufferView&& other) : _numDynamicSlices(other._numDynamicSlices), _root(std::move(other._root))
	{ //
		_root.fixParentPointers(0);
	}

	StructuredBufferView& operator=(StructuredBufferView other)
	{
		swap(*this, other);
		_root.fixParentPointers(0);
		return *this;
	}

	friend void swap(StructuredBufferView& first, StructuredBufferView& second)
	{
		swap(first._root, second._root);
		std::swap(first._numDynamicSlices, second._numDynamicSlices);
		first._root.fixParentPointers(0);
		second._root.fixParentPointers(0);
	}

	/// <summary>Assigns memory for this structured buffer view to point towards. Can also set the mapped dynamic
	/// slice which will be used to adjust any offsets retrieved or used when setting buffer values.</summary>
	/// <param name="mappedMemory">The mapped memory to point to</param>
	/// <param name="mappedDynamicSlice">The dynamic slice used when mapping the mapped memory.</param>
	void pointToMappedMemory(void* mappedMemory, const uint32_t mappedDynamicSlice = 0) { _root.setMappedMemory(mappedMemory, mappedDynamicSlice); }

	/// <summary>Get the size of the whole buffer represented by the StructuredBufferView. This size takes into account dynamic slices.</summary>
	/// <returns>Return the size of the buffer</returns>
	uint64_t getSize() const { return getDynamicSliceSize() * _numDynamicSlices; }

	/// <summary>Get dynamic slice which was mapped when a buffer was mapped which follows the structure set out by this StructuredBufferView. The mapped
	/// dynamic slice is used to adjust the offset value used when setting values. The mapped dynamic slice is set via a call to pointToMappedMemory.</summary>
	/// <returns>Return the mapped dynamic slice</returns>
	uint32_t getMappedDynamicSlice() const { return _root.getMappedDynamicSlice(); }

	/// <summary>Get the mapped memory pointed to by this StructuredBufferView. The mapped memory is set via a call to pointToMappedMemory.</summary>
	/// <returns>Return the mapped memory</returns>
	const void* getMappedMemory() const { return _root.getMappedMemory(); }

	/// <summary>Get the size of a given dynamic slice of the buffer represented by the StructuredBufferView.</summary>
	/// <returns>Return the size of particular dynamic slice</returns>
	uint64_t getDynamicSliceSize() const { return _root.getSize(); }

	/// <summary>Get the number of dynamic slices the StructuredBufferView uses.</summary>
	/// <returns>Return the number of dynamic slices</returns>
	uint32_t getNumDynamicSlices() const { return _numDynamicSlices; }

	/// <summary>Get the StructuredBufferViewName.</summary>
	/// <returns>The name of root element</returns>
	const std::string& getName() const { return _root.getName(); }

	/// <summary>Get the offset for the given dynamic slice.</summary>
	/// <param name="dynamicSliceIndex">The dynamic slice to retrieve the offset for</param>
	/// <returns>Return the offset for the given dynamic slice</returns>
	uint32_t getDynamicSliceOffset(uint32_t dynamicSliceIndex) const { return dynamicSliceIndex * static_cast<uint32_t>(getDynamicSliceSize()); }

	/// <summary>Initialises the StructuredBufferView for a non-dynamic buffer.</summary>
	/// <param name="desc">The description to use for initialising the StructuredBufferView</param>
	void init(const StructuredMemoryDescription& desc) { _root.init(desc); }

	/// <summary>Initialises the StructuredBufferView for a dynamic buffer.</summary>
	/// <param name="desc">The description to use for initialising the StructuredBufferView</param>
	/// <param name="numDynamicSlices">The number of dynamic slices to use for the StructuredBufferView</param>
	/// <param name="usage">The intended usage of the underlying buffer</param>
	/// <param name="minUboDynamicAlignment">The minimum dynamic alignment when used as a uniform buffer</param>
	/// <param name="minSsboDynamicAlignment">The minimum dynamic alignment when used as a storage buffer</param>
	void initDynamic(const StructuredMemoryDescription& desc, uint32_t numDynamicSlices = 1, BufferUsageFlags usage = BufferUsageFlags::UniformBuffer,
		const uint64_t minUboDynamicAlignment = 0, const uint64_t minSsboDynamicAlignment = 0)
	{
		_root.initDynamic(desc, usage, minUboDynamicAlignment, minSsboDynamicAlignment);
		_numDynamicSlices = numDynamicSlices;
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const StringHash& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const StringHash& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const std::string& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const std::string& str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElementByName(const char* str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using a name</summary>
	/// <param name="str">The name of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElementByName(const char* str, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementByName(str, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	const StructuredBufferViewElement getElement(uint32_t elementIndex, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0) const
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElement(elementIndex, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <param name="elementArrayIndex">The element array index of the element to retrieve</param>
	/// <param name="dynamicSlice">The dynamic slice of the element to retrieve.</param>
	/// <returns>Return the StructuredBufferViewElement.</returns>
	StructuredBufferViewElement getElement(uint32_t elementIndex, uint32_t elementArrayIndex = 0, uint32_t dynamicSlice = 0)
	{
		return StructuredBufferViewElement(_root, 0, 0, nullptr).getElement(elementIndex, elementArrayIndex, dynamicSlice);
	}

	/// <summary>Gets an element name using an index</summary>
	/// <param name="elementIndex">The index of the element to retrieve</param>
	/// <returns>Return the name of the element at the index.</returns>
	std::string getElementNameByIndex(uint32_t elementIndex) { return StructuredBufferViewElement(_root, 0, 0, nullptr).getElementNameByIndex(elementIndex); }

	/// <summary>Gets the number of elements for a particular level of the structured buffer view</summary>
	/// <returns>Return the number of elements for a particular level of the structured buffer view.</returns>
	uint32_t getNumElements() { return StructuredBufferViewElement(_root, 0, 0, nullptr).getNumElements(); }

	/// <summary>Sets the element array size for the last entry in the StructuredBufferViewElement.
	/// Only the last element in the StructuredBufferViewElement may have its size set from the api</summary>
	/// <param name="arraySize">The size of the array</param>
	void setLastElementArraySize(uint32_t arraySize) { return _root.setLastElementArraySize(arraySize); }

	/// <summary>Retrieve the index of a variable by its name</summary>
	/// <param name="name">The name of a element</param>
	/// <returns>The index of a variable entry</returns>
	uint32_t getIndex(const StringHash& name) const { return _root.getIndex(name); }

	/// <summary>Retrieve the index of a variable by its name</summary>
	/// <param name="name">The name of a element</param>
	/// <returns>The index of a variable entry</returns>
	uint32_t getIndex(const char* name) const { return _root.getIndex(name); }

	/// <summary>Retrieve the index of a variable by its name</summary>
	/// <param name="name">The name of a element</param>
	/// <returns>The index of a variable entry</returns>
	uint32_t getIndex(const std::string& name) const { return _root.getIndex(name); }

	/// <summary>Converts the StructuredBufferView to a readable string entry</summary>
	/// <returns>The human readable string corresponding to the StructuredBufferView</returns>
	std::string toString()
	{
		std::stringstream ss;
		ss << "\n";
		_root.printIntoStringStream(ss, 0);
		return ss.str();
	}
};
} // namespace utils
} // namespace pvr
