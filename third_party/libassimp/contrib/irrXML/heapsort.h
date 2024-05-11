// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_HEAPSORT_H_INCLUDED__
#define __IRR_HEAPSORT_H_INCLUDED__

#include "irrTypes.h"

namespace irr
{
namespace core
{

//! Sinks an element into the heap.
template<class T>
inline void heapsink(T*array, s32 element, s32 max)
{
	while ((element<<1) < max)	// there is a left child
	{
		s32 j = (element<<1);
	
		if (j+1 < max && array[j] < array[j+1])
			j = j+1;							// take right child

		if (array[element] < array[j])
		{
			T t = array[j];						// swap elements
			array[j] = array[element];
			array[element] = t;
			element = j;
		}
		else
			return;
	}
}


//! Sorts an array with size 'size' using heapsort.
template<class T>
inline void heapsort(T* array_, s32 size)
{
	// for heapsink we pretent this is not c++, where
	// arrays start with index 0. So we decrease the array pointer,
	// the maximum always +2 and the element always +1

	T* virtualArray = array_ - 1;
	s32 virtualSize = size + 2;
	s32 i;

	// build heap

	for (i=((size-1)/2); i>=0; --i)	
		heapsink(virtualArray, i+1, virtualSize-1);

	// sort array

	for (i=size-1; i>=0; --i)	
	{
		T t = array_[0];
		array_[0] = array_[i];
		array_[i] = t;
		heapsink(virtualArray, 1, i + 1);
	}
}

} // end namespace core
} // end namespace irr



#endif

