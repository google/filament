// Copyright (C) 2002-2005 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine" and the "irrXML" project.
// For conditions of distribution and use, see copyright notice in irrlicht.h and/or irrXML.h

#ifndef __ICXML_READER_IMPL_H_INCLUDED__
#define __ICXML_READER_IMPL_H_INCLUDED__

#include "irrXML.h"
#include "irrString.h"
#include "irrArray.h"

#include <cassert>
#include <stdlib.h>    
#include <cctype>
#include <cstdint>
//using namespace Assimp;


#ifdef _DEBUG
#define IRR_DEBUGPRINT(x) printf((x));
#else // _DEBUG 
#define IRR_DEBUGPRINT(x)
#endif // _DEBUG


namespace irr
{
namespace io
{


//! implementation of the IrrXMLReader
template<class char_type, class superclass>
class CXMLReaderImpl : public IIrrXMLReader<char_type, superclass>
{
public:

	//! Constructor
	CXMLReaderImpl(IFileReadCallBack* callback, bool deleteCallBack = true)
		: TextData(0), P(0), TextBegin(0), TextSize(0), CurrentNodeType(EXN_NONE),
		SourceFormat(ETF_ASCII), TargetFormat(ETF_ASCII)
	{
		if (!callback)
			return;

		storeTargetFormat();

		// read whole xml file

		readFile(callback);
		
		// clean up

		if (deleteCallBack)
			delete callback;

		// create list with special characters

		createSpecialCharacterList();

		// set pointer to text begin
		P = TextBegin;
	}
    	

	//! Destructor
	virtual ~CXMLReaderImpl()
	{
		delete [] TextData;
	}


	//! Reads forward to the next xml node. 
	//! \return Returns false, if there was no further node. 
	virtual bool read()
	{
		// if not end reached, parse the node
		if (P && (unsigned int)(P - TextBegin) < TextSize - 1 && *P != 0)
		{
			parseCurrentNode();
			return true;
		}

		_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
		return false;
	}


	//! Returns the type of the current XML node.
	virtual EXML_NODE getNodeType() const
	{
		return CurrentNodeType;
	}


	//! Returns attribute count of the current XML node.
	virtual int getAttributeCount() const
	{
		return Attributes.size();
	}


	//! Returns name of an attribute.
	virtual const char_type* getAttributeName(int idx) const
	{
		if (idx < 0 || idx >= (int)Attributes.size())
			return 0;

		return Attributes[idx].Name.c_str();
	}


	//! Returns the value of an attribute. 
	virtual const char_type* getAttributeValue(int idx) const
	{
		if (idx < 0 || idx >= (int)Attributes.size())
			return 0;

		return Attributes[idx].Value.c_str();
	}


	//! Returns the value of an attribute. 
	virtual const char_type* getAttributeValue(const char_type* name) const
	{
		const SAttribute* attr = getAttributeByName(name);
		if (!attr)
			return 0;

		return attr->Value.c_str();
	}


	//! Returns the value of an attribute
	virtual const char_type* getAttributeValueSafe(const char_type* name) const
	{
		const SAttribute* attr = getAttributeByName(name);
		if (!attr)
			return EmptyString.c_str();

		return attr->Value.c_str();
	}



	//! Returns the value of an attribute as integer. 
	int getAttributeValueAsInt(const char_type* name) const
	{
		return (int)getAttributeValueAsFloat(name);
	}


	//! Returns the value of an attribute as integer. 
	int getAttributeValueAsInt(int idx) const
	{
		return (int)getAttributeValueAsFloat(idx);
	}


	//! Returns the value of an attribute as float. 
	float getAttributeValueAsFloat(const char_type* name) const
	{
		const SAttribute* attr = getAttributeByName(name);
		if (!attr)
			return 0;

		core::stringc c = attr->Value.c_str();
        return static_cast<float>(atof(c.c_str()));
        //return fast_atof(c.c_str());
	}


