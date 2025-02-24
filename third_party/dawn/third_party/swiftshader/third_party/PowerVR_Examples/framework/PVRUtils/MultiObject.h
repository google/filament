/*!
\brief This file contains the Multi<T> class template, which is the usual container for objects that must be
mirrored in the API level: Framebuffers etc.
\file PVRUtils/MultiObject.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <vector>

namespace pvr {
/// <summary>A small statically allocated array This class represents a small array of items. The array is
/// statically allocated, and has at most 255 items, usually 8. It is not meant to (and cannot) be used to store
/// large numbers of items (use a std::vector instead), rather it is meant to hold small tuples of items. The
/// PowerVR framework utilizes this class to store tuples of one-per-swap-image items</summary>
template<typename T_, uint8_t MAX_ITEMS = 4>
class Multi
{
public:
	typedef T_ ElementType; //!<  The items contained in this object
	typedef ElementType ContainerType[MAX_ITEMS]; //!<  The type of the container
private:
	ContainerType container;
	uint32_t numItems;

public:
	/// <summary>Get data</summary>
	/// <returns>Returns a pointer to the first element.</returns>
	ElementType* data() { return numItems > 0 ? container : nullptr; }

	/// <summary>Indexing operator</summary>
	/// <param name="idx">The index</param>
	/// <returns>Returns a reference to the indexed item.</returns>
	ElementType& operator[](uint32_t idx)
	{
		if (idx >= numItems) { numItems = idx + 1; }
		return container[idx];
	}
	/// <summary>Const Indexing operator</summary>
	/// <param name="idx">The index</param>
	/// <returns>Returns a const reference to the indexed item.</returns>
	const ElementType& operator[](uint32_t idx) const
	{
		assertion(idx < MAX_ITEMS);
		return container[idx];
	}

	/// <summary>Get a reference to the actually held items (a C-array of the specified max number of items)</summary>
	/// <returns>The array of items.</returns>
	ContainerType& getContainer() { return container; }

	/// <summary>Get a reference to the last item</summary>
	/// <returns>The last object. If it is out of range, the behaviour is undefined.</returns>
	ElementType& back() { return container[numItems - 1]; }

	/// <summary>Get a reference to the last item</summary>
	/// <returns>The last object. If it is out of range, the behaviour is undefined.</returns>
	const ElementType& back() const { return container[numItems - 1]; }

	/// <summary>Number of items currently held in the object</summary>
	/// <returns>Number of items currently held in the object</returns>
	size_t size() const { return numItems; }

	/// <summary>Set the number of items held in the object</summary>
	/// <param name="newsize">The new size. Clears unused items by default-initializing.</param>
	/// <returns>The number of items currently held in the object</returns>
	void resize(uint32_t newsize)
	{
		for (uint32_t i = newsize + 1; i < numItems; ++i)
		{
			container[i] = ElementType(); // clean up unused elements
		}
		numItems = static_cast<uint32_t>(newsize);
	}

	/// <summary>Empty the object</summary>
	void clear() { resize(0); }

	/// <summary>Constructor. Constructs empty object.</summary>
	Multi() : numItems(0) {}

	/// <summary>Constructor. Copy the given object to all entries of the container</summary>
	/// <param name="element">The element to copy into all entries of the container</param>
	explicit Multi(const ElementType& element) : numItems(MAX_ITEMS)
	{
		for (size_t i = 0; i < numItems; ++i) { container[i] = element; }
	}

	/// <summary>Constructor. Copy the initial objects from a c-style array</summary>
	/// <param name="elements">A C-style array of elements to copy</param>
	/// <param name="count">The number of items to copy</param>
	Multi(const ElementType* elements, const uint32_t count) : numItems(count)
	{
		assertion(count <= MAX_ITEMS, "Multi<T>: Index out of range");
		for (size_t i = 0; i < count; ++i) { container[i] = elements[i]; }
	}

	/// <summary>Add an item past the current end of the array.</summary>
	/// <param name="element">The item to add</param>
	void add(const ElementType& element)
	{
		assertion(numItems < MAX_ITEMS, "Multi<T>: Index out of range");
		container[numItems++] = element;
	}
	/// <summary>Add multiple item past the current end of the array.</summary>
	/// <param name="element">C-style array of items to add</param>
	/// <param name="count">Number of items to add</param>
	void add(const ElementType* element, const uint32_t count)
	{
		assertion(numItems + count <= MAX_ITEMS, "Multi<T>: Index out of range");
		for (uint32_t i = 0; i < count; ++i) { container[numItems++] = element[i]; }
	}
};
} // namespace pvr
