// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and/or irrXML.h

#ifndef __IRR_XML_H_INCLUDED__
#define __IRR_XML_H_INCLUDED__

#include <stdio.h>

/** \mainpage irrXML 1.2 API documentation
 <div align="center"><img src="logobig.png" ></div>

 \section intro Introduction

  Welcome to the irrXML API documentation.
  Here you'll find any information you'll need to develop applications with
  irrXML. If you look for a tutorial on how to start, take a look at the \ref irrxmlexample,
  at the homepage of irrXML at <A HREF="http://xml.irrlicht3d.org" >xml.irrlicht3d.org</A> 
  or into the SDK in the directory \example.
 
  irrXML is intended to be a high speed and easy-to-use XML Parser for C++, and
  this documentation is an important part of it. If you have any questions or
  suggestions, just send a email to the author of the engine, Nikolaus Gebhardt
  (niko (at) irrlicht3d.org). For more informations about this parser, see \ref history.

  \section features Features

  irrXML provides forward-only, read-only 
     access to a stream of non validated XML data. It was fully implemented by
	 Nikolaus Gebhardt. Its current features are:

	 - It it fast as lighting and has very low memory usage. It was 
	   developed with the intention of being used in 3D games, as it already has been.
	 - irrXML is very small: It only consists of 60 KB of code and can be added easily
	   to your existing project.
	 - Of course, it is platform independent and works with lots of compilers.
	 - It is able to parse ASCII, UTF-8, UTF-16 and UTF-32 text files, both in 
	   little and big endian format. 
	 - Independent of the input file format, the parser can return all strings in ASCII, UTF-8,
	   UTF-16 and UTF-32 format. 
	 - With its optional file access abstraction it has the advantage that it can read not
	   only from files but from any type of data (memory, network, ...). For example when 
	   used with the Irrlicht Engine, it directly reads from compressed .zip files. 
	 - Just like the Irrlicht Engine for which it was originally created, it is extremely easy 
	   to use.
	 - It has no external dependencies, it does not even need the STL. 

	 Although irrXML has some strenghts, it currently also has the following limitations:

	 - The input xml file is not validated and assumed to be correct. 

    \section irrxmlexample Example

    The following code demonstrates the basic usage of irrXML. A simple xml
	file like this is parsed:
    \code
	<?xml version="1.0"?>
	<config>
		<!-- This is a config file for the mesh viewer -->
		<model file="dwarf.dea" />
		<messageText caption="Irrlicht Engine Mesh Viewer">
		Welcome to the Mesh Viewer of the &quot;Irrlicht Engine&quot;.
		</messageText>
	</config>
	\endcode

	The code for parsing this file would look like this:
	\code
	#include <irrXML.h>
	using namespace irr; // irrXML is located in the namespace irr::io
	using namespace io;

	#include <string> // we use STL strings to store data in this example

	void main()
	{
		// create the reader using one of the factory functions

		IrrXMLReader* xml = createIrrXMLReader("config.xml");

		// strings for storing the data we want to get out of the file
		std::string modelFile;
		std::string messageText;
		std::string caption;

		// parse the file until end reached

		while(xml && xml->read())
		{
			switch(xml->getNodeType())
			{
			case EXN_TEXT:
				// in this xml file, the only text which occurs is the messageText
				messageText = xml->getNodeData();
				break;
			case EXN_ELEMENT:
				{
					if (!strcmp("model", xml->getNodeName()))
						modelFile = xml->getAttributeValue("file");
					else
					if (!strcmp("messageText", xml->getNodeName()))
						caption = xml->getAttributeValue("caption");
				}
				break;
			}
		}

		// delete the xml parser after usage
		delete xml;
	}
	\endcode

	\section howto How to use

	Simply add the source files in the /src directory of irrXML to your project. Done.

	\section license License

	The irrXML license is based on the zlib license. Basicly, this means you can do with
	irrXML whatever you want:

	Copyright (C) 2002-2005 Nikolaus Gebhardt

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.

	\section history History

	As lots of references in this documentation and the source show, this xml 
	parser has originally been a part of the 
	<A HREF="http://irrlicht.sourceforge.net" >Irrlicht Engine</A>. But because
	the parser has become very useful with the latest release, people asked for a 
	separate version of it, to be able to use it in non Irrlicht projects. With
	irrXML 1.0, this has now been done.
*/

namespace irr
{
namespace io
{
	//! Enumeration of all supported source text file formats 
	enum ETEXT_FORMAT
	{
		//! ASCII, file without byte order mark, or not a text file
		ETF_ASCII,