	//! Returns the value of an attribute as float. 
	float getAttributeValueAsFloat(int idx) const
	{
		const char_type* attrvalue = getAttributeValue(idx);
		if (!attrvalue)
			return 0;

		core::stringc c = attrvalue;
        return static_cast<float>(atof(c.c_str()));
		//return fast_atof(c.c_str());
	}


	//! Returns the name of the current node.
	virtual const char_type* getNodeName() const
	{
		return NodeName.c_str();
	}


	//! Returns data of the current node.
	virtual const char_type* getNodeData() const
	{
		return NodeName.c_str();
	}


	//! Returns if an element is an empty element, like <foo />
	virtual bool isEmptyElement() const
	{
		return IsEmptyElement;
	}

	//! Returns format of the source xml file.
	virtual ETEXT_FORMAT getSourceFormat() const
	{
		return SourceFormat;
	}

	//! Returns format of the strings returned by the parser.
	virtual ETEXT_FORMAT getParserFormat() const
	{
		return TargetFormat;
	}

private:

	// Reads the current xml node
	void parseCurrentNode()
	{
		char_type* start = P;

		// move forward until '<' found
		while(*P != L'<' && *P)
			++P;

		if (!*P)
			return;

		if (P - start > 0)
		{
			// we found some text, store it
			if (setText(start, P))
				return;
		}

		++P;

		// based on current token, parse and report next element
		switch(*P)
		{
		case L'/':
			parseClosingXMLElement(); 
			break;
		case L'?':
			ignoreDefinition();	
			break;
		case L'!':
			if (!parseCDATA())
				parseComment();	
			break;
		default:
			parseOpeningXMLElement();
			break;
		}
	}


	//! sets the state that text was found. Returns true if set should be set
	bool setText(char_type* start, char_type* end)
	{
		// check if text is more than 2 characters, and if not, check if there is 
		// only white space, so that this text won't be reported
		if (end - start < 3)
		{
			char_type* p = start;
			for(; p != end; ++p)
				if (!isWhiteSpace(*p))
					break;

			if (p == end)
				return false;
		}

		// set current text to the parsed text, and replace xml special characters
		core::string<char_type> s(start, (int)(end - start));
		NodeName = replaceSpecialCharacters(s);

		// current XML node type is text
		CurrentNodeType = EXN_TEXT;

		return true;
	}



	//! ignores an xml definition like <?xml something />
	void ignoreDefinition()
	{
		CurrentNodeType = EXN_UNKNOWN;

		// move until end marked with '>' reached
		while(*P != L'>')
			++P;

		++P;
	}


	//! parses a comment
	void parseComment()
	{
		CurrentNodeType = EXN_COMMENT;
		P += 1;

		char_type *pCommentBegin = P;

		int count = 1;

		// move until end of comment reached
		while(count)
		{
			if (*P == L'>')
				--count;
			else
			if (*P == L'<')
				++count;

			++P;
		}

		P -= 3;
		NodeName = core::string<char_type>(pCommentBegin+2, (int)(P - pCommentBegin-2));
		P += 3;
	}


