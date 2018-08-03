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

/// \file AMFImporter.hpp
/// \brief AMF-format files importer for Assimp.
/// \date 2016
/// \author smal.root@gmail.com
// Thanks to acorn89 for support.

#pragma once
#ifndef INCLUDED_AI_AMF_IMPORTER_H
#define INCLUDED_AI_AMF_IMPORTER_H

#include "AMFImporter_Node.hpp"

// Header files, Assimp.
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>
#include "assimp/types.h"
#include "BaseImporter.h"
#include "irrXMLWrapper.h"

// Header files, stdlib.
#include <set>

namespace Assimp
{
/// \class AMFImporter
/// Class that holding scene graph which include: geometry, metadata, materials etc.
///
/// Implementing features.
///
/// Limitations.
///
/// 1. When for texture mapping used set of source textures (r, g, b, a) not only one then attribute "tiled" for all set will be true if it true in any of
///    source textures.
///    Example. Triangle use for texture mapping three textures. Two of them has "tiled" set to false and one - set to true. In scene all three textures
///    will be tiled.
///
/// Unsupported features:
/// 1. Node <composite>, formulas in <composite> and <color>. For implementing this feature can be used expression parser "muParser" like in project
///    "amf_tools".
/// 2. Attribute "profile" in node <color>.
/// 3. Curved geometry: <edge>, <normal> and children nodes of them.
/// 4. Attributes: "unit" and "version" in <amf> read but do nothing.
/// 5. <metadata> stored only for root node <amf>.
/// 6. Color averaging of vertices for which <triangle>'s set different colors.
///
/// Supported nodes:
///    General:
///        <amf>; <constellation>; <instance> and children <deltax>, <deltay>, <deltaz>, <rx>, <ry>, <rz>; <metadata>;
///
///    Geometry:
///        <object>; <mesh>; <vertices>; <vertex>; <coordinates> and children <x>, <y>, <z>; <volume>; <triangle> and children <v1>, <v2>, <v3>;
///
///    Material:
///        <color> and children <r>, <g>, <b>, <a>; <texture>; <material>;
///        two variants of texture coordinates:
///            new - <texmap> and children <utex1>, <utex2>, <utex3>, <vtex1>, <vtex2>, <vtex3>
///            old - <map> and children <u1>, <u2>, <u3>, <v1>, <v2>, <v3>
///
class AMFImporter : public BaseImporter
{
	/***********************************************/
	/******************** Types ********************/
	/***********************************************/

private:

	struct SPP_Material;// forward declaration

	/// \struct SPP_Composite
	/// Data type for postprocessing step. More suitable container for part of material's composition.
	struct SPP_Composite
	{
		SPP_Material* Material;///< Pointer to material - part of composition.
		std::string Formula;///< Formula for calculating ratio of \ref Material.
	};

	/// \struct SPP_Material
	/// Data type for postprocessing step. More suitable container for material.
	struct SPP_Material
	{
		std::string ID;///< Material ID.
		std::list<CAMFImporter_NodeElement_Metadata*> Metadata;///< Metadata of material.
		CAMFImporter_NodeElement_Color* Color;///< Color of material.
		std::list<SPP_Composite> Composition;///< List of child materials if current material is composition of few another.

		/// \fn aiColor4D GetColor(const float pX, const float pY, const float pZ) const
		/// Return color calculated for specified coordinate.
		/// \param [in] pX - "x" coordinate.
		/// \param [in] pY - "y" coordinate.
		/// \param [in] pZ - "z" coordinate.
		/// \return calculated color.
		aiColor4D GetColor(const float pX, const float pY, const float pZ) const;
	};

	/// \struct SPP_Texture
	/// Data type for post-processing step. More suitable container for texture.
	struct SPP_Texture
	{
		std::string ID;
		size_t      Width, Height, Depth;
		bool        Tiled;
        char        FormatHint[ 9 ];// 8 for string + 1 for terminator.
		uint8_t    *Data;
	};

	///	\struct SComplexFace
	/// Data type for post-processing step. Contain face data.
	struct SComplexFace
	{
		aiFace Face;///< Face vertices.
		const CAMFImporter_NodeElement_Color* Color;///< Face color. Equal to nullptr if color is not set for the face.
		const CAMFImporter_NodeElement_TexMap* TexMap;///< Face texture mapping data. Equal to nullptr if texture mapping is not set for the face.
	};