		//! UTF-8 format
		ETF_UTF8,

		//! UTF-16 format, big endian
		ETF_UTF16_BE,

		//! UTF-16 format, little endian
		ETF_UTF16_LE,

		//! UTF-32 format, big endian
		ETF_UTF32_BE,

		//! UTF-32 format, little endian
		ETF_UTF32_LE
	};


	//! Enumeration for all xml nodes which are parsed by IrrXMLReader
	enum EXML_NODE
	{
		//! No xml node. This is usually the node if you did not read anything yet.
		EXN_NONE,

		//! A xml element, like <foo>
		EXN_ELEMENT,

		//! End of an xml element, like </foo>
		EXN_ELEMENT_END,

		//! Text within a xml element: <foo> this is the text. </foo>
		EXN_TEXT,

		//! An xml comment like &lt;!-- I am a comment --&gt; or a DTD definition.
		EXN_COMMENT,

		//! An xml cdata section like &lt;![CDATA[ this is some CDATA ]]&gt;
		EXN_CDATA,

		//! Unknown element.
		EXN_UNKNOWN
	};

	//! Callback class for file read abstraction. 
	/** With this, it is possible to make the xml parser read in other things 
	than just files. The Irrlicht engine is using this for example to 
	read xml from compressed .zip files. To make the parser read in 
	any other data, derive a class from this interface, implement the 
	two methods to read your data and give a pointer to an instance of
	your implementation when calling createIrrXMLReader(), 
	createIrrXMLReaderUTF16() or createIrrXMLReaderUTF32() */
	class IFileReadCallBack
	{
	public:

		//! virtual destructor
		virtual ~IFileReadCallBack() {};

		//! Reads an amount of bytes from the file.
		/** \param buffer: Pointer to buffer where to read bytes will be written to.
		\param sizeToRead: Amount of bytes to read from the file.
		\return Returns how much bytes were read. */
		virtual int read(void* buffer, int sizeToRead) = 0;

		//! Returns size of file in bytes
		virtual int getSize() = 0;
	};

	//! Empty class to be used as parent class for IrrXMLReader.
	/** If you need another class as base class for the xml reader, you can do this by creating
	the reader using for example new CXMLReaderImpl<char, YourBaseClass>(yourcallback);
	The Irrlicht Engine for example needs IUnknown as base class for every object to
	let it automaticly reference countend, hence it replaces IXMLBase with IUnknown.
	See irrXML.cpp on how this can be done in detail. */
	class IXMLBase
	{
	};	

	//! Interface providing easy read access to a XML file.
	/** You can create an instance of this reader using one of the factory functions
	createIrrXMLReader(), createIrrXMLReaderUTF16() and createIrrXMLReaderUTF32().
	If using the parser from the Irrlicht Engine, please use IFileSystem::createXMLReader() 
	instead.
	For a detailed intro how to use the parser, see \ref irrxmlexample and \ref features.

	The typical usage of this parser looks like this:
	\code
	#include <irrXML.h>
	using namespace irr; // irrXML is located in the namespace irr::io
	using namespace io;

	void main()
	{
		// create the reader using one of the factory functions
		IrrXMLReader* xml = createIrrXMLReader("config.xml");

		if (xml == 0)
			return; // file could not be opened

		// parse the file until end reached
		while(xml->read())
		{
			// based on xml->getNodeType(), do something.
		}

		// delete the xml parser after usage
		delete xml;
	}
	\endcode
	See \ref irrxmlexample for a more detailed example.
	*/
	template<class char_type, class super_class>
	class IIrrXMLReader : public super_class
	{
	public:

		//! Destructor
		virtual ~IIrrXMLReader() {};

		//! Reads forward to the next xml node. 
		/** \return Returns false, if there was no further node.  */
		virtual bool read() = 0;

		//! Returns the type of the current XML node.
		virtual EXML_NODE getNodeType() const = 0;

        //! Returns attribute count of the current XML node. 
		/** This is usually
		non null if the current node is EXN_ELEMENT, and the element has attributes.
		\return Returns amount of attributes of this xml node. */
		virtual int getAttributeCount() const = 0;

		//! Returns name of an attribute. 
		/** \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		\return Name of the attribute, 0 if an attribute with this index does not exist. */
		virtual const char_type* getAttributeName(int idx) const = 0;

		//! Returns the value of an attribute. 
		/** \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		\return Value of the attribute, 0 if an attribute with this index does not exist. */
		virtual const char_type* getAttributeValue(int idx) const = 0;

