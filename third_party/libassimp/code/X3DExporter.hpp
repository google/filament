/// \file   X3DExporter.hpp
/// \brief  X3D-format files exporter for Assimp.
/// \date   2016
/// \author smal.root@gmail.com
// Thanks to acorn89 for support.

#ifndef INCLUDED_AI_X3D_EXPORTER_H
#define INCLUDED_AI_X3D_EXPORTER_H

// Header files, Assimp.
#include <assimp/DefaultLogger.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>

// Header files, stdlib.
#include <list>
#include <string>

namespace Assimp
{

/// \class X3DExporter
/// Class which export aiScene to X3D file.
///
/// Limitations.
///
/// Pay attention that X3D is format for interactive graphic and simulations for web browsers. aiScene can not contain all features of the X3D format.
/// Also, aiScene contain rasterized-like data. For example, X3D can describe circle all cylinder with one tag, but aiScene contain result of tesselation:
/// vertices, faces etc. Yes, you can use algorithm for detecting figures or shapes, but that's not a good idea at all.
///
/// Supported nodes:
///		Core component:
///			"MetadataBoolean", "MetadataDouble", "MetadataFloat", "MetadataInteger", "MetadataSet", "MetadataString"
///		Geometry3D component:
///			"IndexedFaceSet"
///		Grouping component:
///			"Group", "Transform"
///		Lighting component:
///			"DirectionalLight", "PointLight", "SpotLight"
///		Rendering component:
///			"ColorRGBA", "Coordinate", "Normal"
///		Shape component:
///			"Shape", "Appearance", "Material"
///		Texturing component:
///			"ImageTexture", "TextureCoordinate", "TextureTransform"
///
class X3DExporter
{
	/***********************************************/
	/******************** Types ********************/
	/***********************************************/

	struct SAttribute
	{
		const std::string Name;
		const std::string Value;
	};

	/***********************************************/
	/****************** Constants ******************/
	/***********************************************/

	const aiScene* const mScene;

	/***********************************************/
	/****************** Variables ******************/
	/***********************************************/

	IOStream* mOutFile;
	std::map<size_t, std::string> mDEF_Map_Mesh;
	std::map<size_t, std::string> mDEF_Map_Material;

private:

	std::string mIndentationString;

	/***********************************************/
	/****************** Functions ******************/
	/***********************************************/

	/// \fn void IndentationStringSet(const size_t pNewLevel)
	/// Set value of the indentation string.
	/// \param [in] pNewLevel - new level of the indentation.
	void IndentationStringSet(const size_t pNewLevel);

	/// \fn void XML_Write(const std::string& pData)
	/// Write data to XML-file.
	/// \param [in] pData - reference to string which must be written.
	void XML_Write(const std::string& pData);

	/// \fn aiMatrix4x4 Matrix_GlobalToCurrent(const aiNode& pNode) const
	/// Calculate transformation matrix for transformation from global coordinate system to pointed aiNode.
	/// \param [in] pNode - reference to local node.
	/// \return calculated matrix.
	aiMatrix4x4 Matrix_GlobalToCurrent(const aiNode& pNode) const;

	/// \fn void AttrHelper_CommaToPoint(std::string& pStringWithComma)
	/// Convert commas in string to points. That's needed because "std::to_string" result depends on locale (regional settings).
	/// \param [in, out] pStringWithComma - reference to string, which must be modified.
	void AttrHelper_CommaToPoint(std::string& pStringWithComma) { for(char& c: pStringWithComma) { if(c == ',') c = '.'; } }

	/// \fn void AttrHelper_FloatToString(const float pValue, std::string& pTargetString)
	/// Converts float to string.
	/// \param [in] pValue - value for converting.
	/// \param [out] pTargetString - reference to string where result will be placed. Will be cleared before using.
	void AttrHelper_FloatToString(const float pValue, std::string& pTargetString);

	/// \fn void AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// Converts array of vectors to string.
	/// \param [in] pArray - pointer to array of vectors.
	/// \param [in] pArray_Size - count of elements in array.
	/// \param [out] pTargetString - reference to string where result will be placed. Will be cleared before using.
	void AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString);

	/// \fn void AttrHelper_Vec2DArrToString(const aiVector2D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// \overload void AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	void AttrHelper_Vec2DArrToString(const aiVector2D* pArray, const size_t pArray_Size, std::string& pTargetString);