	/***********************************************/
	/****************** Constants ******************/
	/***********************************************/

private:

	static const aiImporterDesc Description;

	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

private:

    CAMFImporter_NodeElement* mNodeElement_Cur;///< Current element.
    std::list<CAMFImporter_NodeElement*> mNodeElement_List;///< All elements of scene graph.
	irr::io::IrrXMLReader* mReader;///< Pointer to XML-reader object
	std::string mUnit;
	std::list<SPP_Material> mMaterial_Converted;///< List of converted materials for postprocessing step.
	std::list<SPP_Texture> mTexture_Converted;///< List of converted textures for postprocessing step.

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

private:

	/// \fn AMFImporter(const AMFImporter& pScene)
	/// Disabled copy constructor.
	AMFImporter(const AMFImporter& pScene);

	/// \fn AMFImporter& operator=(const AMFImporter& pScene)
	/// Disabled assign operator.
	AMFImporter& operator=(const AMFImporter& pScene);

	/// \fn void Clear()
	/// Clear all temporary data.
	void Clear();

	/***********************************************/
	/************* Functions: find set *************/
	/***********************************************/

	/// \fn bool Find_NodeElement(const std::string& pID, const CAMFImporter_NodeElement::EType pType, aiNode** pNode) const
	/// Find specified node element in node elements list ( \ref mNodeElement_List).
	/// \param [in] pID - ID(name) of requested node element.
	/// \param [in] pType - type of node element.
	/// \param [out] pNode - pointer to pointer to item found.
	/// \return true - if the node element is found, else - false.
	bool Find_NodeElement(const std::string& pID, const CAMFImporter_NodeElement::EType pType, CAMFImporter_NodeElement** pNodeElement) const;

	/// \fn bool Find_ConvertedNode(const std::string& pID, std::list<aiNode*>& pNodeList, aiNode** pNode) const
	/// Find requested aiNode in node list.
	/// \param [in] pID - ID(name) of requested node.
	/// \param [in] pNodeList - list of nodes where to find the node.
	/// \param [out] pNode - pointer to pointer to item found.
	/// \return true - if the node is found, else - false.
	bool Find_ConvertedNode(const std::string& pID, std::list<aiNode*>& pNodeList, aiNode** pNode) const;

	/// \fn bool Find_ConvertedMaterial(const std::string& pID, const SPP_Material** pConvertedMaterial) const
	/// Find material in list for converted materials. Use at postprocessing step.
	/// \param [in] pID - material ID.
	/// \param [out] pConvertedMaterial - pointer to found converted material (\ref SPP_Material).
	/// \return true - if the material is found, else - false.
	bool Find_ConvertedMaterial(const std::string& pID, const SPP_Material** pConvertedMaterial) const;

	/// \fn bool Find_ConvertedTexture(const std::string& pID_R, const std::string& pID_G, const std::string& pID_B, const std::string& pID_A, uint32_t* pConvertedTextureIndex = nullptr) const
	/// Find texture in list of converted textures. Use at postprocessing step,
	/// \param [in] pID_R - ID of source "red" texture.
	/// \param [in] pID_G - ID of source "green" texture.
	/// \param [in] pID_B - ID of source "blue" texture.
	/// \param [in] pID_A - ID of source "alpha" texture. Use empty string to find RGB-texture.
	/// \param [out] pConvertedTextureIndex - pointer where index in list of found texture will be written. If equivalent to nullptr then nothing will be
	/// written.
	/// \return true - if the texture is found, else - false.
	bool Find_ConvertedTexture(const std::string& pID_R, const std::string& pID_G, const std::string& pID_B, const std::string& pID_A,
								uint32_t* pConvertedTextureIndex = nullptr) const;

	/***********************************************/
	/********* Functions: postprocess set **********/
	/***********************************************/