	//! parses an opening xml element and reads attributes
	void parseOpeningXMLElement()
	{
		CurrentNodeType = EXN_ELEMENT;
		IsEmptyElement = false;
		Attributes.clear();

		// find name
		const char_type* startName = P;

		// find end of element
		while(*P != L'>' && !isWhiteSpace(*P))
			++P;

		const char_type* endName = P;

		// find Attributes
		while(*P != L'>')
		{
			if (isWhiteSpace(*P))
				++P;
			else
			{
				if (*P != L'/')
				{
					// we've got an attribute

					// read the attribute names
					const char_type* attributeNameBegin = P;

					while(!isWhiteSpace(*P) && *P != L'=')
						++P;

					const char_type* attributeNameEnd = P;
					++P;

					// read the attribute value
					// check for quotes and single quotes, thx to murphy
					while( (*P != L'\"') && (*P != L'\'') && *P) 
						++P;

					if (!*P) // malformatted xml file
						return;

					const char_type attributeQuoteChar = *P;

					++P;
					const char_type* attributeValueBegin = P;
					
					while(*P != attributeQuoteChar && *P)
						++P;

					if (!*P) // malformatted xml file
						return;

					const char_type* attributeValueEnd = P;
					++P;

					SAttribute attr;
					attr.Name = core::string<char_type>(attributeNameBegin, 
						(int)(attributeNameEnd - attributeNameBegin));

					core::string<char_type> s(attributeValueBegin, 
						(int)(attributeValueEnd - attributeValueBegin));

					attr.Value = replaceSpecialCharacters(s);
					Attributes.push_back(attr);
				}
				else
				{
					// tag is closed directly
					++P;
					IsEmptyElement = true;
					break;
				}
			}
		}

		// check if this tag is closing directly
		if (endName > startName && *(endName-1) == L'/')
		{
			// directly closing tag
			IsEmptyElement = true;
			endName--;
		}
		
		NodeName = core::string<char_type>(startName, (int)(endName - startName));

		++P;
	}


	//! parses an closing xml tag
	void parseClosingXMLElement()
	{
		CurrentNodeType = EXN_ELEMENT_END;
		IsEmptyElement = false;
		Attributes.clear();

		++P;
		const char_type* pBeginClose = P;

		while(*P != L'>')
			++P;

    // remove trailing whitespace, if any
    while( std::isspace( P[-1]))
      --P;

		NodeName = core::string<char_type>(pBeginClose, (int)(P - pBeginClose));
		++P;
	}

	//! parses a possible CDATA section, returns false if begin was not a CDATA section
	bool parseCDATA()
	{
		if (*(P+1) != L'[')
			return false;

		CurrentNodeType = EXN_CDATA;

		// skip '<![CDATA['
		int count=0;
		while( *P && count<8 )
		{
			++P;
			++count;
		}

		if (!*P)
			return true;

		char_type *cDataBegin = P;
		char_type *cDataEnd = 0;

		// find end of CDATA
		while(*P && !cDataEnd)
		{
			if (*P == L'>' && 
			   (*(P-1) == L']') &&
			   (*(P-2) == L']'))
			{
				cDataEnd = P - 2;
			}

			++P;
		}

		if ( cDataEnd )
			NodeName = core::string<char_type>(cDataBegin, (int)(cDataEnd - cDataBegin));
		else
			NodeName = "";

		return true;
	}


	// structure for storing attribute-name pairs
	struct SAttribute
	{
		core::string<char_type> Name;
		core::string<char_type> Value;
	};

	// finds a current attribute by name, returns 0 if not found
	const SAttribute* getAttributeByName(const char_type* name) const
	{
		if (!name)
			return 0;

		core::string<char_type> n = name;

		for (int i=0; i<(int)Attributes.size(); ++i)
			if (Attributes[i].Name == n)
				return &Attributes[i];

		return 0;
	}

	// replaces xml special characters in a string and creates a new one
	core::string<char_type> replaceSpecialCharacters(
		core::string<char_type>& origstr)
	{
		int pos = origstr.findFirst(L'&');
		int oldPos = 0;

		if (pos == -1)
			return origstr;

		core::string<char_type> newstr;

		while(pos != -1 && pos < origstr.size()-2)
		{
			// check if it is one of the special characters

			int specialChar = -1;
			for (int i=0; i<(int)SpecialCharacters.size(); ++i)
			{
				const char_type* p = &origstr.c_str()[pos]+1;

				if (equalsn(&SpecialCharacters[i][1], p, SpecialCharacters[i].size()-1))
				{
					specialChar = i;
					break;
				}
			}

			if (specialChar != -1)
			{
				newstr.append(origstr.subString(oldPos, pos - oldPos));
				newstr.append(SpecialCharacters[specialChar][0]);
				pos += SpecialCharacters[specialChar].size();
			}
			else
			{
				newstr.append(origstr.subString(oldPos, pos - oldPos + 1));
				pos += 1;
			}

			// find next &
			oldPos = pos;
			pos = origstr.findNext(L'&', pos);		
		}

		if (oldPos < origstr.size()-1)
			newstr.append(origstr.subString(oldPos, origstr.size()-oldPos));

		return newstr;
	}



