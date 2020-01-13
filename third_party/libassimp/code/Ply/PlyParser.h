/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file Defines the helper data structures for importing PLY files  */
#pragma once
#ifndef AI_PLYFILEHELPER_H_INC
#define AI_PLYFILEHELPER_H_INC

#include <assimp/ParsingUtils.h>
#include <assimp/IOStreamBuffer.h>
#include <vector>

namespace Assimp
{

//pre-declaration
class PLYImporter;

// http://local.wasp.uwa.edu.au/~pbourke/dataformats/ply/
// http://w3.impa.br/~lvelho/outgoing/sossai/old/ViHAP_D4.4.2_PLY_format_v1.1.pdf
// http://www.okino.com/conv/exp_ply.htm
namespace PLY {

// ---------------------------------------------------------------------------------
/*
name        type        number of bytes
---------------------------------------
char       character                 1
uchar      unsigned character        1
short      short integer             2
ushort     unsigned short integer    2
int        integer                   4
uint       unsigned integer          4
float      single-precision float    4
double     double-precision float    8

int8
int16
uint8 ... forms are also used
*/
enum EDataType {
    EDT_Char = 0x0u,
    EDT_UChar,
    EDT_Short,
    EDT_UShort,
    EDT_Int,
    EDT_UInt,
    EDT_Float,
    EDT_Double,

    // Marks invalid entries
    EDT_INVALID
};

// ---------------------------------------------------------------------------------
/** \brief Specifies semantics for PLY element properties
 *
 * Semantics define the usage of a property, e.g. x coordinate
*/
enum ESemantic {
    //! vertex position x coordinate
    EST_XCoord = 0x0u,
    //! vertex position x coordinate
    EST_YCoord,
    //! vertex position x coordinate
    EST_ZCoord,

    //! vertex normal x coordinate
    EST_XNormal,
    //! vertex normal y coordinate
    EST_YNormal,
    //! vertex normal z coordinate
    EST_ZNormal,

    //! u texture coordinate
    EST_UTextureCoord,
    //! v texture coordinate
    EST_VTextureCoord,

    //! vertex colors, red channel
    EST_Red,
    //! vertex colors, green channel
    EST_Green,
    //! vertex colors, blue channel
    EST_Blue,
    //! vertex colors, alpha channel
    EST_Alpha,

    //! vertex index list
    EST_VertexIndex,

    //! texture index
    EST_TextureIndex,

    //! texture coordinates (stored as element of a face)
    EST_TextureCoordinates,

    //! material index
    EST_MaterialIndex,

    //! ambient color, red channel
    EST_AmbientRed,
    //! ambient color, green channel
    EST_AmbientGreen,
    //! ambient color, blue channel
    EST_AmbientBlue,
    //! ambient color, alpha channel
    EST_AmbientAlpha,

    //! diffuse color, red channel
    EST_DiffuseRed,
    //! diffuse color, green channel
    EST_DiffuseGreen,
    //! diffuse color, blue channel
    EST_DiffuseBlue,
    //! diffuse color, alpha channel
    EST_DiffuseAlpha,

    //! specular color, red channel
    EST_SpecularRed,
    //! specular color, green channel
    EST_SpecularGreen,
    //! specular color, blue channel
    EST_SpecularBlue,
    //! specular color, alpha channel
    EST_SpecularAlpha,

    //! specular power for phong shading
    EST_PhongPower,

    //! opacity between 0 and 1
    EST_Opacity,

    //! Marks invalid entries
    EST_INVALID
};

// ---------------------------------------------------------------------------------
/** \brief Specifies semantics for PLY elements
 *
 * Semantics define the usage of an element, e.g. vertex or material
*/
enum EElementSemantic {
    //! The element is a vertex
    EEST_Vertex = 0x0u,

    //! The element is a face description (index table)
    EEST_Face,

    //! The element is a triangle-strip description (index table)
    EEST_TriStrip,

    //! The element is an edge description (ignored)
    EEST_Edge,

    //! The element is a material description
    EEST_Material,

    //! texture path
    EEST_TextureFile,

