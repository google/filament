// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and/or irrXML.h

// Need to include Assimp, too. We're using Assimp's version of fast_atof
// so we need stdint.h. But no PCH.


#include "irrXML.h"
#include "irrString.h"
#include "irrArray.h"
#include "./../../code/fast_atof.h"
#include "CXMLReaderImpl.h"

namespace irr
{
namespace io
{

//! Implementation of the file read callback for ordinary files
class CFileReadCallBack : public IFileReadCallBack
{
public:

	//! construct from filename
	CFileReadCallBack(const char* filename)
		: File(0), Size(0), Close(true)
	{
		// open file
		File = fopen(filename, "rb");

		if (File)
			getFileSize();
	}

	//! construct from FILE pointer
	CFileReadCallBack(FILE* file)
		: File(file), Size(0), Close(false)
	{
		if (File)
			getFileSize();
	}

	//! destructor
	virtual ~CFileReadCallBack()
	{
		if (Close && File)
			fclose(File);
	}

	//! Reads an amount of bytes from the file.
	virtual int read(void* buffer, int sizeToRead)
	{
		if (!File)
			return 0;

		return (int)fread(buffer, 1, sizeToRead, File);
	}

	//! Returns size of file in bytes
	virtual int getSize()
	{
		return Size;
	}

private:

	//! retrieves the file size of the open file
	void getFileSize()
	{
		fseek(File, 0, SEEK_END);
		Size = ftell(File);
		fseek(File, 0, SEEK_SET);
	}

	FILE* File;
	int Size;
	bool Close;

}; // end class CFileReadCallBack



// FACTORY FUNCTIONS:


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IrrXMLReader* createIrrXMLReader(const char* filename)
{
	return new CXMLReaderImpl<char, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IrrXMLReader* createIrrXMLReader(FILE* file)
{
	return new CXMLReaderImpl<char, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UFT-8 or ASCII character xml parser. 
IrrXMLReader* createIrrXMLReader(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char, IXMLBase>(callback, false); 
}


//! Creates an instance of an UTF-16 xml parser. 
IrrXMLReaderUTF16* createIrrXMLReaderUTF16(const char* filename)
{
	return new CXMLReaderImpl<char16, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UTF-16 xml parser. 
IrrXMLReaderUTF16* createIrrXMLReaderUTF16(FILE* file)
{
	return new CXMLReaderImpl<char16, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UTF-16 xml parser. 
IrrXMLReaderUTF16* createIrrXMLReaderUTF16(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char16, IXMLBase>(callback, false); 
}


//! Creates an instance of an UTF-32 xml parser. 
IrrXMLReaderUTF32* createIrrXMLReaderUTF32(const char* filename)
{
	return new CXMLReaderImpl<char32, IXMLBase>(new CFileReadCallBack(filename)); 
}


//! Creates an instance of an UTF-32 xml parser. 
IrrXMLReaderUTF32* createIrrXMLReaderUTF32(FILE* file)
{
	return new CXMLReaderImpl<char32, IXMLBase>(new CFileReadCallBack(file)); 
}


//! Creates an instance of an UTF-32 xml parser. 
IrrXMLReaderUTF32* createIrrXMLReaderUTF32(IFileReadCallBack* callback)
{
	return new CXMLReaderImpl<char32, IXMLBase>(callback, false); 
}


} // end namespace io
} // end namespace irr