	//! reads the xml file and converts it into the wanted character format.
	bool readFile(IFileReadCallBack* callback)
	{
		int size = callback->getSize();		
		size += 4; // We need two terminating 0's at the end.
		           // For ASCII we need 1 0's, for UTF-16 2, for UTF-32 4.

		char* data8 = new char[size];

		if (!callback->read(data8, size-4))
		{
			delete [] data8;
			return false;
		}

		// add zeros at end

		data8[size-1] = 0;
		data8[size-2] = 0;
		data8[size-3] = 0;
		data8[size-4] = 0;

		char16* data16 = reinterpret_cast<char16*>(data8);
		char32* data32 = reinterpret_cast<char32*>(data8);	

		// now we need to convert the data to the desired target format
		// based on the byte order mark.

		const unsigned char UTF8[] = {0xEF, 0xBB, 0xBF}; // 0xEFBBBF;
		const int UTF16_BE = 0xFFFE;
		const int UTF16_LE = 0xFEFF;
		const int UTF32_BE = 0xFFFE0000;
		const int UTF32_LE = 0x0000FEFF;

		// check source for all utf versions and convert to target data format
		
		if (size >= 4 && data32[0] == (char32)UTF32_BE)
		{
			// UTF-32, big endian
			SourceFormat = ETF_UTF32_BE;
			convertTextData(data32+1, data8, (size/4)); // data32+1 because we need to skip the header
		}
		else
		if (size >= 4 && data32[0] == (char32)UTF32_LE)
		{
			// UTF-32, little endian
			SourceFormat = ETF_UTF32_LE;
			convertTextData(data32+1, data8, (size/4)); // data32+1 because we need to skip the header
		}
		else
		if (size >= 2 && data16[0] == UTF16_BE)
		{
			// UTF-16, big endian
			SourceFormat = ETF_UTF16_BE;
			convertTextData(data16+1, data8, (size/2)); // data16+1 because we need to skip the header
		}
		else
		if (size >= 2 && data16[0] == UTF16_LE)
		{
			// UTF-16, little endian
			SourceFormat = ETF_UTF16_LE;
			convertTextData(data16+1, data8, (size/2)); // data16+1 because we need to skip the header
		}
		else
		if (size >= 3 && data8[0] == UTF8[0] && data8[1] == UTF8[1] && data8[2] == UTF8[2])
		{
			// UTF-8
			SourceFormat = ETF_UTF8;
			convertTextData(data8+3, data8, size); // data8+3 because we need to skip the header
		}
		else
		{
			// ASCII
			SourceFormat = ETF_ASCII;
			convertTextData(data8, data8, size);
		}

		return true;
	}


	//! converts the text file into the desired format.
	//! \param source: begin of the text (without byte order mark)
	//! \param pointerToStore: pointer to text data block which can be
	//! stored or deleted based on the nesessary conversion.
	//! \param sizeWithoutHeader: Text size in characters without header
	template<class src_char_type>
	void convertTextData(src_char_type* source, char* pointerToStore, int sizeWithoutHeader)
	{
		// convert little to big endian if necessary
		if (sizeof(src_char_type) > 1 && 
			isLittleEndian(TargetFormat) != isLittleEndian(SourceFormat))
			convertToLittleEndian(source);

		// check if conversion is necessary:
		if (sizeof(src_char_type) == sizeof(char_type))
		{
			// no need to convert
			TextBegin = (char_type*)source;
			TextData = (char_type*)pointerToStore;
			TextSize = sizeWithoutHeader;
		}
		else
		{
			// convert source into target data format. 
			// TODO: implement a real conversion. This one just 
			// copies bytes. This is a problem when there are 
			// unicode symbols using more than one character.

			TextData = new char_type[sizeWithoutHeader];

			// MSVC debugger complains here about loss of data ...
			size_t numShift = sizeof( char_type) * 8;
			assert(numShift < 64);
			const src_char_type cc = (src_char_type)(((uint64_t(1u) << numShift) - 1));
			for (int i=0; i<sizeWithoutHeader; ++i)
				TextData[i] = char_type( source[i] & cc); 

			TextBegin = TextData;
			TextSize = sizeWithoutHeader;

			// delete original data because no longer needed
			delete [] pointerToStore;
		}
	}