		//! Returns the value of an attribute. 
		/** \param name: Name of the attribute.
		\return Value of the attribute, 0 if an attribute with this name does not exist. */
		virtual const char_type* getAttributeValue(const char_type* name) const = 0;

		//! Returns the value of an attribute in a safe way.
		/** Like getAttributeValue(), but does not 
		return 0 if the attribute does not exist. An empty string ("") is returned then.
		\param name: Name of the attribute.
		\return Value of the attribute, and "" if an attribute with this name does not exist */
		virtual const char_type* getAttributeValueSafe(const char_type* name) const = 0;

		//! Returns the value of an attribute as integer. 
		/** \param name Name of the attribute.
		\return Value of the attribute as integer, and 0 if an attribute with this name does not exist or
		the value could not be interpreted as integer. */
		virtual int getAttributeValueAsInt(const char_type* name) const = 0;

		//! Returns the value of an attribute as integer. 
		/** \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		\return Value of the attribute as integer, and 0 if an attribute with this index does not exist or
		the value could not be interpreted as integer. */
		virtual int getAttributeValueAsInt(int idx) const = 0;

		//! Returns the value of an attribute as float. 
		/** \param name: Name of the attribute.
		\return Value of the attribute as float, and 0 if an attribute with this name does not exist or
		the value could not be interpreted as float. */
		virtual float getAttributeValueAsFloat(const char_type* name) const = 0;

		//! Returns the value of an attribute as float. 
		/** \param idx: Zero based index, should be something between 0 and getAttributeCount()-1.
		\return Value of the attribute as float, and 0 if an attribute with this index does not exist or
		the value could not be interpreted as float. */
		virtual float getAttributeValueAsFloat(int idx) const = 0;

		//! Returns the name of the current node. 
		/** Only non null, if the node type is EXN_ELEMENT.
		\return Name of the current node or 0 if the node has no name. */
		virtual const char_type* getNodeName() const = 0;

		//! Returns data of the current node. 
		/** Only non null if the node has some
		data and it is of type EXN_TEXT or EXN_UNKNOWN. */
		virtual const char_type* getNodeData() const = 0;

		//! Returns if an element is an empty element, like <foo />
		virtual bool isEmptyElement() const = 0;

		//! Returns format of the source xml file. 
		/** It is not necessary to use
		this method because the parser will convert the input file format
		to the format wanted by the user when creating the parser. This
		method is useful to get/display additional informations. */
		virtual ETEXT_FORMAT getSourceFormat() const = 0;

		//! Returns format of the strings returned by the parser. 
		/** This will be UTF8 for example when you created a parser with
		IrrXMLReaderUTF8() and UTF32 when it has been created using 
		IrrXMLReaderUTF32. It should not be necessary to call this
		method and only exists for informational purposes. */
		virtual ETEXT_FORMAT getParserFormat() const = 0;
	};


	//! defines the utf-16 type.
	/** Not using wchar_t for this because 
	wchar_t has 16 bit on windows and 32 bit on other operating systems. */
	typedef unsigned short char16;

	//! defines the utf-32 type. 
	/** Not using wchar_t for this because 
	wchar_t has 16 bit on windows and 32 bit on other operating systems. */
	typedef unsigned long char32;

	//! A UTF-8 or ASCII character xml parser.
	/** This means that all character data will be returned in 8 bit ASCII or UTF-8 by this parser. 
	The file to read can be in any format, it will be converted to UTF-8 if it is not
	in this format.
	Create an instance of this with createIrrXMLReader(); 
	See IIrrXMLReader for description on how to use it. */
	typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader;

	//! A UTF-16 xml parser. 
	/** This means that all character data will be returned in UTF-16 by this parser. 
	The file to read can be in any format, it will be converted to UTF-16 if it is not
	in this format.
	Create an instance of this with createIrrXMLReaderUTF16(); 
	See IIrrXMLReader for description on how to use it.  */
	typedef IIrrXMLReader<char16, IXMLBase> IrrXMLReaderUTF16;

	//! A UTF-32 xml parser. 
	/** This means that all character data will be returned in UTF-32 by this parser. 
	The file to read can be in any format, it will be converted to UTF-32 if it is not
	in this format.
	Create an instance of this with createIrrXMLReaderUTF32(); 
	See IIrrXMLReader for description on how to use it. */
	typedef IIrrXMLReader<char32, IXMLBase> IrrXMLReaderUTF32;