	/// \fn void PostprocessHelper_CreateMeshDataArray(const CAMFImporter_NodeElement_Mesh& pNodeElement, std::vector<aiVector3D>& pVertexCoordinateArray, std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray) const
	/// Get data stored in <vertices> and place it to arrays.
	/// \param [in] pNodeElement - reference to node element which kept <object> data.
	/// \param [in] pVertexCoordinateArray - reference to vertices coordinates kept in <vertices>.
	/// \param [in] pVertexColorArray - reference to vertices colors for all <vertex's. If color for vertex is not set then corresponding member of array
	/// contain nullptr.
	void PostprocessHelper_CreateMeshDataArray(const CAMFImporter_NodeElement_Mesh& pNodeElement, std::vector<aiVector3D>& pVertexCoordinateArray,
												std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray) const;

	/// \fn size_t PostprocessHelper_GetTextureID_Or_Create(const std::string& pID_R, const std::string& pID_G, const std::string& pID_B, const std::string& pID_A)
	/// Return converted texture ID which related to specified source textures ID's. If converted texture does not exist then it will be created and ID on new
	/// converted texture will be returned. Conversion: set of textures from \ref CAMFImporter_NodeElement_Texture to one \ref SPP_Texture and place it
	/// to converted textures list.
	/// Any of source ID's can be absent(empty string) or even one ID only specified. But at least one ID must be specified.
	/// \param [in] pID_R - ID of source "red" texture.
	/// \param [in] pID_G - ID of source "green" texture.
	/// \param [in] pID_B - ID of source "blue" texture.
	/// \param [in] pID_A - ID of source "alpha" texture.
	/// \return index of the texture in array of the converted textures.
	size_t PostprocessHelper_GetTextureID_Or_Create(const std::string& pID_R, const std::string& pID_G, const std::string& pID_B, const std::string& pID_A);

	/// \fn void PostprocessHelper_SplitFacesByTextureID(std::list<SComplexFace>& pInputList, std::list<std::list<SComplexFace> > pOutputList_Separated)
	/// Separate input list by texture IDs. This step is needed because aiMesh can contain mesh which is use only one texture (or set: diffuse, bump etc).
	/// \param [in] pInputList - input list with faces. Some of them can contain color or texture mapping, or both of them, or nothing. Will be cleared after
	/// processing.
	/// \param [out] pOutputList_Separated - output list of the faces lists. Separated faces list by used texture IDs. Will be cleared before processing.
	void PostprocessHelper_SplitFacesByTextureID(std::list<SComplexFace>& pInputList, std::list<std::list<SComplexFace> >& pOutputList_Separated);

	/// \fn void Postprocess_AddMetadata(const std::list<CAMFImporter_NodeElement_Metadata*>& pMetadataList, aiNode& pSceneNode) const
	/// Check if child elements of node element is metadata and add it to scene node.
	/// \param [in] pMetadataList - reference to list with collected metadata.
	/// \param [out] pSceneNode - scene node in which metadata will be added.
	void Postprocess_AddMetadata(const std::list<CAMFImporter_NodeElement_Metadata*>& pMetadataList, aiNode& pSceneNode) const;

	/// \fn void Postprocess_BuildNodeAndObject(const CAMFImporter_NodeElement_Object& pNodeElement, std::list<aiMesh*>& pMeshList, aiNode** pSceneNode)
	/// To create aiMesh and aiNode for it from <object>.
	/// \param [in] pNodeElement - reference to node element which kept <object> data.
	/// \param [out] pMeshList - reference to a list with all aiMesh of the scene.
	/// \param [out] pSceneNode - pointer to place where new aiNode will be created.
	void Postprocess_BuildNodeAndObject(const CAMFImporter_NodeElement_Object& pNodeElement, std::list<aiMesh*>& pMeshList, aiNode** pSceneNode);