	//! converts whole text buffer to little endian
	template<class src_char_type>
	void convertToLittleEndian(src_char_type* t)
	{
		if (sizeof(src_char_type) == 4) 
		{
			// 32 bit

			while(*t)
			{
				*t = ((*t & 0xff000000) >> 24) |
				     ((*t & 0x00ff0000) >> 8)  |
				     ((*t & 0x0000ff00) << 8)  |
				     ((*t & 0x000000ff) << 24);
				++t;
			}
		}
		else
		{
			// 16 bit 

			while(*t)
			{
				*t = (*t >> 8) | (*t << 8);
				++t;
			}
		}
	}

	//! returns if a format is little endian
	inline bool isLittleEndian(ETEXT_FORMAT f)
	{
		return f == ETF_ASCII ||
		       f == ETF_UTF8 ||
		       f == ETF_UTF16_LE ||
		       f == ETF_UTF32_LE;
	}


	//! returns true if a character is whitespace
	inline bool isWhiteSpace(char_type c)
	{
		return (c==' ' || c=='\t' || c=='\n' || c=='\r');
	}


	//! generates a list with xml special characters
	void createSpecialCharacterList()
	{
		// list of strings containing special symbols, 
		// the first character is the special character,
		// the following is the symbol string without trailing &.

		SpecialCharacters.push_back("&amp;");
		SpecialCharacters.push_back("<lt;");
		SpecialCharacters.push_back(">gt;");
		SpecialCharacters.push_back("\"quot;");
		SpecialCharacters.push_back("'apos;");
		
	}


	//! compares the first n characters of the strings
	bool equalsn(const char_type* str1, const char_type* str2, int len)
	{
		int i;
		for(i=0; str1[i] && str2[i] && i < len; ++i)
			if (str1[i] != str2[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same lenght
		return (i == len) || (str1[i] == 0 && str2[i] == 0);
	}


	//! stores the target text format
	void storeTargetFormat()
	{
		// get target format. We could have done this using template specialization,
		// but VisualStudio 6 don't like it and we want to support it.

		switch(sizeof(char_type))
		{
		case 1: 
			TargetFormat = ETF_UTF8;
			break;
		case 2: 
			TargetFormat = ETF_UTF16_LE;
			break;
		case 4: 
			TargetFormat = ETF_UTF32_LE;
			break;
		default:
			TargetFormat = ETF_ASCII; // should never happen.
		}
	}


	// instance variables:

	char_type* TextData;         // data block of the text file
	char_type* P;                // current point in text to parse
	char_type* TextBegin;        // start of text to parse
	unsigned int TextSize;       // size of text to parse in characters, not bytes

	EXML_NODE CurrentNodeType;   // type of the currently parsed node
	ETEXT_FORMAT SourceFormat;   // source format of the xml file
	ETEXT_FORMAT TargetFormat;   // output format of this parser

	core::string<char_type> NodeName;    // name of the node currently in
	core::string<char_type> EmptyString; // empty string to be returned by getSafe() methods

	bool IsEmptyElement;       // is the currently parsed node empty?

	core::array< core::string<char_type> > SpecialCharacters; // see createSpecialCharacterList()

	core::array<SAttribute> Attributes; // attributes of current element
	
}; // end CXMLReaderImpl


} // end namespace
} // end namespace

#endif