	//! Creates an instance of an UFT-8 or ASCII character xml parser.
	/** This means that all character data will be returned in 8 bit ASCII or UTF-8. 
	The file to read can be in any format, it will be converted to UTF-8 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReaderUTF8() instead.
	\param filename: Name of file to be opened.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReader* createIrrXMLReader(const char* filename);

	//! Creates an instance of an UFT-8 or ASCII character xml parser.
	/** This means that all character data will be returned in 8 bit ASCII or UTF-8. The file to read can 
	be in any format, it will be converted to UTF-8 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReaderUTF8() instead.
	\param file: Pointer to opened file, must have been opened in binary mode, e.g.
	using fopen("foo.bar", "wb"); The file will not be closed after it has been read.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReader* createIrrXMLReader(FILE* file);

	//! Creates an instance of an UFT-8 or ASCII character xml parser. 
	/** This means that all character data will be returned in 8 bit ASCII or UTF-8. The file to read can 
	 be in any format, it will be converted to UTF-8 if it is not in this format.
	 If you are using the Irrlicht Engine, it is better not to use this function but
	 IFileSystem::createXMLReaderUTF8() instead.
	 \param callback: Callback for file read abstraction. Implement your own
	 callback to make the xml parser read in other things than just files. See
	 IFileReadCallBack for more information about this.
	 \return Returns a pointer to the created xml parser. This pointer should be 
	 deleted using 'delete' after no longer needed. Returns 0 if an error occured
	 and the file could not be opened. */
	IrrXMLReader* createIrrXMLReader(IFileReadCallBack* callback);

	//! Creates an instance of an UFT-16 xml parser. 
	/** This means that
	all character data will be returned in UTF-16. The file to read can 
	be in any format, it will be converted to UTF-16 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param filename: Name of file to be opened.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF16* createIrrXMLReaderUTF16(const char* filename);

	//! Creates an instance of an UFT-16 xml parser. 
	/** This means that all character data will be returned in UTF-16. The file to read can 
	be in any format, it will be converted to UTF-16 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param file: Pointer to opened file, must have been opened in binary mode, e.g.
	using fopen("foo.bar", "wb"); The file will not be closed after it has been read.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF16* createIrrXMLReaderUTF16(FILE* file);

	//! Creates an instance of an UFT-16 xml parser. 
	/** This means that all character data will be returned in UTF-16. The file to read can 
	be in any format, it will be converted to UTF-16 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param callback: Callback for file read abstraction. Implement your own
	callback to make the xml parser read in other things than just files. See
	IFileReadCallBack for more information about this.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF16* createIrrXMLReaderUTF16(IFileReadCallBack* callback);


	//! Creates an instance of an UFT-32 xml parser. 
	/** This means that all character data will be returned in UTF-32. The file to read can 
	be in any format, it will be converted to UTF-32 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param filename: Name of file to be opened.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF32* createIrrXMLReaderUTF32(const char* filename);

	//! Creates an instance of an UFT-32 xml parser. 
	/** This means that all character data will be returned in UTF-32. The file to read can 
	be in any format, it will be converted to UTF-32 if it is not in this format.
	if you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param file: Pointer to opened file, must have been opened in binary mode, e.g.
	using fopen("foo.bar", "wb"); The file will not be closed after it has been read.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF32* createIrrXMLReaderUTF32(FILE* file);

	//! Creates an instance of an UFT-32 xml parser. 
	/** This means that
	all character data will be returned in UTF-32. The file to read can 
	be in any format, it will be converted to UTF-32 if it is not in this format.
	If you are using the Irrlicht Engine, it is better not to use this function but
	IFileSystem::createXMLReader() instead.
	\param callback: Callback for file read abstraction. Implement your own
	callback to make the xml parser read in other things than just files. See
	IFileReadCallBack for more information about this.
	\return Returns a pointer to the created xml parser. This pointer should be 
	deleted using 'delete' after no longer needed. Returns 0 if an error occured
	and the file could not be opened. */
	IrrXMLReaderUTF32* createIrrXMLReaderUTF32(IFileReadCallBack* callback);
	

	/*! \file irrxml.h
    \brief Header file of the irrXML, the Irrlicht XML parser.
    
    This file includes everything needed for using irrXML, 
    the XML parser of the Irrlicht Engine. To use irrXML,
	you only need to include this file in your project:

	\code
	#include <irrXML.h>
	\endcode

	It is also common to use the two namespaces in which irrXML is included, 
	directly after #including irrXML.h:

	\code
	#include <irrXML.h>
	using namespace irr;
	using namespace io;
	\endcode
	*/

} // end namespace io
} // end namespace irr

#endif // __IRR_XML_H_INCLUDED__