    //! Marks invalid entries
    EEST_INVALID
};

// ---------------------------------------------------------------------------------
/** \brief Helper class for a property in a PLY file.
 *
 * This can e.g. be a part of the vertex declaration
 */
class Property {
public:
    //! Default constructor
    Property() AI_NO_EXCEPT
    : eType (EDT_Int)
    , Semantic()
    , bIsList(false)
    , eFirstType(EDT_UChar) {
        // empty
    }

    //! Data type of the property
    EDataType eType;

    //! Semantical meaning of the property
    ESemantic Semantic;

    //! Of the semantic of the property could not be parsed:
    //! Contains the semantic specified in the file
    std::string szName;

    //! Specifies whether the data type is a list where
    //! the first element specifies the size of the list
    bool bIsList;
    EDataType eFirstType;

    // -------------------------------------------------------------------
    //! Parse a property from a string. The end of the
    //! string is either '\n', '\r' or '\0'. Return value is false
    //! if the input string is NOT a valid property (E.g. does
    //! not start with the "property" keyword)
    static bool ParseProperty(std::vector<char> &buffer, Property* pOut);

    // -------------------------------------------------------------------
    //! Parse a data type from a string
    static EDataType ParseDataType(std::vector<char> &buffer);

    // -------------------------------------------------------------------
    //! Parse a semantic from a string
    static ESemantic ParseSemantic(std::vector<char> &buffer);
};

// ---------------------------------------------------------------------------------
/** \brief Helper class for an element in a PLY file.
 *
 * This can e.g. be the vertex declaration. Elements contain a
 * well-defined number of properties.
 */
class Element {
public:
    //! Default constructor
    Element() AI_NO_EXCEPT
    : eSemantic (EEST_INVALID)
    , NumOccur(0) {
        // empty
    }

    //! List of properties assigned to the element
    //! std::vector to support operator[]
    std::vector<Property> alProperties;

    //! Semantic of the element
    EElementSemantic eSemantic;

    //! Of the semantic of the element could not be parsed:
    //! Contains the semantic specified in the file
    std::string szName;

    //! How many times will the element occur?
    unsigned int NumOccur;


    // -------------------------------------------------------------------
    //! Parse an element from a string.
    //! The function will parse all properties contained in the
    //! element, too.
    static bool ParseElement(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, Element* pOut);

    // -------------------------------------------------------------------
    //! Parse a semantic from a string
    static EElementSemantic ParseSemantic(std::vector<char> &buffer);
};

// ---------------------------------------------------------------------------------
/** \brief Instance of a property in a PLY file
 */
class PropertyInstance
{
public:

    //! Default constructor
    PropertyInstance() AI_NO_EXCEPT {
        // empty
    }

    union ValueUnion
    {

        //! uInt32 representation of the property. All
        // uint types are automatically converted to uint32
        uint32_t iUInt;

        //! Int32 representation of the property. All
        // int types are automatically converted to int32
        int32_t iInt;

        //! Float32 representation of the property
        float fFloat;

        //! Float64 representation of the property
        double fDouble;

    };

    // -------------------------------------------------------------------
    //! List of all values parsed. Contains only one value
    // for non-list properties
    std::vector<ValueUnion> avList;

    // -------------------------------------------------------------------
    //! Parse a property instance
    static bool ParseInstance(const char* &pCur,
        const Property* prop, PropertyInstance* p_pcOut);

    // -------------------------------------------------------------------
    //! Parse a property instance in binary format
    static bool ParseInstanceBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
        const char* &pCur, unsigned int &bufferSize, const Property* prop, PropertyInstance* p_pcOut, bool p_bBE);

    // -------------------------------------------------------------------
    //! Get the default value for a given data type
    static ValueUnion DefaultValue(EDataType eType);

    // -------------------------------------------------------------------
    //! Parse a value
    static bool ParseValue(const char* &pCur, EDataType eType, ValueUnion* out);

    // -------------------------------------------------------------------
    //! Parse a binary value
    static bool ParseValueBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
        const char* &pCur, unsigned int &bufferSize, EDataType eType, ValueUnion* out, bool p_bBE);

    // -------------------------------------------------------------------
    //! Convert a property value to a given type TYPE
    template <typename TYPE>
    static TYPE ConvertTo(ValueUnion v, EDataType eType);
};

