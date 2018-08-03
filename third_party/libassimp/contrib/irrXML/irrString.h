// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and irrXML.h

#ifndef __IRR_STRING_H_INCLUDED__
#define __IRR_STRING_H_INCLUDED__

#include "irrTypes.h"

namespace irr
{
namespace core
{

//!	Very simple string class with some useful features.
/**	string<c8> and string<wchar_t> work both with unicode AND ascii,
so you can assign unicode to string<c8> and ascii to string<wchar_t> 
(and the other way round) if your ever would want to. 
Note that the conversation between both is not done using an encoding.

Known bugs:
Special characters like 'Ä', 'Ü' and 'Ö' are ignored in the
methods make_upper, make_lower and equals_ignore_case.
*/
template <class T>
class string
{
public:

	//! Default constructor
	string()
	: array(0), allocated(1), used(1)
	{
		array = new T[1];
		array[0] = 0x0;
	}



	//! Constructor
	string(const string<T>& other)
	: array(0), allocated(0), used(0)
	{
		*this = other;
	}


	//! Constructs a string from an int
	string(int number)
	: array(0), allocated(0), used(0)
	{
		// store if negative and make positive

		bool negative = false;
		if (number < 0)
		{
			number *= -1;
			negative = true;
		}

		// temporary buffer for 16 numbers

		c8 tmpbuf[16];
		tmpbuf[15] = 0;
		s32 idx = 15;	

		// special case '0'

		if (!number) 
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			idx--;	
			tmpbuf[idx] = (c8)('0' + (number % 10));
			number = number / 10;					
		}

		// add sign

		if (negative)
		{
			idx--;
			tmpbuf[idx] = '-';			
		}

		*this = &tmpbuf[idx];
	}



	//! Constructor for copying a string from a pointer with a given lenght
	template <class B>
	string(const B* c, s32 lenght)
	: array(0), allocated(0), used(0)
	{
		if (!c)
			return;

        allocated = used = lenght+1;
		array = new T[used];

		for (s32 l = 0; l<lenght; ++l)
			array[l] = (T)c[l];

		array[lenght] = 0;
	}



	//! Constructor for unicode and ascii strings
	template <class B>
	string(const B* c)
	: array(0),allocated(0), used(0)
	{
		*this = c;
	}



	//! destructor
	~string()
	{
		delete [] array;
	}



	//! Assignment operator
	string<T>& operator=(const string<T>& other) 
	{
		if (this == &other)
			return *this;

		delete [] array;
		allocated = used = other.size()+1;
		array = new T[used];

		const T* p = other.c_str();
		for (s32 i=0; i<used; ++i, ++p)
			array[i] = *p;

		return *this;
	}



	//! Assignment operator for strings, ascii and unicode
	template <class B>
	string<T>& operator=(const B* c) 
	{
		if (!c)
		{
			if (!array)
			{
				array = new T[1];
				allocated = 1;
				used = 1;
			}
			array[0] = 0x0;
			return *this;
		}

		if ((void*)c == (void*)array)
			return *this;

		s32 len = 0;
		const B* p = c;
		while(*p)
		{
			++len;
			++p;
		}

		// we'll take the old string for a while, because the new string could be
		// a part of the current string.
		T* oldArray = array;

        allocated = used = len+1;
		array = new T[used];

		for (s32 l = 0; l<len+1; ++l)
			array[l] = (T)c[l];

		delete [] oldArray;
		return *this;
	}

	//! Add operator for other strings
	string<T> operator+(const string<T>& other) 
	{ 
		string<T> str(*this); 
		str.append(other); 

		return str; 
	} 

	//! Add operator for strings, ascii and unicode 
	template <class B> 
	string<T> operator+(const B* c) 
	{ 
		string<T> str(*this); 
		str.append(c); 

		return str; 
	}



	//! Direct access operator
	T& operator [](const s32 index)  const
	{
		_IRR_DEBUG_BREAK_IF(index>=used) // bad index

		return array[index];
	}


