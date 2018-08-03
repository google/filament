/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

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
---------------------------------------------------------------------------
*/

/// \file AMFImporter_Material.cpp
/// \brief Parsing data from material nodes.
/// \date 2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_AMF_IMPORTER

#include "AMFImporter.hpp"
#include "AMFImporter_Macro.hpp"

namespace Assimp
{

// <color
// profile="" - The ICC color space used to interpret the three color channels <r>, <g> and <b>.
// >
// </color>
// A color definition.
// Multi elements - No.
// Parent element - <material>, <object>, <volume>, <vertex>, <triangle>.
//
// "profile" can be one of "sRGB", "AdobeRGB", "Wide-Gamut-RGB", "CIERGB", "CIELAB", or "CIEXYZ".
// Children elements:
//   <r>, <g>, <b>, <a>
//   Multi elements - No.
//   Red, Greed, Blue and Alpha (transparency) component of a color in sRGB space, values ranging from 0 to 1. The
//   values can be specified as constants, or as a formula depending on the coordinates.
void AMFImporter::ParseNode_Color()
{
std::string profile;
CAMFImporter_NodeElement* ne;

	// Read attributes for node <color>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("profile", profile, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create new color object.
	ne = new CAMFImporter_NodeElement_Color(mNodeElement_Cur);

	CAMFImporter_NodeElement_Color& als = *((CAMFImporter_NodeElement_Color*)ne);// alias for convenience

	als.Profile = profile;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool read_flag[4] = { false, false, false, false };

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("color");
			MACRO_NODECHECK_READCOMP_F("r", read_flag[0], als.Color.r);
			MACRO_NODECHECK_READCOMP_F("g", read_flag[1], als.Color.g);
			MACRO_NODECHECK_READCOMP_F("b", read_flag[2], als.Color.b);
			MACRO_NODECHECK_READCOMP_F("a", read_flag[3], als.Color.a);
		MACRO_NODECHECK_LOOPEND("color");
		ParseHelper_Node_Exit();
		// check that all components was defined
		if(!(read_flag[0] && read_flag[1] && read_flag[2])) throw DeadlyImportError("Not all color components are defined.");
		// check if <a> is absent. Then manualy add "a == 1".
		if(!read_flag[3]) als.Color.a = 1;

	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	als.Composed = false;
	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <material
// id="" - A unique material id. material ID "0" is reserved to denote no material (void) or sacrificial material.
// >
// </material>
// An available material.
// Multi elements - Yes.
// Parent element - <amf>.
void AMFImporter::ParseNode_Material()
{
std::string id;
CAMFImporter_NodeElement* ne;

	// Read attributes for node <color>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("id", id, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create new object.
	ne = new CAMFImporter_NodeElement_Material(mNodeElement_Cur);
	// and assign read data
	((CAMFImporter_NodeElement_Material*)ne)->ID = id;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool col_read = false;

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("material");
			if(XML_CheckNode_NameEqual("color"))
			{
				// Check if data already defined.
				if(col_read) Throw_MoreThanOnceDefined("color", "Only one color can be defined for <material>.");
				// read data and set flag about it
				ParseNode_Color();
				col_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("material");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <texture
// id=""     - Assigns a unique texture id for the new texture.
// width=""  - Width (horizontal size, x) of the texture, in pixels.
// height="" - Height (lateral size, y) of the texture, in pixels.
// depth=""  - Depth (vertical size, z) of the texture, in pixels.
// type=""   - Encoding of the data in the texture. Currently allowed values are "grayscale" only. In grayscale mode, each pixel is represented by one byte
//   in the range of 0-255. When the texture is referenced using the tex function, these values are converted into a single floating point number in the
//   range of 0-1 (see Annex 2). A full color graphics will typically require three textures, one for each of the color channels. A graphic involving
//   transparency may require a fourth channel.
// tiled=""  - If true then texture repeated when UV-coordinates is greater than 1.
// >
// </triangle>
// Specifies an texture data to be used as a map. Lists a sequence of Base64 values specifying values for pixels from left to right then top to bottom,
// then layer by layer.
// Multi elements - Yes.
// Parent element - <amf>.
void AMFImporter::ParseNode_Texture()
{
std::string id;
uint32_t width = 0;
uint32_t height = 0;
uint32_t depth = 1;
std::string type;
bool tiled = false;
std::string enc64_data;
CAMFImporter_NodeElement* ne;

	// Read attributes for node <color>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("id", id, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("width", width, XML_ReadNode_GetAttrVal_AsU32);
		MACRO_ATTRREAD_CHECK_RET("height", height, XML_ReadNode_GetAttrVal_AsU32);
		MACRO_ATTRREAD_CHECK_RET("depth", depth, XML_ReadNode_GetAttrVal_AsU32);
		MACRO_ATTRREAD_CHECK_RET("type", type, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("tiled", tiled, XML_ReadNode_GetAttrVal_AsBool);
	MACRO_ATTRREAD_LOOPEND;

	// create new texture object.
	ne = new CAMFImporter_NodeElement_Texture(mNodeElement_Cur);

	CAMFImporter_NodeElement_Texture& als = *((CAMFImporter_NodeElement_Texture*)ne);// alias for convenience

	// Check for child nodes
	if(!mReader->isEmptyElement()) XML_ReadNode_GetVal_AsString(enc64_data);

	// check that all components was defined
	if(id.empty()) throw DeadlyImportError("ID for texture must be defined.");
	if(width < 1) Throw_IncorrectAttrValue("width");
	if(height < 1) Throw_IncorrectAttrValue("height");
	if(depth < 1) Throw_IncorrectAttrValue("depth");
	if(type != "grayscale") Throw_IncorrectAttrValue("type");
	if(enc64_data.empty()) throw DeadlyImportError("Texture data not defined.");
	// copy data
	als.ID = id;
	als.Width = width;
	als.Height = height;
	als.Depth = depth;
	als.Tiled = tiled;
	ParseHelper_Decode_Base64(enc64_data, als.Data);
	// check data size
	if((width * height * depth) != als.Data.size()) throw DeadlyImportError("Texture has incorrect data size.");

	mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <texmap
// rtexid="" - Texture ID for red color component.
// gtexid="" - Texture ID for green color component.
// btexid="" - Texture ID for blue color component.
// atexid="" - Texture ID for alpha color component. Optional.
// >
// </texmap>, old name: <map>
// Specifies texture coordinates for triangle.
// Multi elements - No.
// Parent element - <triangle>.
// Children elements:
//   <utex1>, <utex2>, <utex3>, <vtex1>, <vtex2>, <vtex3>. Old name: <u1>, <u2>, <u3>, <v1>, <v2>, <v3>.
//   Multi elements - No.
//   Texture coordinates for every vertex of triangle.
void AMFImporter::ParseNode_TexMap(const bool pUseOldName)
{
std::string rtexid, gtexid, btexid, atexid;
CAMFImporter_NodeElement* ne;

	// Read attributes for node <color>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("rtexid", rtexid, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("gtexid", gtexid, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("btexid", btexid, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("atexid", atexid, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create new texture coordinates object.
	ne = new CAMFImporter_NodeElement_TexMap(mNodeElement_Cur);

	CAMFImporter_NodeElement_TexMap& als = *((CAMFImporter_NodeElement_TexMap*)ne);// alias for convenience
	// check data
	if(rtexid.empty() && gtexid.empty() && btexid.empty()) throw DeadlyImportError("ParseNode_TexMap. At least one texture ID must be defined.");
	// Check for children nodes
	XML_CheckNode_MustHaveChildren();
	// read children nodes
	bool read_flag[6] = { false, false, false, false, false, false };

	ParseHelper_Node_Enter(ne);
	if(!pUseOldName)
	{
		MACRO_NODECHECK_LOOPBEGIN("texmap");
			MACRO_NODECHECK_READCOMP_F("utex1", read_flag[0], als.TextureCoordinate[0].x);
			MACRO_NODECHECK_READCOMP_F("utex2", read_flag[1], als.TextureCoordinate[1].x);
			MACRO_NODECHECK_READCOMP_F("utex3", read_flag[2], als.TextureCoordinate[2].x);
			MACRO_NODECHECK_READCOMP_F("vtex1", read_flag[3], als.TextureCoordinate[0].y);
			MACRO_NODECHECK_READCOMP_F("vtex2", read_flag[4], als.TextureCoordinate[1].y);
			MACRO_NODECHECK_READCOMP_F("vtex3", read_flag[5], als.TextureCoordinate[2].y);
		MACRO_NODECHECK_LOOPEND("texmap");
	}
	else
	{
		MACRO_NODECHECK_LOOPBEGIN("map");
			MACRO_NODECHECK_READCOMP_F("u1", read_flag[0], als.TextureCoordinate[0].x);
			MACRO_NODECHECK_READCOMP_F("u2", read_flag[1], als.TextureCoordinate[1].x);
			MACRO_NODECHECK_READCOMP_F("u3", read_flag[2], als.TextureCoordinate[2].x);
			MACRO_NODECHECK_READCOMP_F("v1", read_flag[3], als.TextureCoordinate[0].y);
			MACRO_NODECHECK_READCOMP_F("v2", read_flag[4], als.TextureCoordinate[1].y);
			MACRO_NODECHECK_READCOMP_F("v3", read_flag[5], als.TextureCoordinate[2].y);
		MACRO_NODECHECK_LOOPEND("map");
	}// if(!pUseOldName) else

	ParseHelper_Node_Exit();

	// check that all components was defined
	if(!(read_flag[0] && read_flag[1] && read_flag[2] && read_flag[3] && read_flag[4] && read_flag[5]))
		throw DeadlyImportError("Not all texture coordinates are defined.");

	// copy attributes data
	als.TextureID_R = rtexid;
	als.TextureID_G = gtexid;
	als.TextureID_B = btexid;
	als.TextureID_A = atexid;

	mNodeElement_List.push_back(ne);// add to node element list because its a new object in graph.
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_AMF_IMPORTER