	/// \fn void Postprocess_BuildMeshSet(const CAMFImporter_NodeElement_Mesh& pNodeElement, const std::vector<aiVector3D>& pVertexCoordinateArray, const std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray, const CAMFImporter_NodeElement_Color* pObjectColor, std::list<aiMesh*>& pMeshList, aiNode& pSceneNode)
	/// Create mesh for every <volume> in <mesh>.
	/// \param [in] pNodeElement - reference to node element which kept <mesh> data.
	/// \param [in] pVertexCoordinateArray - reference to vertices coordinates for all <volume>'s.
	/// \param [in] pVertexColorArray - reference to vertices colors for all <volume>'s. If color for vertex is not set then corresponding member of array
	/// contain nullptr.
	/// \param [in] pObjectColor - pointer to colors for <object>. If color is not set then argument contain nullptr.
	/// \param [in] pMaterialList - reference to a list with defined materials.
	/// \param [out] pMeshList - reference to a list with all aiMesh of the scene.
	/// \param [out] pSceneNode - reference to aiNode which will own new aiMesh's.
	void Postprocess_BuildMeshSet(const CAMFImporter_NodeElement_Mesh& pNodeElement, const std::vector<aiVector3D>& pVertexCoordinateArray,
									const std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray, const CAMFImporter_NodeElement_Color* pObjectColor,
									std::list<aiMesh*>& pMeshList, aiNode& pSceneNode);

	/// \fn void Postprocess_BuildMaterial(const CAMFImporter_NodeElement_Material& pMaterial)
	/// Convert material from \ref CAMFImporter_NodeElement_Material to \ref SPP_Material.
	/// \param [in] pMaterial - source CAMFImporter_NodeElement_Material.
	void Postprocess_BuildMaterial(const CAMFImporter_NodeElement_Material& pMaterial);

	/// \fn void Postprocess_BuildConstellation(CAMFImporter_NodeElement_Constellation& pConstellation, std::list<aiNode*>& pNodeList) const
	/// Create and add to aiNode's list new part of scene graph defined by <constellation>.
	/// \param [in] pConstellation - reference to <constellation> node.
	/// \param [out] pNodeList - reference to aiNode's list.
	void Postprocess_BuildConstellation(CAMFImporter_NodeElement_Constellation& pConstellation, std::list<aiNode*>& pNodeList) const;

	/// \fn void Postprocess_BuildScene()
	/// Build Assimp scene graph in aiScene from collected data.
	/// \param [out] pScene - pointer to aiScene where tree will be built.
	void Postprocess_BuildScene(aiScene* pScene);

	/***********************************************/
	/************* Functions: throw set ************/
	/***********************************************/

	/// \fn void Throw_CloseNotFound(const std::string& pNode)
	/// Call that function when close tag of node not found and exception must be raised.
	/// E.g.:
	/// <amf>
	///     <object>
	/// </amf> <!--- object not closed --->
	/// \throw DeadlyImportError.
	/// \param [in] pNode - node name in which exception happened.
	void Throw_CloseNotFound(const std::string& pNode);

	/// \fn void Throw_IncorrectAttr(const std::string& pAttrName)
	/// Call that function when attribute name is incorrect and exception must be raised.
	/// \param [in] pAttrName - attribute name.
	/// \throw DeadlyImportError.
	void Throw_IncorrectAttr(const std::string& pAttrName);

	/// \fn void Throw_IncorrectAttrValue(const std::string& pAttrName)
	/// Call that function when attribute value is incorrect and exception must be raised.
	/// \param [in] pAttrName - attribute name.
	/// \throw DeadlyImportError.
	void Throw_IncorrectAttrValue(const std::string& pAttrName);

	/// \fn void Throw_MoreThanOnceDefined(const std::string& pNode, const std::string& pDescription)
	/// Call that function when some type of nodes are defined twice or more when must be used only once and exception must be raised.
	/// E.g.:
	/// <object>
	///     <color>...    <!--- color defined --->
	///     <color>...    <!--- color defined again --->
	/// </object>
	/// \throw DeadlyImportError.
	/// \param [in] pNodeType - type of node which defined one more time.
	/// \param [in] pDescription - message about error. E.g. what the node defined while exception raised.
	void Throw_MoreThanOnceDefined(const std::string& pNodeType, const std::string& pDescription);

	/// \fn void Throw_ID_NotFound(const std::string& pID) const
	/// Call that function when referenced element ID are not found in graph and exception must be raised.
	/// \param [in] pID - ID of of element which not found.
	/// \throw DeadlyImportError.
	void Throw_ID_NotFound(const std::string& pID) const;

	/***********************************************/
	/************** Functions: LOG set *************/
	/***********************************************/

	/// \fn void LogInfo(const std::string& pMessage)
	/// Short variant for calling \ref DefaultLogger::get()->info()
	void LogInfo(const std::string& pMessage) { DefaultLogger::get()->info(pMessage); }