// ---------------------------------------------------------------------------------
/** \brief Class for an element instance in a PLY file
 */
class ElementInstance {
public:
    //! Default constructor
    ElementInstance()  AI_NO_EXCEPT
    : alProperties() {
        // empty
    }

    //! List of all parsed properties
    std::vector< PropertyInstance > alProperties;

    // -------------------------------------------------------------------
    //! Parse an element instance
    static bool ParseInstance(const char* &pCur,
        const Element* pcElement, ElementInstance* p_pcOut);

    // -------------------------------------------------------------------
    //! Parse a binary element instance
    static bool ParseInstanceBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
        const char* &pCur, unsigned int &bufferSize, const Element* pcElement, ElementInstance* p_pcOut, bool p_bBE);
};

// ---------------------------------------------------------------------------------
/** \brief Class for an element instance list in a PLY file
 */
class ElementInstanceList
{
public:

    //! Default constructor
    ElementInstanceList() AI_NO_EXCEPT
    : alInstances() {
        // empty
    }

    //! List of all element instances
    std::vector< ElementInstance > alInstances;

    // -------------------------------------------------------------------
    //! Parse an element instance list
    static bool ParseInstanceList(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
        const Element* pcElement, ElementInstanceList* p_pcOut, PLYImporter* loader);

    // -------------------------------------------------------------------
    //! Parse a binary element instance list
    static bool ParseInstanceListBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer,
        const char* &pCur, unsigned int &bufferSize, const Element* pcElement, ElementInstanceList* p_pcOut, PLYImporter* loader, bool p_bBE);
};
// ---------------------------------------------------------------------------------
/** \brief Class to represent the document object model of an ASCII or binary
 * (both little and big-endian) PLY file
 */
class DOM
{
public:

    //! Default constructor
    DOM() AI_NO_EXCEPT
    : alElements()
    , alElementData() {

    }


    //! Contains all elements of the file format
    std::vector<Element> alElements;
    //! Contains the real data of each element's instance list
    std::vector<ElementInstanceList> alElementData;

    //! Parse the DOM for a PLY file. The input string is assumed
    //! to be terminated with zero
    static bool ParseInstance(IOStreamBuffer<char> &streamBuffer, DOM* p_pcOut, PLYImporter* loader);
    static bool ParseInstanceBinary(IOStreamBuffer<char> &streamBuffer, DOM* p_pcOut, PLYImporter* loader, bool p_bBE);

    //! Skip all comment lines after this
    static bool SkipComments(std::vector<char> &buffer);

    static bool SkipSpaces(std::vector<char> &buffer);

    static bool SkipLine(std::vector<char> &buffer);

    static bool TokenMatch(std::vector<char> &buffer, const char* token, unsigned int len);

    static bool SkipSpacesAndLineEnd(std::vector<char> &buffer);

private:

    // -------------------------------------------------------------------
    //! Handle the file header and read all element descriptions
    bool ParseHeader(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, bool p_bBE);

    // -------------------------------------------------------------------
    //! Read in all element instance lists
    bool ParseElementInstanceLists(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, PLYImporter* loader);

    // -------------------------------------------------------------------
    //! Read in all element instance lists for a binary file format
    bool ParseElementInstanceListsBinary(IOStreamBuffer<char> &streamBuffer, std::vector<char> &buffer, const char* &pCur, unsigned int &bufferSize, PLYImporter* loader, bool p_bBE);
};

// ---------------------------------------------------------------------------------
template <typename TYPE>
inline TYPE PLY::PropertyInstance::ConvertTo(
    PLY::PropertyInstance::ValueUnion v, PLY::EDataType eType)
{
    switch (eType)
    {
    case EDT_Float:
        return (TYPE)v.fFloat;
    case EDT_Double:
        return (TYPE)v.fDouble;

    case EDT_UInt:
    case EDT_UShort:
    case EDT_UChar:
        return (TYPE)v.iUInt;

    case EDT_Int:
    case EDT_Short:
    case EDT_Char:
        return (TYPE)v.iInt;
    default: ;
    };
    return (TYPE)0;
}

} // Namespace PLY
} // Namespace AssImp

#endif // !! include guard