	//! Comparison operator
	bool operator ==(const T* str) const
	{
		int i;
		for(i=0; array[i] && str[i]; ++i)
			if (array[i] != str[i])
				return false;

		return !array[i] && !str[i];
	}



	//! Comparison operator
	bool operator ==(const string<T>& other) const
	{
		for(s32 i=0; array[i] && other.array[i]; ++i)
			if (array[i] != other.array[i])
				return false;

		return used == other.used;
	}



	//! Is smaller operator
	bool operator <(const string<T>& other) const
	{
		for(s32 i=0; array[i] && other.array[i]; ++i)
			if (array[i] != other.array[i])
				return (array[i] < other.array[i]);

		return used < other.used;
	}



	//! Equals not operator
	bool operator !=(const string<T>& other) const
	{
		return !(*this == other);
	}


    
	//! Returns length of string
	/** \return Returns length of the string in characters. */
	s32 size() const
	{
		return used-1;
	}



	//! Returns character string
	/** \return Returns pointer to C-style zero terminated string. */
	const T* c_str() const
	{
		return array;
	}



	//! Makes the string lower case.
	void make_lower()
	{
		const T A = (T)'A';
		const T Z = (T)'Z';
		const T diff = (T)'a' - A;

		for (s32 i=0; i<used; ++i)
		{
			if (array[i]>=A && array[i]<=Z)
				array[i] += diff;
		}
	}



	//! Makes the string upper case.
	void make_upper()
	{
		const T a = (T)'a';
		const T z = (T)'z';
		const T diff = (T)'A' - a;

		for (s32 i=0; i<used; ++i)
		{
			if (array[i]>=a && array[i]<=z)
				array[i] += diff;
		}
	}



	//! Compares the string ignoring case.
	/** \param other: Other string to compare.
	\return Returns true if the string are equal ignoring case. */
	bool equals_ignore_case(const string<T>& other) const
	{
		for(s32 i=0; array[i] && other[i]; ++i)
			if (toLower(array[i]) != toLower(other[i]))
				return false;

		return used == other.used;
	}


