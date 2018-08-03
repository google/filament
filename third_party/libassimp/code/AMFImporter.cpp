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

/// \file AMFImporter.cpp
/// \brief AMF-format files importer for Assimp: main algorithm implementation.
/// \date 2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_AMF_IMPORTER

// Header files, Assimp.
#include "AMFImporter.hpp"
#include "AMFImporter_Macro.hpp"

#include "fast_atof.h"
#include <assimp/DefaultIOSystem.h>

// Header files, stdlib.
#include <memory>

namespace Assimp
{

/// \var aiImporterDesc AMFImporter::Description
/// Conastant which hold importer description
const aiImporterDesc AMFImporter::Description = {
	"Additive manufacturing file format(AMF) Importer",
	"smalcom",
	"",
	"See documentation in source code. Chapter: Limitations.",
	aiImporterFlags_SupportTextFlavour | aiImporterFlags_LimitedSupport | aiImporterFlags_Experimental,
	0,
	0,
	0,
	0,
	"amf"
};

void AMFImporter::Clear()
{
	mNodeElement_Cur = nullptr;
	mUnit.clear();
	mMaterial_Converted.clear();
	mTexture_Converted.clear();
	// Delete all elements
	if(mNodeElement_List.size())
	{
		for(CAMFImporter_NodeElement* ne: mNodeElement_List) { delete ne; }

		mNodeElement_List.clear();
	}
}

AMFImporter::~AMFImporter()
{
	if(mReader != nullptr) delete mReader;
	// Clear() is accounting if data already is deleted. So, just check again if all data is deleted.
	Clear();
}

/*********************************************************************************************************************************************/
/************************************************************ Functions: find set ************************************************************/
/*********************************************************************************************************************************************/

bool AMFImporter::Find_NodeElement(const std::string& pID, const CAMFImporter_NodeElement::EType pType, CAMFImporter_NodeElement** pNodeElement) const
{
	for(CAMFImporter_NodeElement* ne: mNodeElement_List)
	{
		if((ne->ID == pID) && (ne->Type == pType))
		{
			if(pNodeElement != nullptr) *pNodeElement = ne;

			return true;
		}
	}// for(CAMFImporter_NodeElement* ne: mNodeElement_List)

	return false;
}

bool AMFImporter::Find_ConvertedNode(const std::string& pID, std::list<aiNode*>& pNodeList, aiNode** pNode) const
{
aiString node_name(pID.c_str());

	for(aiNode* node: pNodeList)
	{
		if(node->mName == node_name)
		{
			if(pNode != nullptr) *pNode = node;

			return true;
		}
	}// for(aiNode* node: pNodeList)

	return false;
}

bool AMFImporter::Find_ConvertedMaterial(const std::string& pID, const SPP_Material** pConvertedMaterial) const
{
	for(const SPP_Material& mat: mMaterial_Converted)
	{
		if(mat.ID == pID)
		{
			if(pConvertedMaterial != nullptr) *pConvertedMaterial = &mat;

			return true;
		}
	}// for(const SPP_Material& mat: mMaterial_Converted)

	return false;
}

/*********************************************************************************************************************************************/
/************************************************************ Functions: throw set ***********************************************************/
/*********************************************************************************************************************************************/

void AMFImporter::Throw_CloseNotFound(const std::string& pNode)
{
	throw DeadlyImportError("Close tag for node <" + pNode + "> not found. Seems file is corrupt.");
}

void AMFImporter::Throw_IncorrectAttr(const std::string& pAttrName)
{
	throw DeadlyImportError("Node <" + std::string(mReader->getNodeName()) + "> has incorrect attribute \"" + pAttrName + "\".");
}

void AMFImporter::Throw_IncorrectAttrValue(const std::string& pAttrName)
{
	throw DeadlyImportError("Attribute \"" + pAttrName + "\" in node <" + std::string(mReader->getNodeName()) + "> has incorrect value.");
}

void AMFImporter::Throw_MoreThanOnceDefined(const std::string& pNodeType, const std::string& pDescription)
{
	throw DeadlyImportError("\"" + pNodeType + "\" node can be used only once in " + mReader->getNodeName() + ". Description: " + pDescription);
}

void AMFImporter::Throw_ID_NotFound(const std::string& pID) const
{
	throw DeadlyImportError("Not found node with name \"" + pID + "\".");
}

/*********************************************************************************************************************************************/
/************************************************************* Functions: XML set ************************************************************/
/*********************************************************************************************************************************************/

void AMFImporter::XML_CheckNode_MustHaveChildren()
{
	if(mReader->isEmptyElement()) throw DeadlyImportError(std::string("Node <") + mReader->getNodeName() + "> must have children.");
}

void AMFImporter::XML_CheckNode_SkipUnsupported(const std::string& pParentNodeName)
{
    static const size_t Uns_Skip_Len = 3;
    const char* Uns_Skip[Uns_Skip_Len] = { "composite", "edge", "normal" };

    static bool skipped_before[Uns_Skip_Len] = { false, false, false };

    std::string nn(mReader->getNodeName());
    bool found = false;
    bool close_found = false;
    size_t sk_idx;

	for(sk_idx = 0; sk_idx < Uns_Skip_Len; sk_idx++)
	{
		if(nn != Uns_Skip[sk_idx]) continue;

		found = true;
		if(mReader->isEmptyElement())
		{
			close_found = true;

			goto casu_cres;
		}

		while(mReader->read())
		{
			if((mReader->getNodeType() == irr::io::EXN_ELEMENT_END) && (nn == mReader->getNodeName()))
			{
				close_found = true;

				goto casu_cres;
			}
		}
	}// for(sk_idx = 0; sk_idx < Uns_Skip_Len; sk_idx++)

casu_cres:

	if(!found) throw DeadlyImportError("Unknown node \"" + nn + "\" in " + pParentNodeName + ".");
	if(!close_found) Throw_CloseNotFound(nn);

	if(!skipped_before[sk_idx])
	{
		skipped_before[sk_idx] = true;
		LogWarning("Skipping node \"" + nn + "\" in " + pParentNodeName + ".");
	}
}

bool AMFImporter::XML_SearchNode(const std::string& pNodeName)
{
	while(mReader->read())
	{
		if((mReader->getNodeType() == irr::io::EXN_ELEMENT) && XML_CheckNode_NameEqual(pNodeName)) return true;
	}

	return false;
}

bool AMFImporter::XML_ReadNode_GetAttrVal_AsBool(const int pAttrIdx)
{
    std::string val(mReader->getAttributeValue(pAttrIdx));

	if((val == "false") || (val == "0"))
		return false;
	else if((val == "true") || (val == "1"))
		return true;
	else
		throw DeadlyImportError("Bool attribute value can contain \"false\"/\"0\" or \"true\"/\"1\" not the \"" + val + "\"");
}

float AMFImporter::XML_ReadNode_GetAttrVal_AsFloat(const int pAttrIdx)
{
    std::string val;
    float tvalf;

	ParseHelper_FixTruncatedFloatString(mReader->getAttributeValue(pAttrIdx), val);
	fast_atoreal_move(val.c_str(), tvalf, false);

	return tvalf;
}

uint32_t AMFImporter::XML_ReadNode_GetAttrVal_AsU32(const int pAttrIdx)
{
	return strtoul10(mReader->getAttributeValue(pAttrIdx));
}

float AMFImporter::XML_ReadNode_GetVal_AsFloat()
{
    std::string val;
    float tvalf;

	if(!mReader->read()) throw DeadlyImportError("XML_ReadNode_GetVal_AsFloat. No data, seems file is corrupt.");
	if(mReader->getNodeType() != irr::io::EXN_TEXT) throw DeadlyImportError("XML_ReadNode_GetVal_AsFloat. Invalid type of XML element, seems file is corrupt.");

	ParseHelper_FixTruncatedFloatString(mReader->getNodeData(), val);
	fast_atoreal_move(val.c_str(), tvalf, false);

	return tvalf;
}

uint32_t AMFImporter::XML_ReadNode_GetVal_AsU32()
{
	if(!mReader->read()) throw DeadlyImportError("XML_ReadNode_GetVal_AsU32. No data, seems file is corrupt.");
	if(mReader->getNodeType() != irr::io::EXN_TEXT) throw DeadlyImportError("XML_ReadNode_GetVal_AsU32. Invalid type of XML element, seems file is corrupt.");

	return strtoul10(mReader->getNodeData());
}

void AMFImporter::XML_ReadNode_GetVal_AsString(std::string& pValue)
{
	if(!mReader->read()) throw DeadlyImportError("XML_ReadNode_GetVal_AsString. No data, seems file is corrupt.");
	if(mReader->getNodeType() != irr::io::EXN_TEXT)
		throw DeadlyImportError("XML_ReadNode_GetVal_AsString. Invalid type of XML element, seems file is corrupt.");

	pValue = mReader->getNodeData();
}

/*********************************************************************************************************************************************/
/************************************************************ Functions: parse set ***********************************************************/
/*********************************************************************************************************************************************/

void AMFImporter::ParseHelper_Node_Enter(CAMFImporter_NodeElement* pNode)
{
	mNodeElement_Cur->Child.push_back(pNode);// add new element to current element child list.
	mNodeElement_Cur = pNode;// switch current element to new one.
}

void AMFImporter::ParseHelper_Node_Exit()
{
	// check if we can walk up.
	if(mNodeElement_Cur != nullptr) mNodeElement_Cur = mNodeElement_Cur->Parent;
}

void AMFImporter::ParseHelper_FixTruncatedFloatString(const char* pInStr, std::string& pOutString)
{
    size_t instr_len;

	pOutString.clear();
	instr_len = strlen(pInStr);
	if(!instr_len) return;

	pOutString.reserve(instr_len * 3 / 2);
	// check and correct floats in format ".x". Must be "x.y".
	if(pInStr[0] == '.') pOutString.push_back('0');

	pOutString.push_back(pInStr[0]);
	for(size_t ci = 1; ci < instr_len; ci++)
	{
		if((pInStr[ci] == '.') && ((pInStr[ci - 1] == ' ') || (pInStr[ci - 1] == '-') || (pInStr[ci - 1] == '+') || (pInStr[ci - 1] == '\t')))
		{
			pOutString.push_back('0');
			pOutString.push_back('.');
		}
		else
		{
			pOutString.push_back(pInStr[ci]);
		}
	}
}

static bool ParseHelper_Decode_Base64_IsBase64(const char pChar)
{
	return (isalnum(pChar) || (pChar == '+') || (pChar == '/'));
}

void AMFImporter::ParseHelper_Decode_Base64(const std::string& pInputBase64, std::vector<uint8_t>& pOutputData) const
{
    // With help from
    // René Nyffenegger http://www.adp-gmbh.ch/cpp/common/base64.html
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    uint8_t tidx = 0;
    uint8_t arr4[4], arr3[3];

	// check input data
	if(pInputBase64.size() % 4) throw DeadlyImportError("Base64-encoded data must have size multiply of four.");
	// prepare output place
	pOutputData.clear();
	pOutputData.reserve(pInputBase64.size() / 4 * 3);

	for(size_t in_len = pInputBase64.size(), in_idx = 0; (in_len > 0) && (pInputBase64[in_idx] != '='); in_len--)
	{
		if(ParseHelper_Decode_Base64_IsBase64(pInputBase64[in_idx]))
		{
			arr4[tidx++] = pInputBase64[in_idx++];
			if(tidx == 4)
			{
				for(tidx = 0; tidx < 4; tidx++) arr4[tidx] = (uint8_t)base64_chars.find(arr4[tidx]);

				arr3[0] = (arr4[0] << 2) + ((arr4[1] & 0x30) >> 4);
				arr3[1] = ((arr4[1] & 0x0F) << 4) + ((arr4[2] & 0x3C) >> 2);
				arr3[2] = ((arr4[2] & 0x03) << 6) + arr4[3];
				for(tidx = 0; tidx < 3; tidx++) pOutputData.push_back(arr3[tidx]);

				tidx = 0;
			}// if(tidx == 4)
		}// if(ParseHelper_Decode_Base64_IsBase64(pInputBase64[in_idx]))
		else
		{
			in_idx++;
		}// if(ParseHelper_Decode_Base64_IsBase64(pInputBase64[in_idx])) else
	}

	if(tidx)
	{
		for(uint8_t i = tidx; i < 4; i++) arr4[i] = 0;
		for(uint8_t i = 0; i < 4; i++) arr4[i] = (uint8_t)(base64_chars.find(arr4[i]));

		arr3[0] = (arr4[0] << 2) + ((arr4[1] & 0x30) >> 4);
		arr3[1] = ((arr4[1] & 0x0F) << 4) + ((arr4[2] & 0x3C) >> 2);
		arr3[2] = ((arr4[2] & 0x03) << 6) + arr4[3];
		for(uint8_t i = 0; i < (tidx - 1); i++) pOutputData.push_back(arr3[i]);
	}
}

void AMFImporter::ParseFile(const std::string& pFile, IOSystem* pIOHandler)
{
    irr::io::IrrXMLReader* OldReader = mReader;// store current XMLreader.
    std::unique_ptr<IOStream> file(pIOHandler->Open(pFile, "rb"));

	// Check whether we can read from the file
	if(file.get() == NULL) throw DeadlyImportError("Failed to open AMF file " + pFile + ".");

	// generate a XML reader for it
	std::unique_ptr<CIrrXML_IOStreamReader> mIOWrapper(new CIrrXML_IOStreamReader(file.get()));
	mReader = irr::io::createIrrXMLReader(mIOWrapper.get());
	if(!mReader) throw DeadlyImportError("Failed to create XML reader for file" + pFile + ".");
	//
	// start reading
	// search for root tag <amf>
	if(XML_SearchNode("amf"))
		ParseNode_Root();
	else
		throw DeadlyImportError("Root node \"amf\" not found.");

	delete mReader;
	// restore old XMLreader
	mReader = OldReader;
}

// <amf
// unit="" - The units to be used. May be "inch", "millimeter", "meter", "feet", or "micron".
// version="" - Version of file format.
// >
// </amf>
// Root XML element.
// Multi elements - No.
void AMFImporter::ParseNode_Root()
{
    std::string unit, version;
    CAMFImporter_NodeElement *ne( nullptr );

	// Read attributes for node <amf>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("unit", unit, mReader->getAttributeValue);
		MACRO_ATTRREAD_CHECK_RET("version", version, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND_WSKIP;

	// Check attributes
	if(!mUnit.empty())
	{
		if((mUnit != "inch") && (mUnit != "millimeter") && (mUnit != "meter") && (mUnit != "feet") && (mUnit != "micron")) Throw_IncorrectAttrValue("unit");
	}

	// create root node element.
	ne = new CAMFImporter_NodeElement_Root(nullptr);
	mNodeElement_Cur = ne;// set first "current" element
	// and assign attribute's values
	((CAMFImporter_NodeElement_Root*)ne)->Unit = unit;
	((CAMFImporter_NodeElement_Root*)ne)->Version = version;

	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		MACRO_NODECHECK_LOOPBEGIN("amf");
			if(XML_CheckNode_NameEqual("object")) { ParseNode_Object(); continue; }
			if(XML_CheckNode_NameEqual("material")) { ParseNode_Material(); continue; }
			if(XML_CheckNode_NameEqual("texture")) { ParseNode_Texture(); continue; }
			if(XML_CheckNode_NameEqual("constellation")) { ParseNode_Constellation(); continue; }
			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("amf");
		mNodeElement_Cur = ne;// force restore "current" element
	}// if(!mReader->isEmptyElement())

	mNodeElement_List.push_back(ne);// add to node element list because its a new object in graph.
}

// <constellation
// id="" - The Object ID of the new constellation being defined.
// >
// </constellation>
// A collection of objects or constellations with specific relative locations.
// Multi elements - Yes.
// Parent element - <amf>.
void AMFImporter::ParseNode_Constellation()
{
    std::string id;
    CAMFImporter_NodeElement* ne( nullptr );

	// Read attributes for node <constellation>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("id", id, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create and if needed - define new grouping object.
	ne = new CAMFImporter_NodeElement_Constellation(mNodeElement_Cur);

	CAMFImporter_NodeElement_Constellation& als = *((CAMFImporter_NodeElement_Constellation*)ne);// alias for convenience

	if(!id.empty()) als.ID = id;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("constellation");
			if(XML_CheckNode_NameEqual("instance")) { ParseNode_Instance(); continue; }
			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("constellation");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <instance
// objectid="" - The Object ID of the new constellation being defined.
// >
// </instance>
// A collection of objects or constellations with specific relative locations.
// Multi elements - Yes.
// Parent element - <amf>.
void AMFImporter::ParseNode_Instance()
{
    std::string objectid;
    CAMFImporter_NodeElement* ne( nullptr );

	// Read attributes for node <constellation>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("objectid", objectid, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// used object id must be defined, check that.
	if(objectid.empty()) throw DeadlyImportError("\"objectid\" in <instance> must be defined.");
	// create and define new grouping object.
	ne = new CAMFImporter_NodeElement_Instance(mNodeElement_Cur);

	CAMFImporter_NodeElement_Instance& als = *((CAMFImporter_NodeElement_Instance*)ne);// alias for convenience

	als.ObjectID = objectid;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool read_flag[6] = { false, false, false, false, false, false };

		als.Delta.Set(0, 0, 0);
		als.Rotation.Set(0, 0, 0);
		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("instance");
			MACRO_NODECHECK_READCOMP_F("deltax", read_flag[0], als.Delta.x);
			MACRO_NODECHECK_READCOMP_F("deltay", read_flag[1], als.Delta.y);
			MACRO_NODECHECK_READCOMP_F("deltaz", read_flag[2], als.Delta.z);
			MACRO_NODECHECK_READCOMP_F("rx", read_flag[3], als.Rotation.x);
			MACRO_NODECHECK_READCOMP_F("ry", read_flag[4], als.Rotation.y);
			MACRO_NODECHECK_READCOMP_F("rz", read_flag[5], als.Rotation.z);
		MACRO_NODECHECK_LOOPEND("instance");
		ParseHelper_Node_Exit();
		// also convert degrees to radians.
		als.Rotation.x = AI_MATH_PI_F * als.Rotation.x / 180.0f;
		als.Rotation.y = AI_MATH_PI_F * als.Rotation.y / 180.0f;
		als.Rotation.z = AI_MATH_PI_F * als.Rotation.z / 180.0f;
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <object
// id="" - A unique ObjectID for the new object being defined.
// >
// </object>
// An object definition.
// Multi elements - Yes.
// Parent element - <amf>.
void AMFImporter::ParseNode_Object()
{
    std::string id;
    CAMFImporter_NodeElement* ne( nullptr );

	// Read attributes for node <object>.
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("id", id, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;

	// create and if needed - define new geometry object.
	ne = new CAMFImporter_NodeElement_Object(mNodeElement_Cur);

	CAMFImporter_NodeElement_Object& als = *((CAMFImporter_NodeElement_Object*)ne);// alias for convenience

	if(!id.empty()) als.ID = id;
	// Check for child nodes
	if(!mReader->isEmptyElement())
	{
		bool col_read = false;

		ParseHelper_Node_Enter(ne);
		MACRO_NODECHECK_LOOPBEGIN("object");
			if(XML_CheckNode_NameEqual("color"))
			{
				// Check if color already defined for object.
				if(col_read) Throw_MoreThanOnceDefined("color", "Only one color can be defined for <object>.");
				// read data and set flag about it
				ParseNode_Color();
				col_read = true;

				continue;
			}

			if(XML_CheckNode_NameEqual("mesh")) { ParseNode_Mesh(); continue; }
			if(XML_CheckNode_NameEqual("metadata")) { ParseNode_Metadata(); continue; }
		MACRO_NODECHECK_LOOPEND("object");
		ParseHelper_Node_Exit();
	}// if(!mReader->isEmptyElement())
	else
	{
		mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	}// if(!mReader->isEmptyElement()) else

	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

// <metadata
// type="" - The type of the attribute.
// >
// </metadata>
// Specify additional information about an entity.
// Multi elements - Yes.
// Parent element - <amf>, <object>, <volume>, <material>, <vertex>.
//
// Reserved types are:
// "Name" - The alphanumeric label of the entity, to be used by the interpreter if interacting with the user.
// "Description" - A description of the content of the entity
// "URL" - A link to an external resource relating to the entity
// "Author" - Specifies the name(s) of the author(s) of the entity
// "Company" - Specifying the company generating the entity
// "CAD" - specifies the name of the originating CAD software and version
// "Revision" - specifies the revision of the entity
// "Tolerance" - specifies the desired manufacturing tolerance of the entity in entity's unit system
// "Volume" - specifies the total volume of the entity, in the entity's unit system, to be used for verification (object and volume only)
void AMFImporter::ParseNode_Metadata()
{
    std::string type, value;
    CAMFImporter_NodeElement* ne( nullptr );

	// read attribute
	MACRO_ATTRREAD_LOOPBEG;
		MACRO_ATTRREAD_CHECK_RET("type", type, mReader->getAttributeValue);
	MACRO_ATTRREAD_LOOPEND;
	// and value of node.
	value = mReader->getNodeData();
	// Create node element and assign read data.
	ne = new CAMFImporter_NodeElement_Metadata(mNodeElement_Cur);
	((CAMFImporter_NodeElement_Metadata*)ne)->Type = type;
	((CAMFImporter_NodeElement_Metadata*)ne)->Value = value;
	mNodeElement_Cur->Child.push_back(ne);// Add element to child list of current element
	mNodeElement_List.push_back(ne);// and to node element list because its a new object in graph.
}

/*********************************************************************************************************************************************/
/******************************************************** Functions: BaseImporter set ********************************************************/
/*********************************************************************************************************************************************/

bool AMFImporter::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool pCheckSig) const
{
    const std::string extension = GetExtension(pFile);

    if ( extension == "amf" ) {
        return true;
    }

	if(!extension.length() || pCheckSig)
	{
		const char* tokens[] = { "<amf" };

		return SearchFileHeaderForToken( pIOHandler, pFile, tokens, 1 );
	}

	return false;
}

void AMFImporter::GetExtensionList(std::set<std::string>& pExtensionList)
{
	pExtensionList.insert("amf");
}

const aiImporterDesc* AMFImporter::GetInfo () const
{
	return &Description;
}

void AMFImporter::InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
	Clear();// delete old graph.
	ParseFile(pFile, pIOHandler);
	Postprocess_BuildScene(pScene);
	// scene graph is ready, exit.
}

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_AMF_IMPORTER