	/// \fn void AttrHelper_Vec3DAsVec2fArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// \overload void AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// Only x, y is used from aiVector3D.
	void AttrHelper_Vec3DAsVec2fArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString);

	/// \fn void AttrHelper_Col4DArrToString(const aiColor4D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// \overload void AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// Converts array of colors to string.
	void AttrHelper_Col4DArrToString(const aiColor4D* pArray, const size_t pArray_Size, std::string& pTargetString);

	/// \fn void AttrHelper_Col3DArrToString(const aiColor3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// \overload void AttrHelper_Col4DArrToString(const aiColor4D* pArray, const size_t pArray_Size, std::string& pTargetString)
	/// Converts array of colors to string.
	void AttrHelper_Col3DArrToString(const aiColor3D* pArray, const size_t pArray_Size, std::string& pTargetString);

	/// \fn void AttrHelper_FloatToAttrList(std::list<SAttribute> pList, const std::string& pName, const float pValue, const float pDefaultValue)
	/// \overload void AttrHelper_Col3DArrToString(const aiColor3D* pArray, const size_t pArray_Size, std::string& pTargetString)
	void AttrHelper_FloatToAttrList(std::list<SAttribute> &pList, const std::string& pName, const float pValue, const float pDefaultValue);

	/// \fn void AttrHelper_Color3ToAttrList(std::list<SAttribute> pList, const std::string& pName, const aiColor3D& pValue, const aiColor3D& pDefaultValue)
	/// Add attribute to list if value not equal to default.
	/// \param [in] pList - target list of the attributes.
	/// \param [in] pName - name of new attribute.
	/// \param [in] pValue - value of the new attribute.
	/// \param [in] pDefaultValue - default value for checking: if pValue is equal to pDefaultValue then attribute will not be added.
	void AttrHelper_Color3ToAttrList(std::list<SAttribute> &pList, const std::string& pName, const aiColor3D& pValue, const aiColor3D& pDefaultValue);

	/// \fn void NodeHelper_OpenNode(const std::string& pNodeName, const size_t pTabLevel, const bool pEmptyElement, const std::list<SAttribute>& pAttrList)
	/// Begin new XML-node element.
	/// \param [in] pNodeName - name of the element.
	/// \param [in] pTabLevel - indentation level.
	/// \param [in] pEmtyElement - if true then empty element will be created.
	/// \param [in] pAttrList - list of the attributes for element.
	void NodeHelper_OpenNode(const std::string& pNodeName, const size_t pTabLevel, const bool pEmptyElement, const std::list<SAttribute>& pAttrList);

	/// \fn void NodeHelper_OpenNode(const std::string& pNodeName, const size_t pTabLevel, const bool pEmptyElement = false)
	/// \overload void NodeHelper_OpenNode(const std::string& pNodeName, const size_t pTabLevel, const bool pEmptyElement, const std::list<SAttribute>& pAttrList)
	void NodeHelper_OpenNode(const std::string& pNodeName, const size_t pTabLevel, const bool pEmptyElement = false);

	/// \fn void NodeHelper_CloseNode(const std::string& pNodeName, const size_t pTabLevel)
	/// End XML-node element.
	/// \param [in] pNodeName - name of the element.
	/// \param [in] pTabLevel - indentation level.
	void NodeHelper_CloseNode(const std::string& pNodeName, const size_t pTabLevel);

	/// \fn void Export_Node(const aiNode* pNode, const size_t pTabLevel)
	/// Export data from scene to XML-file: aiNode.
	/// \param [in] pNode - source aiNode.
	/// \param [in] pTabLevel - indentation level.
	void Export_Node(const aiNode* pNode, const size_t pTabLevel);

	/// \fn void Export_Mesh(const size_t pIdxMesh, const size_t pTabLevel)
	/// Export data from scene to XML-file: aiMesh.
	/// \param [in] pMesh - index of the source aiMesh.
	/// \param [in] pTabLevel - indentation level.
	void Export_Mesh(const size_t pIdxMesh, const size_t pTabLevel);

	/// \fn void Export_Material(const size_t pIdxMaterial, const size_t pTabLevel)
	/// Export data from scene to XML-file: aiMaterial.
	/// \param [in] pIdxMaterial - index of the source aiMaterial.
	/// \param [in] pTabLevel - indentation level.
	void Export_Material(const size_t pIdxMaterial, const size_t pTabLevel);

	/// \fn void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
	/// Export data from scene to XML-file: aiMetadata.
	/// \param [in] pKey - source data: value of the metadata key.
	/// \param [in] pValue - source data: value of the metadata value.
	/// \param [in] pTabLevel - indentation level.
	void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel);

	/// \fn void Export_MetadataDouble(const aiString& pKey, const double pValue, const size_t pTabLevel)
	/// \overload void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
	void Export_MetadataDouble(const aiString& pKey, const double pValue, const size_t pTabLevel);

	/// \fn void Export_MetadataFloat(const aiString& pKey, const float pValue, const size_t pTabLevel)
	/// \overload void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
	void Export_MetadataFloat(const aiString& pKey, const float pValue, const size_t pTabLevel);

	/// \fn void Export_MetadataInteger(const aiString& pKey, const int32_t pValue, const size_t pTabLevel)
	/// \overload void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
	void Export_MetadataInteger(const aiString& pKey, const int32_t pValue, const size_t pTabLevel);

	/// \fn void Export_MetadataString(const aiString& pKey, const aiString& pValue, const size_t pTabLevel)
	/// \overload void Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
	void Export_MetadataString(const aiString& pKey, const aiString& pValue, const size_t pTabLevel);

	/// \fn bool CheckAndExport_Light(const aiNode& pNode, const size_t pTabLevel)
	/// Check if node point to light source. If yes then export light source.
	/// \param [in] pNode - reference to node for checking.
	/// \param [in] pTabLevel - indentation level.
	/// \return true - if node assigned with light and it was exported, else - return false.
	bool CheckAndExport_Light(const aiNode& pNode, const size_t pTabLevel);

	/***********************************************/
	/************** Functions: LOG set *************/
	/***********************************************/

	/// \fn void LogError(const std::string& pMessage)
	/// Short variant for calling \ref DefaultLogger::get()->error()
	void LogError(const std::string& pMessage) { DefaultLogger::get()->error(pMessage); }

public:

	/// \fn X3DExporter()
	/// Default constructor.
	X3DExporter(const char* pFileName, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* pProperties);

	/// \fn ~X3DExporter()
	/// Default destructor.
	~X3DExporter() {}

};// class X3DExporter

}// namespace Assimp

#endif // INCLUDED_AI_X3D_EXPORTER_H