	/// \fn void LogWarning(const std::string& pMessage)
	/// Short variant for calling \ref DefaultLogger::get()->warn()
	void LogWarning(const std::string& pMessage) { DefaultLogger::get()->warn(pMessage); }

	/// \fn void LogError(const std::string& pMessage)
	/// Short variant for calling \ref DefaultLogger::get()->error()
	void LogError(const std::string& pMessage) { DefaultLogger::get()->error(pMessage); }

	/***********************************************/
	/************** Functions: XML set *************/
	/***********************************************/

	/// \fn void XML_CheckNode_MustHaveChildren()
	/// Check if current node have children: <node>...</node>. If not then exception will throwed.
	void XML_CheckNode_MustHaveChildren();

	/// \fn bool XML_CheckNode_NameEqual(const std::string& pNodeName)
	/// Check if current node name is equal to pNodeName.
	/// \param [in] pNodeName - name for checking.
	/// return true if current node name is equal to pNodeName, else - false.
	bool XML_CheckNode_NameEqual(const std::string& pNodeName) { return mReader->getNodeName() == pNodeName; }

	/// \fn void XML_CheckNode_SkipUnsupported(const std::string& pParentNodeName)
	/// Skip unsupported node and report about that. Depend on node name can be skipped begin tag of node all whole node.
	/// \param [in] pParentNodeName - parent node name. Used for reporting.
	void XML_CheckNode_SkipUnsupported(const std::string& pParentNodeName);

	/// \fn bool XML_SearchNode(const std::string& pNodeName)
	/// Search for specified node in file. XML file read pointer(mReader) will point to found node or file end after search is end.
	/// \param [in] pNodeName - requested node name.
	/// return true - if node is found, else - false.
	bool XML_SearchNode(const std::string& pNodeName);

	/// \fn bool XML_ReadNode_GetAttrVal_AsBool(const int pAttrIdx)
	/// Read attribute value.
	/// \param [in] pAttrIdx - attribute index (\ref mReader->getAttribute* set).
	/// \return read data.
	bool XML_ReadNode_GetAttrVal_AsBool(const int pAttrIdx);

	/// \fn float XML_ReadNode_GetAttrVal_AsFloat(const int pAttrIdx)
	/// Read attribute value.
	/// \param [in] pAttrIdx - attribute index (\ref mReader->getAttribute* set).
	/// \return read data.
	float XML_ReadNode_GetAttrVal_AsFloat(const int pAttrIdx);

	/// \fn uint32_t XML_ReadNode_GetAttrVal_AsU32(const int pAttrIdx)
	/// Read attribute value.
	/// \param [in] pAttrIdx - attribute index (\ref mReader->getAttribute* set).
	/// \return read data.
	uint32_t XML_ReadNode_GetAttrVal_AsU32(const int pAttrIdx);

	/// \fn float XML_ReadNode_GetVal_AsFloat()
	/// Read node value.
	/// \return read data.
	float XML_ReadNode_GetVal_AsFloat();

	/// \fn uint32_t XML_ReadNode_GetVal_AsU32()
	/// Read node value.
	/// \return read data.
	uint32_t XML_ReadNode_GetVal_AsU32();

	/// \fn void XML_ReadNode_GetVal_AsString(std::string& pValue)
	/// Read node value.
	/// \return read data.
	void XML_ReadNode_GetVal_AsString(std::string& pValue);

	/***********************************************/
	/******** Functions: parse set private *********/
	/***********************************************/

	/// \fn void ParseHelper_Node_Enter(CAMFImporter_NodeElement* pNode)
	/// Make pNode as current and enter deeper for parsing child nodes. At end \ref ParseHelper_Node_Exit must be called.
	/// \param [in] pNode - new current node.
	void ParseHelper_Node_Enter(CAMFImporter_NodeElement* pNode);

	/// \fn void ParseHelper_Group_End()
	/// This function must be called when exiting from grouping node. \ref ParseHelper_Group_Begin.
	void ParseHelper_Node_Exit();