	//! compares the first n characters of the strings
	bool equalsn(const string<T>& other, int len)
	{
		int i;
		for(i=0; array[i] && other[i] && i < len; ++i)
			if (array[i] != other[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same lenght
		return (i == len) || (used == other.used);
	}


	//! compares the first n characters of the strings
	bool equalsn(const T* str, int len)
	{
		int i;	
		for(i=0; array[i] && str[i] && i < len; ++i)
			if (array[i] != str[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same lenght
		return (i == len) || (array[i] == 0 && str[i] == 0);
	}


	//! Appends a character to this string
	/** \param character: Character to append. */
	void append(T character)
	{
		if (used + 1 > allocated)
			reallocate((s32)used + 1);

		used += 1;

		array[used-2] = character;
		array[used-1] = 0;
	}

	//! Appends a string to this string
	/** \param other: String to append. */
	void append(const string<T>& other)
	{
		--used;

		s32 len = other.size();
		
		if (used + len + 1 > allocated)
			reallocate((s32)used + (s32)len + 1);

		for (s32 l=0; l<len+1; ++l)
			array[l+used] = other[l];

		used = used + len + 1;
	}


	//! Appends a string of the length l to this string.
	/** \param other: other String to append to this string.
	 \param length: How much characters of the other string to add to this one. */
	void append(const string<T>& other, s32 length)
	{
		s32 len = other.size();

		if (len < length)
		{
			append(other);
			return;
		}

		len = length;
		--used;
		
		if (used + len > allocated)
			reallocate((s32)used + (s32)len);

		for (s32 l=0; l<len; ++l)
			array[l+used] = other[l];

		used = used + len;
	}


	//! Reserves some memory.
	/** \param count: Amount of characters to reserve. */
	void reserve(s32 count)
	{
		if (count < allocated)
			return;

		reallocate(count);
	}


	//! finds first occurrence of character in string
	/** \param c: Character to search for.
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findFirst(T c) const
	{
		for (s32 i=0; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds first occurrence of a character of a list in string
	/** \param c: List of strings to find. For example if the method
	should find the first occurance of 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally, 
	this should be strlen(ofParameter1)
	\return Returns position where one of the character has been found,
	or -1 if not found. */
	s32 findFirstChar(T* c, int count) const
	{
		for (s32 i=0; i<used; ++i)
			for (int j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! Finds first position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	 should find the first occurance of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally, 
	this should be strlen(ofParameter1)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B> 
	s32 findFirstCharNotInList(B* c, int count) const
	{
		for (int i=0; i<used; ++i)
		{
            int j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! Finds last position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	 should find the first occurance of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally, 
	this should be strlen(ofParameter1)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B> 
	s32 findLastCharNotInList(B* c, int count) const
	{
		for (int i=used-2; i>=0; --i)
		{
            int j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! finds next occurrence of character in string
	/** \param c: Character to search for.
	\param startPos: Position in string to start searching. 
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findNext(T c, s32 startPos) const
	{
		for (s32 i=startPos; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}


	//! finds last occurrence of character in string
	//! \param c: Character to search for.
	//! \return Returns position where the character has been found,
	//! or -1 if not found.
	s32 findLast(T c) const
	{
		for (s32 i=used-1; i>=0; --i)
			if (array[i] == c)
				return i;

		return -1;
	}


	//! Returns a substring
	//! \param begin: Start of substring.
	//! \param length: Length of substring.
	string<T> subString(s32 begin, s32 length)
	{
		if (length <= 0)
			return string<T>("");

		string<T> o;
		o.reserve(length+1);

		for (s32 i=0; i<length; ++i)
			o.array[i] = array[i+begin];

		o.array[length] = 0;
		o.used = o.allocated;

		return o;
	}


	void operator += (T c)
	{
		append(c);
	}

	void operator += (const string<T>& other)
	{
		append(other);
	}

	void operator += (int i)
	{
		append(string<T>(i));
	}

	//! replaces all characters of a special type with another one
	void replace(T toReplace, T replaceWith)
	{
		for (s32 i=0; i<used; ++i)
			if (array[i] == toReplace)
				array[i] = replaceWith;
	}

	//! trims the string.
	/** Removes whitespace from begin and end of the string. */
	void trim()
	{
		const char whitespace[] = " \t\n";
		const int whitespacecount = 3;

		// find start and end of real string without whitespace
		int begin = findFirstCharNotInList(whitespace, whitespacecount);
		if (begin == -1)
			return;

		int end = findLastCharNotInList(whitespace, whitespacecount);
		if (end == -1)
			return;

		*this = subString(begin, (end +1) - begin);
	}


	//! Erases a character from the string. May be slow, because all elements 
	//! following after the erased element have to be copied.
	//! \param index: Index of element to be erased.
	void erase(int index)
	{
		_IRR_DEBUG_BREAK_IF(index>=used || index<0) // access violation

		for (int i=index+1; i<used; ++i)
			array[i-1] = array[i];

		--used;
	}

    	

private:

	//! Returns a character converted to lower case
	T toLower(const T& t) const
	{
		if (t>=(T)'A' && t<=(T)'Z')
			return t + ((T)'a' - (T)'A');
		else
			return t;
	}

	//! Reallocate the array, make it bigger or smaler
	void reallocate(s32 new_size)
	{
		T* old_array = array;

		array = new T[new_size];
		allocated = new_size;
		
		s32 amount = used < new_size ? used : new_size;
		for (s32 i=0; i<amount; ++i)
			array[i] = old_array[i];

		if (allocated < used)
			used = allocated;
		
		delete [] old_array;
	}


	//--- member variables

	T* array;
	s32 allocated;
	s32 used;
};


//! Typedef for character strings
typedef string<irr::c8> stringc;

//! Typedef for wide character strings
typedef string<wchar_t> stringw;

} // end namespace core
} // end namespace irr

#endif

