/// \file   X3DExporter.cpp
/// \brief  X3D-format files exporter for Assimp. Implementation.
/// \date   2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_X3D_EXPORTER

#include "X3DExporter.hpp"

// Header files, Assimp.
#include <assimp/Exceptional.h>
#include <assimp/StringUtils.h>
#include <assimp/Exporter.hpp>
#include <assimp/IOSystem.hpp>

using namespace std;

namespace Assimp
{

void ExportSceneX3D(const char* pFile, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* pProperties)
{
	X3DExporter exporter(pFile, pIOSystem, pScene, pProperties);
}

}// namespace Assimp

namespace Assimp
{

void X3DExporter::IndentationStringSet(const size_t pNewLevel)
{
	if(pNewLevel > mIndentationString.size())
	{
		if(pNewLevel > mIndentationString.capacity()) mIndentationString.reserve(pNewLevel + 1);

		for(size_t i = 0, i_e = pNewLevel - mIndentationString.size(); i < i_e; i++) mIndentationString.push_back('\t');
	}
	else if(pNewLevel < mIndentationString.size())
	{
		mIndentationString.resize(pNewLevel);
	}
}

void X3DExporter::XML_Write(const string& pData)
{
	if(pData.size() == 0) return;
	if(mOutFile->Write((void*)pData.data(), pData.length(), 1) != 1) throw DeadlyExportError("Failed to write scene data!");
}

aiMatrix4x4 X3DExporter::Matrix_GlobalToCurrent(const aiNode& pNode) const
{
aiNode* cur_node;
std::list<aiMatrix4x4> matr;
aiMatrix4x4 out_matr;

	// starting walk from current element to root
	matr.push_back(pNode.mTransformation);
	cur_node = pNode.mParent;
	if(cur_node != nullptr)
	{
		do
		{
			matr.push_back(cur_node->mTransformation);
			cur_node = cur_node->mParent;
		} while(cur_node != nullptr);
	}

	// multiplicate all matrices in reverse order
	for(std::list<aiMatrix4x4>::reverse_iterator rit = matr.rbegin(); rit != matr.rend(); ++rit) out_matr = out_matr * (*rit);

	return out_matr;
}

void X3DExporter::AttrHelper_FloatToString(const float pValue, std::string& pTargetString)
{
	pTargetString = to_string(pValue);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Vec3DArrToString(const aiVector3D* pArray, const size_t pArray_Size, string& pTargetString)
{
	pTargetString.clear();
	pTargetString.reserve(pArray_Size * 6);// (Number + space) * 3.
	for(size_t idx = 0; idx < pArray_Size; idx++)
		pTargetString.append(to_string(pArray[idx].x) + " " + to_string(pArray[idx].y) + " " + to_string(pArray[idx].z) + " ");

	// remove last space symbol.
	pTargetString.resize(pTargetString.length() - 1);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Vec2DArrToString(const aiVector2D* pArray, const size_t pArray_Size, std::string& pTargetString)
{
	pTargetString.clear();
	pTargetString.reserve(pArray_Size * 4);// (Number + space) * 2.
	for(size_t idx = 0; idx < pArray_Size; idx++)
		pTargetString.append(to_string(pArray[idx].x) + " " + to_string(pArray[idx].y) + " ");

	// remove last space symbol.
	pTargetString.resize(pTargetString.length() - 1);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Vec3DAsVec2fArrToString(const aiVector3D* pArray, const size_t pArray_Size, string& pTargetString)
{
	pTargetString.clear();
	pTargetString.reserve(pArray_Size * 4);// (Number + space) * 2.
	for(size_t idx = 0; idx < pArray_Size; idx++)
		pTargetString.append(to_string(pArray[idx].x) + " " + to_string(pArray[idx].y) + " ");

	// remove last space symbol.
	pTargetString.resize(pTargetString.length() - 1);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Col4DArrToString(const aiColor4D* pArray, const size_t pArray_Size, string& pTargetString)
{
	pTargetString.clear();
	pTargetString.reserve(pArray_Size * 8);// (Number + space) * 4.
	for(size_t idx = 0; idx < pArray_Size; idx++)
		pTargetString.append(to_string(pArray[idx].r) + " " + to_string(pArray[idx].g) + " " + to_string(pArray[idx].b) + " " +
								to_string(pArray[idx].a) + " ");

	// remove last space symbol.
	pTargetString.resize(pTargetString.length() - 1);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Col3DArrToString(const aiColor3D* pArray, const size_t pArray_Size, std::string& pTargetString)
{
	pTargetString.clear();
	pTargetString.reserve(pArray_Size * 6);// (Number + space) * 3.
	for(size_t idx = 0; idx < pArray_Size; idx++)
		pTargetString.append(to_string(pArray[idx].r) + " " + to_string(pArray[idx].g) + " " + to_string(pArray[idx].b) + " ");

	// remove last space symbol.
	pTargetString.resize(pTargetString.length() - 1);
	AttrHelper_CommaToPoint(pTargetString);
}

void X3DExporter::AttrHelper_Color3ToAttrList(std::list<SAttribute>& pList, const std::string& pName, const aiColor3D& pValue, const aiColor3D& pDefaultValue)
{
string tstr;

	if(pValue == pDefaultValue) return;

	AttrHelper_Col3DArrToString(&pValue, 1, tstr);
	pList.push_back({pName, tstr});
}

void X3DExporter::AttrHelper_FloatToAttrList(std::list<SAttribute>& pList, const string& pName, const float pValue, const float pDefaultValue)
{
string tstr;

	if(pValue == pDefaultValue) return;

	AttrHelper_FloatToString(pValue, tstr);
	pList.push_back({pName, tstr});
}

void X3DExporter::NodeHelper_OpenNode(const string& pNodeName, const size_t pTabLevel, const bool pEmptyElement, const list<SAttribute>& pAttrList)
{
	// Write indentation.
	IndentationStringSet(pTabLevel);
	XML_Write(mIndentationString);
	// Begin of the element
	XML_Write("<" + pNodeName);
	// Write attributes
	for(const SAttribute& attr: pAttrList) { XML_Write(" " + attr.Name + "='" + attr.Value + "'"); }

	// End of the element
	if(pEmptyElement)
	{
		XML_Write("/>\n");
	}
	else
	{
		XML_Write(">\n");
	}
}

void X3DExporter::NodeHelper_OpenNode(const string& pNodeName, const size_t pTabLevel, const bool pEmptyElement)
{
const list<SAttribute> attr_list;

	NodeHelper_OpenNode(pNodeName, pTabLevel, pEmptyElement, attr_list);
}

void X3DExporter::NodeHelper_CloseNode(const string& pNodeName, const size_t pTabLevel)
{
	// Write indentation.
	IndentationStringSet(pTabLevel);
	XML_Write(mIndentationString);
	// Write element
	XML_Write("</" + pNodeName + ">\n");
}

void X3DExporter::Export_Node(const aiNode *pNode, const size_t pTabLevel)
{
bool transform = false;
list<SAttribute> attr_list;

	// In Assimp lights is stored in next way: light source store in mScene->mLights and in node tree must present aiNode with name same as
	// light source has. Considering it we must compare every aiNode name with light sources names. Why not to look where ligths is present
	// and save them to fili? Because corresponding aiNode can be already written to file and we can only add information to file not to edit.
	if(CheckAndExport_Light(*pNode, pTabLevel)) return;

	// Check if need DEF.
	if(pNode->mName.length) attr_list.push_back({"DEF", pNode->mName.C_Str()});

	// Check if need <Transformation> node against <Group>.
	if(!pNode->mTransformation.IsIdentity())
	{
		auto Vector2String = [this](const aiVector3D pVector) -> string
		{
			string tstr = to_string(pVector.x) + " " + to_string(pVector.y) + " " + to_string(pVector.z);

			AttrHelper_CommaToPoint(tstr);

			return tstr;
		};

		auto Rotation2String = [this](const aiVector3D pAxis, const ai_real pAngle) -> string
		{
			string tstr = to_string(pAxis.x) + " " + to_string(pAxis.y) + " " + to_string(pAxis.z) + " " + to_string(pAngle);

			AttrHelper_CommaToPoint(tstr);

			return tstr;
		};

		aiVector3D scale, translate, rotate_axis;
		ai_real rotate_angle;

		transform = true;
		pNode->mTransformation.Decompose(scale, rotate_axis, rotate_angle, translate);
		// Check if values different from default
		if((rotate_angle != 0) && (rotate_axis.Length() > 0))
			attr_list.push_back({"rotation", Rotation2String(rotate_axis, rotate_angle)});

        if(!scale.Equal({1.0,1.0,1.0})) {
            attr_list.push_back({"scale", Vector2String(scale)});
        }
        if(translate.Length() > 0) {
            attr_list.push_back({"translation", Vector2String(translate)});
        }
	}

	// Begin node if need.
	if(transform)
		NodeHelper_OpenNode("Transform", pTabLevel, false, attr_list);
	else
		NodeHelper_OpenNode("Group", pTabLevel);

	// Export metadata
	if(pNode->mMetaData != nullptr)
	{
		for(size_t idx_prop = 0; idx_prop < pNode->mMetaData->mNumProperties; idx_prop++)
		{
			const aiString* key;
			const aiMetadataEntry* entry;

			if(pNode->mMetaData->Get(idx_prop, key, entry))
			{
				switch(entry->mType)
				{
					case AI_BOOL:
						Export_MetadataBoolean(*key, *static_cast<bool*>(entry->mData), pTabLevel + 1);
						break;
					case AI_DOUBLE:
						Export_MetadataDouble(*key, *static_cast<double*>(entry->mData), pTabLevel + 1);
						break;
					case AI_FLOAT:
						Export_MetadataFloat(*key, *static_cast<float*>(entry->mData), pTabLevel + 1);
						break;
					case AI_INT32:
						Export_MetadataInteger(*key, *static_cast<int32_t*>(entry->mData), pTabLevel + 1);
						break;
					case AI_AISTRING:
						Export_MetadataString(*key, *static_cast<aiString*>(entry->mData), pTabLevel + 1);
						break;
					default:
						LogError("Unsupported metadata type: " + to_string(entry->mType));
						break;
				}// switch(entry->mType)
			}
		}
	}// if(pNode->mMetaData != nullptr)

	// Export meshes.
	for(size_t idx_mesh = 0; idx_mesh < pNode->mNumMeshes; idx_mesh++) Export_Mesh(pNode->mMeshes[idx_mesh], pTabLevel + 1);
	// Export children.
	for(size_t idx_node = 0; idx_node < pNode->mNumChildren; idx_node++) Export_Node(pNode->mChildren[idx_node], pTabLevel + 1);

	// End node if need.
	if(transform)
		NodeHelper_CloseNode("Transform", pTabLevel);
	else
		NodeHelper_CloseNode("Group", pTabLevel);
}

void X3DExporter::Export_Mesh(const size_t pIdxMesh, const size_t pTabLevel)
{
const char* NodeName_IFS = "IndexedFaceSet";
const char* NodeName_Shape = "Shape";

list<SAttribute> attr_list;
aiMesh& mesh = *mScene->mMeshes[pIdxMesh];// create alias for conveniance.

	// Check if mesh already defined early.
	if(mDEF_Map_Mesh.find(pIdxMesh) != mDEF_Map_Mesh.end())
	{
		// Mesh already defined, just refer to it
		attr_list.push_back({"USE", mDEF_Map_Mesh.at(pIdxMesh)});
		NodeHelper_OpenNode(NodeName_Shape, pTabLevel, true, attr_list);

		return;
	}

	string mesh_name(mesh.mName.C_Str() + string("_IDX_") + to_string(pIdxMesh));// Create mesh name

	// Define mesh name.
	attr_list.push_back({"DEF", mesh_name});
	mDEF_Map_Mesh[pIdxMesh] = mesh_name;

	//
	// "Shape" node.
	//
	NodeHelper_OpenNode(NodeName_Shape, pTabLevel, false, attr_list);
	attr_list.clear();

	//
	// "Appearance" node.
	//
	Export_Material(mesh.mMaterialIndex, pTabLevel + 1);

	//
	// "IndexedFaceSet" node.
	//
	// Fill attributes which differ from default. In Assimp for colors, vertices and normals used one indices set. So, only "coordIndex" must be set.
	string coordIndex;

	// fill coordinates index.
	coordIndex.reserve(mesh.mNumVertices * 4);// Index + space + Face delimiter
	for(size_t idx_face = 0; idx_face < mesh.mNumFaces; idx_face++)
	{
		const aiFace& face_cur = mesh.mFaces[idx_face];

		for(size_t idx_vert = 0; idx_vert < face_cur.mNumIndices; idx_vert++)
		{
			coordIndex.append(to_string(face_cur.mIndices[idx_vert]) + " ");
		}

		coordIndex.append("-1 ");// face delimiter.
	}

	// remove last space symbol.
	coordIndex.resize(coordIndex.length() - 1);
	attr_list.push_back({"coordIndex", coordIndex});
	// create node
	NodeHelper_OpenNode(NodeName_IFS, pTabLevel + 1, false, attr_list);
	attr_list.clear();
	// Child nodes for "IndexedFaceSet" needed when used colors, textures or normals.
	string attr_value;

	// Export <Coordinate>
	AttrHelper_Vec3DArrToString(mesh.mVertices, mesh.mNumVertices, attr_value);
	attr_list.push_back({"point", attr_value});
	NodeHelper_OpenNode("Coordinate", pTabLevel + 2, true, attr_list);
	attr_list.clear();

	// Export <ColorRGBA>
	if(mesh.HasVertexColors(0))
	{
		AttrHelper_Col4DArrToString(mesh.mColors[0], mesh.mNumVertices, attr_value);
		attr_list.push_back({"color", attr_value});
		NodeHelper_OpenNode("ColorRGBA", pTabLevel + 2, true, attr_list);
		attr_list.clear();
	}

	// Export <TextureCoordinate>
	if(mesh.HasTextureCoords(0))
	{
		AttrHelper_Vec3DAsVec2fArrToString(mesh.mTextureCoords[0], mesh.mNumVertices, attr_value);
		attr_list.push_back({"point", attr_value});
		NodeHelper_OpenNode("TextureCoordinate", pTabLevel + 2, true, attr_list);
		attr_list.clear();
	}

	// Export <Normal>
	if(mesh.HasNormals())
	{
		AttrHelper_Vec3DArrToString(mesh.mNormals, mesh.mNumVertices, attr_value);
		attr_list.push_back({"vector", attr_value});
		NodeHelper_OpenNode("Normal", pTabLevel + 2, true, attr_list);
		attr_list.clear();
	}

	//
	// Close opened nodes.
	//
	NodeHelper_CloseNode(NodeName_IFS, pTabLevel + 1);
	NodeHelper_CloseNode(NodeName_Shape, pTabLevel);
}

void X3DExporter::Export_Material(const size_t pIdxMaterial, const size_t pTabLevel)
{
const char* NodeName_A = "Appearance";

list<SAttribute> attr_list;
aiMaterial& material = *mScene->mMaterials[pIdxMaterial];// create alias for conveniance.

	// Check if material already defined early.
	if(mDEF_Map_Material.find(pIdxMaterial) != mDEF_Map_Material.end())
	{
		// Material already defined, just refer to it
		attr_list.push_back({"USE", mDEF_Map_Material.at(pIdxMaterial)});
		NodeHelper_OpenNode(NodeName_A, pTabLevel, true, attr_list);

		return;
	}

	string material_name(string("_IDX_") + to_string(pIdxMaterial));// Create material name
	aiString ai_mat_name;

	if(material.Get(AI_MATKEY_NAME, ai_mat_name) == AI_SUCCESS) material_name.insert(0, ai_mat_name.C_Str());

	// Define material name.
	attr_list.push_back({"DEF", material_name});
	mDEF_Map_Material[pIdxMaterial] = material_name;

	//
	// "Appearance" node.
	//
	NodeHelper_OpenNode(NodeName_A, pTabLevel, false, attr_list);
	attr_list.clear();

	//
	// "Material" node.
	//
	{
		auto Color4ToAttrList = [&](const string& pAttrName, const aiColor4D& pAttrValue, const aiColor3D& pAttrDefaultValue)
		{
			string tstr;

			if(aiColor3D(pAttrValue.r, pAttrValue.g, pAttrValue.b) != pAttrDefaultValue)
			{
				AttrHelper_Col4DArrToString(&pAttrValue, 1, tstr);
				attr_list.push_back({pAttrName, tstr});
			}
		};

		float tvalf;
		aiColor3D color3;
		aiColor4D color4;

		// ambientIntensity="0.2"     SFFloat [inputOutput]
		if(material.Get(AI_MATKEY_COLOR_AMBIENT, color3) == AI_SUCCESS)
			AttrHelper_FloatToAttrList(attr_list, "ambientIntensity", (color3.r + color3.g + color3.b) / 3.0f, 0.2f);
		else if(material.Get(AI_MATKEY_COLOR_AMBIENT, color4) == AI_SUCCESS)
			AttrHelper_FloatToAttrList(attr_list, "ambientIntensity", (color4.r + color4.g + color4.b) / 3.0f, 0.2f);

		// diffuseColor="0.8 0.8 0.8" SFColor [inputOutput]
		if(material.Get(AI_MATKEY_COLOR_DIFFUSE, color3) == AI_SUCCESS)
			AttrHelper_Color3ToAttrList(attr_list, "diffuseColor", color3, aiColor3D(0.8f, 0.8f, 0.8f));
		else if(material.Get(AI_MATKEY_COLOR_DIFFUSE, color4) == AI_SUCCESS)
			Color4ToAttrList("diffuseColor", color4, aiColor3D(0.8f, 0.8f, 0.8f));

		// emissiveColor="0 0 0"      SFColor [inputOutput]
		if(material.Get(AI_MATKEY_COLOR_EMISSIVE, color3) == AI_SUCCESS)
			AttrHelper_Color3ToAttrList(attr_list, "emissiveColor", color3, aiColor3D(0, 0, 0));
		else if(material.Get(AI_MATKEY_COLOR_EMISSIVE, color4) == AI_SUCCESS)
			Color4ToAttrList("emissiveColor", color4, aiColor3D(0, 0, 0));

		// shininess="0.2"            SFFloat [inputOutput]
		if(material.Get(AI_MATKEY_SHININESS, tvalf) == AI_SUCCESS) AttrHelper_FloatToAttrList(attr_list, "shininess", tvalf, 0.2f);

		// specularColor="0 0 0"      SFColor [inputOutput]
		if(material.Get(AI_MATKEY_COLOR_SPECULAR, color3) == AI_SUCCESS)
			AttrHelper_Color3ToAttrList(attr_list, "specularColor", color3, aiColor3D(0, 0, 0));
		else if(material.Get(AI_MATKEY_COLOR_SPECULAR, color4) == AI_SUCCESS)
			Color4ToAttrList("specularColor", color4, aiColor3D(0, 0, 0));

		// transparency="0"           SFFloat [inputOutput]
		if(material.Get(AI_MATKEY_OPACITY, tvalf) == AI_SUCCESS)
		{
			if(tvalf > 1) tvalf = 1;

			tvalf = 1.0f - tvalf;
			AttrHelper_FloatToAttrList(attr_list, "transparency", tvalf, 0);
		}

		NodeHelper_OpenNode("Material", pTabLevel + 1, true, attr_list);
		attr_list.clear();
	}// "Material" node. END.

	//
	// "ImageTexture" node.
	//
	{
		auto RepeatToAttrList = [&](const string& pAttrName, const bool pAttrValue)
		{
			if(!pAttrValue) attr_list.push_back({pAttrName, "false"});
		};

		bool tvalb;
		aiString tstring;

		// url=""         MFString
		if(material.Get(AI_MATKEY_TEXTURE_DIFFUSE(0), tstring) == AI_SUCCESS)
		{
			if(strncmp(tstring.C_Str(), AI_EMBEDDED_TEXNAME_PREFIX, strlen(AI_EMBEDDED_TEXNAME_PREFIX)) == 0)
				LogError("Embedded texture is not supported");
			else
				attr_list.push_back({"url", string("\"") + tstring.C_Str() + "\""});
		}

		// repeatS="true" SFBool
		if(material.Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), tvalb) == AI_SUCCESS) RepeatToAttrList("repeatS", tvalb);

		// repeatT="true" SFBool
		if(material.Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), tvalb) == AI_SUCCESS) RepeatToAttrList("repeatT", tvalb);

		NodeHelper_OpenNode("ImageTexture", pTabLevel + 1, true, attr_list);
		attr_list.clear();
	}// "ImageTexture" node. END.

	//
	// "TextureTransform" node.
	//
	{
		auto Vec2ToAttrList = [&](const string& pAttrName, const aiVector2D& pAttrValue, const aiVector2D& pAttrDefaultValue)
		{
			string tstr;

			if(pAttrValue != pAttrDefaultValue)
			{
				AttrHelper_Vec2DArrToString(&pAttrValue, 1, tstr);
				attr_list.push_back({pAttrName, tstr});
			}
		};

		aiUVTransform transform;

		if(material.Get(AI_MATKEY_UVTRANSFORM_DIFFUSE(0), transform) == AI_SUCCESS)
		{
			Vec2ToAttrList("translation", transform.mTranslation, aiVector2D(0, 0));
			AttrHelper_FloatToAttrList(attr_list, "rotation", transform.mRotation, 0);
			Vec2ToAttrList("scale", transform.mScaling, aiVector2D(1, 1));

			NodeHelper_OpenNode("TextureTransform", pTabLevel + 1, true, attr_list);
			attr_list.clear();
		}
	}// "TextureTransform" node. END.

	//
	// Close opened nodes.
	//
	NodeHelper_CloseNode(NodeName_A, pTabLevel);

}

void X3DExporter::Export_MetadataBoolean(const aiString& pKey, const bool pValue, const size_t pTabLevel)
{
list<SAttribute> attr_list;

	attr_list.push_back({"name", pKey.C_Str()});
	attr_list.push_back({"value", pValue ? "true" : "false"});
	NodeHelper_OpenNode("MetadataBoolean", pTabLevel, true, attr_list);
}

void X3DExporter::Export_MetadataDouble(const aiString& pKey, const double pValue, const size_t pTabLevel)
{
list<SAttribute> attr_list;

	attr_list.push_back({"name", pKey.C_Str()});
	attr_list.push_back({"value", to_string(pValue)});
	NodeHelper_OpenNode("MetadataDouble", pTabLevel, true, attr_list);
}

void X3DExporter::Export_MetadataFloat(const aiString& pKey, const float pValue, const size_t pTabLevel)
{
list<SAttribute> attr_list;

	attr_list.push_back({"name", pKey.C_Str()});
	attr_list.push_back({"value", to_string(pValue)});
	NodeHelper_OpenNode("MetadataFloat", pTabLevel, true, attr_list);
}

void X3DExporter::Export_MetadataInteger(const aiString& pKey, const int32_t pValue, const size_t pTabLevel)
{
list<SAttribute> attr_list;

	attr_list.push_back({"name", pKey.C_Str()});
	attr_list.push_back({"value", to_string(pValue)});
	NodeHelper_OpenNode("MetadataInteger", pTabLevel, true, attr_list);
}

void X3DExporter::Export_MetadataString(const aiString& pKey, const aiString& pValue, const size_t pTabLevel)
{
list<SAttribute> attr_list;

	attr_list.push_back({"name", pKey.C_Str()});
	attr_list.push_back({"value", pValue.C_Str()});
	NodeHelper_OpenNode("MetadataString", pTabLevel, true, attr_list);
}

bool X3DExporter::CheckAndExport_Light(const aiNode& pNode, const size_t pTabLevel)
{
list<SAttribute> attr_list;

auto Vec3ToAttrList = [&](const string& pAttrName, const aiVector3D& pAttrValue, const aiVector3D& pAttrDefaultValue)
{
	string tstr;

	if(pAttrValue != pAttrDefaultValue)
	{
		AttrHelper_Vec3DArrToString(&pAttrValue, 1, tstr);
		attr_list.push_back({pAttrName, tstr});
	}
};

size_t idx_light;
bool found = false;

	// Name of the light source can not be empty.
	if(pNode.mName.length == 0) return false;

	// search for light with name like node has.
	for(idx_light = 0; mScene->mNumLights; idx_light++)
	{
		if(pNode.mName == mScene->mLights[idx_light]->mName)
		{
			found = true;
			break;
		}
	}

	if(!found) return false;

	// Light source is found.
	const aiLight& light = *mScene->mLights[idx_light];// Alias for conveniance.

	aiMatrix4x4 trafo_mat = Matrix_GlobalToCurrent(pNode).Inverse();

	attr_list.push_back({"DEF", light.mName.C_Str()});
	attr_list.push_back({"global", "true"});// "false" is not supported.
	// ambientIntensity="0" SFFloat [inputOutput]
	AttrHelper_FloatToAttrList(attr_list, "ambientIntensity", aiVector3D(light.mColorAmbient.r, light.mColorAmbient.g, light.mColorAmbient.b).Length(), 0);
	// color="1 1 1"        SFColor [inputOutput]
	AttrHelper_Color3ToAttrList(attr_list, "color", light.mColorDiffuse, aiColor3D(1, 1, 1));

	switch(light.mType)
	{
		case aiLightSource_DIRECTIONAL:
			{
				aiVector3D direction = trafo_mat * light.mDirection;

				Vec3ToAttrList("direction", direction, aiVector3D(0, 0, -1));
				NodeHelper_OpenNode("DirectionalLight", pTabLevel, true, attr_list);
			}

			break;
		case aiLightSource_POINT:
			{
				aiVector3D attenuation(light.mAttenuationConstant, light.mAttenuationLinear, light.mAttenuationQuadratic);
				aiVector3D location = trafo_mat * light.mPosition;

				Vec3ToAttrList("attenuation", attenuation, aiVector3D(1, 0, 0));
				Vec3ToAttrList("location", location, aiVector3D(0, 0, 0));
				NodeHelper_OpenNode("PointLight", pTabLevel, true, attr_list);
			}

			break;
		case aiLightSource_SPOT:
			{
				aiVector3D attenuation(light.mAttenuationConstant, light.mAttenuationLinear, light.mAttenuationQuadratic);
				aiVector3D location = trafo_mat * light.mPosition;
				aiVector3D direction = trafo_mat * light.mDirection;

				Vec3ToAttrList("attenuation", attenuation, aiVector3D(1, 0, 0));
				Vec3ToAttrList("location", location, aiVector3D(0, 0, 0));
				Vec3ToAttrList("direction", direction, aiVector3D(0, 0, -1));
				AttrHelper_FloatToAttrList(attr_list, "beamWidth", light.mAngleInnerCone, 0.7854f);
				AttrHelper_FloatToAttrList(attr_list, "cutOffAngle", light.mAngleOuterCone, 1.570796f);
				NodeHelper_OpenNode("SpotLight", pTabLevel, true, attr_list);
			}

			break;
		default:
			throw DeadlyExportError("Unknown light type: " + to_string(light.mType));
	}// switch(light.mType)

	return true;
}

X3DExporter::X3DExporter(const char* pFileName, IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* /*pProperties*/)
	: mScene(pScene)
{
list<SAttribute> attr_list;

	mOutFile = pIOSystem->Open(pFileName, "wt");
	if(mOutFile == nullptr) throw DeadlyExportError("Could not open output .x3d file: " + string(pFileName));

	// Begin document
	XML_Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	XML_Write("<!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.3//EN\" \"http://www.web3d.org/specifications/x3d-3.3.dtd\">\n");
	// Root node
	attr_list.push_back({"profile", "Interchange"});
	attr_list.push_back({"version", "3.3"});
	attr_list.push_back({"xmlns:xsd", "http://www.w3.org/2001/XMLSchema-instance"});
	attr_list.push_back({"xsd:noNamespaceSchemaLocation", "http://www.web3d.org/specifications/x3d-3.3.xsd"});
	NodeHelper_OpenNode("X3D", 0, false, attr_list);
	attr_list.clear();
	// <head>: meta data.
	NodeHelper_OpenNode("head", 1);
	XML_Write(mIndentationString + "<!-- All \"meta\" from this section tou will found in <Scene> node as MetadataString nodes. -->\n");
	NodeHelper_CloseNode("head", 1);
	// Scene node.
	NodeHelper_OpenNode("Scene", 1);
	Export_Node(mScene->mRootNode, 2);
	NodeHelper_CloseNode("Scene", 1);
	// Close Root node.
	NodeHelper_CloseNode("X3D", 0);
	// Cleanup
	pIOSystem->Close(mOutFile);
	mOutFile = nullptr;
}

}// namespace Assimp

#endif // ASSIMP_BUILD_NO_X3D_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