	/// \fn void ParseHelper_FixTruncatedFloatString(const char* pInStr, std::string& pOutString)
	/// Attribute values of floating point types can take form ".x"(without leading zero). irrXMLReader can not read this form of values and it
	/// must be converted to right form - "0.xxx".
	/// \param [in] pInStr - pointer to input string which can contain incorrect form of values.
	/// \param [out[ pOutString - output string with right form of values.
	void ParseHelper_FixTruncatedFloatString(const char* pInStr, std::string& pOutString);

	/// \fn void ParseHelper_Decode_Base64(const std::string& pInputBase64, std::vector<uint8_t>& pOutputData) const
	/// Decode Base64-encoded data.
	/// \param [in] pInputBase64 - reference to input Base64-encoded string.
	/// \param [out] pOutputData - reference to output array for decoded data.
	void ParseHelper_Decode_Base64(const std::string& pInputBase64, std::vector<uint8_t>& pOutputData) const;

	/// \fn void ParseNode_Root()
	/// Parse <AMF> node of the file.
	void ParseNode_Root();

	/******** Functions: top nodes *********/

	/// \fn void ParseNode_Constellation()
	/// Parse <constellation> node of the file.
	void ParseNode_Constellation();

	/// \fn void ParseNode_Constellation()
	/// Parse <instance> node of the file.
	void ParseNode_Instance();

	/// \fn void ParseNode_Material()
	/// Parse <material> node of the file.
	void ParseNode_Material();

	/// \fn void ParseNode_Metadata()
	/// Parse <metadata> node.
	void ParseNode_Metadata();

	/// \fn void ParseNode_Object()
	/// Parse <object> node of the file.
	void ParseNode_Object();

	/// \fn void ParseNode_Texture()
	/// Parse <texture> node of the file.
	void ParseNode_Texture();

	/******** Functions: geometry nodes *********/

	/// \fn void ParseNode_Coordinates()
	/// Parse <coordinates> node of the file.
	void ParseNode_Coordinates();

	/// \fn void ParseNode_Edge()
	/// Parse <edge> node of the file.
	void ParseNode_Edge();

	/// \fn void ParseNode_Mesh()
	/// Parse <mesh> node of the file.
	void ParseNode_Mesh();

	/// \fn void ParseNode_Triangle()
	/// Parse <triangle> node of the file.
	void ParseNode_Triangle();

	/// \fn void ParseNode_Vertex()
	/// Parse <vertex> node of the file.
	void ParseNode_Vertex();

	/// \fn void ParseNode_Vertices()
	/// Parse <vertices> node of the file.
	void ParseNode_Vertices();

	/// \fn void ParseNode_Volume()
	/// Parse <volume> node of the file.
	void ParseNode_Volume();

	/******** Functions: material nodes *********/

	/// \fn void ParseNode_Color()
	/// Parse <color> node of the file.
	void ParseNode_Color();

	/// \fn void ParseNode_TexMap(const bool pUseOldName = false)
	/// Parse <texmap> of <map> node of the file.
	/// \param [in] pUseOldName - if true then use old name of node(and children) - <map>, instead of new name - <texmap>.
	void ParseNode_TexMap(const bool pUseOldName = false);

public:

	/// \fn AMFImporter()
	/// Default constructor.
	AMFImporter()
		: mNodeElement_Cur(nullptr), mReader(nullptr)
	{}

	/// \fn ~AMFImporter()
	/// Default destructor.
	~AMFImporter();

	/***********************************************/
	/******** Functions: parse set, public *********/
	/***********************************************/

	/// \fn void ParseFile(const std::string& pFile, IOSystem* pIOHandler)
	/// Parse AMF file and fill scene graph. The function has no return value. Result can be found by analyzing the generated graph.
	/// Also exception can be throwed if trouble will found.
	/// \param [in] pFile - name of file to be parsed.
	/// \param [in] pIOHandler - pointer to IO helper object.
	void ParseFile(const std::string& pFile, IOSystem* pIOHandler);

	/***********************************************/
	/********* Functions: BaseImporter set *********/
	/***********************************************/

	bool CanRead(const std::string& pFile, IOSystem* pIOHandler, bool pCheckSig) const;
	void GetExtensionList(std::set<std::string>& pExtensionList);
	void InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);
	const aiImporterDesc* GetInfo ()const;

};// class AMFImporter

}// namespace Assimp

#endif // INCLUDED_AI_AMF_IMPORTER_H
