/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team



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

/// \file AMFImporter_Postprocess.cpp
/// \brief Convert built scenegraph and objects to Assimp scenegraph.
/// \date 2016
/// \author smal.root@gmail.com

#ifndef ASSIMP_BUILD_NO_AMF_IMPORTER

#include "AMFImporter.hpp"

// Header files, Assimp.
#include <assimp/SceneCombiner.h>
#include <assimp/StandardShapes.h>
#include <assimp/StringUtils.h>

// Header files, stdlib.
#include <iterator>

namespace Assimp
{

aiColor4D AMFImporter::SPP_Material::GetColor(const float /*pX*/, const float /*pY*/, const float /*pZ*/) const
{
    aiColor4D tcol;

	// Check if stored data are supported.
	if(Composition.size() != 0)
	{
		throw DeadlyImportError("IME. GetColor for composition");
	}
	else if(Color->Composed)
	{
		throw DeadlyImportError("IME. GetColor, composed color");
	}
	else
	{
		tcol = Color->Color;
	}

	// Check if default color must be used
	if((tcol.r == 0) && (tcol.g == 0) && (tcol.b == 0) && (tcol.a == 0))
	{
		tcol.r = 0.5f;
		tcol.g = 0.5f;
		tcol.b = 0.5f;
		tcol.a = 1;
	}

	return tcol;
}

void AMFImporter::PostprocessHelper_CreateMeshDataArray(const CAMFImporter_NodeElement_Mesh& pNodeElement, std::vector<aiVector3D>& pVertexCoordinateArray,
														std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray) const
{
    CAMFImporter_NodeElement_Vertices* vn = nullptr;
    size_t col_idx;

	// All data stored in "vertices", search for it.
	for(CAMFImporter_NodeElement* ne_child: pNodeElement.Child)
	{
		if(ne_child->Type == CAMFImporter_NodeElement::ENET_Vertices) vn = (CAMFImporter_NodeElement_Vertices*)ne_child;
	}

	// If "vertices" not found then no work for us.
	if(vn == nullptr) return;

	pVertexCoordinateArray.reserve(vn->Child.size());// all coordinates stored as child and we need to reserve space for future push_back's.
	pVertexColorArray.resize(vn->Child.size());// colors count equal vertices count.
	col_idx = 0;
	// Inside vertices collect all data and place to arrays
	for(CAMFImporter_NodeElement* vn_child: vn->Child)
	{
		// vertices, colors
		if(vn_child->Type == CAMFImporter_NodeElement::ENET_Vertex)
		{
			// by default clear color for current vertex
			pVertexColorArray[col_idx] = nullptr;

			for(CAMFImporter_NodeElement* vtx: vn_child->Child)
			{
				if(vtx->Type == CAMFImporter_NodeElement::ENET_Coordinates)
				{
					pVertexCoordinateArray.push_back(((CAMFImporter_NodeElement_Coordinates*)vtx)->Coordinate);

					continue;
				}

				if(vtx->Type == CAMFImporter_NodeElement::ENET_Color)
				{
					pVertexColorArray[col_idx] = (CAMFImporter_NodeElement_Color*)vtx;

					continue;
				}
			}// for(CAMFImporter_NodeElement* vtx: vn_child->Child)

			col_idx++;
		}// if(vn_child->Type == CAMFImporter_NodeElement::ENET_Vertex)
	}// for(CAMFImporter_NodeElement* vn_child: vn->Child)
}

size_t AMFImporter::PostprocessHelper_GetTextureID_Or_Create(const std::string& pID_R, const std::string& pID_G, const std::string& pID_B,
																const std::string& pID_A)
{
    size_t TextureConverted_Index;
    std::string TextureConverted_ID;

	// check input data
	if(pID_R.empty() && pID_G.empty() && pID_B.empty() && pID_A.empty())
		throw DeadlyImportError("PostprocessHelper_GetTextureID_Or_Create. At least one texture ID must be defined.");

	// Create ID
	TextureConverted_ID = pID_R + "_" + pID_G + "_" + pID_B + "_" + pID_A;
	// Check if texture specified by set of IDs is converted already.
	TextureConverted_Index = 0;
	for(const SPP_Texture& tex_convd: mTexture_Converted)
	{
        if ( tex_convd.ID == TextureConverted_ID ) {
            return TextureConverted_Index;
        } else {
            ++TextureConverted_Index;
        }
	}

	//
	// Converted texture not found, create it.
	//
	CAMFImporter_NodeElement_Texture* src_texture[4]{nullptr};
	std::vector<CAMFImporter_NodeElement_Texture*> src_texture_4check;
	SPP_Texture converted_texture;

	{// find all specified source textures
		CAMFImporter_NodeElement* t_tex;

		// R
		if(!pID_R.empty())
		{
			if(!Find_NodeElement(pID_R, CAMFImporter_NodeElement::ENET_Texture, &t_tex)) Throw_ID_NotFound(pID_R);

			src_texture[0] = (CAMFImporter_NodeElement_Texture*)t_tex;
			src_texture_4check.push_back((CAMFImporter_NodeElement_Texture*)t_tex);
		}
		else
		{
			src_texture[0] = nullptr;
		}

		// G
		if(!pID_G.empty())
		{
			if(!Find_NodeElement(pID_G, CAMFImporter_NodeElement::ENET_Texture, &t_tex)) Throw_ID_NotFound(pID_G);

			src_texture[1] = (CAMFImporter_NodeElement_Texture*)t_tex;
			src_texture_4check.push_back((CAMFImporter_NodeElement_Texture*)t_tex);
		}
		else
		{
			src_texture[1] = nullptr;
		}

		// B
		if(!pID_B.empty())
		{
			if(!Find_NodeElement(pID_B, CAMFImporter_NodeElement::ENET_Texture, &t_tex)) Throw_ID_NotFound(pID_B);

			src_texture[2] = (CAMFImporter_NodeElement_Texture*)t_tex;
			src_texture_4check.push_back((CAMFImporter_NodeElement_Texture*)t_tex);
		}
		else
		{
			src_texture[2] = nullptr;
		}

		// A
		if(!pID_A.empty())
		{
			if(!Find_NodeElement(pID_A, CAMFImporter_NodeElement::ENET_Texture, &t_tex)) Throw_ID_NotFound(pID_A);

			src_texture[3] = (CAMFImporter_NodeElement_Texture*)t_tex;
			src_texture_4check.push_back((CAMFImporter_NodeElement_Texture*)t_tex);
		}
		else
		{
			src_texture[3] = nullptr;
		}
	}// END: find all specified source textures

	// check that all textures has same size
	if(src_texture_4check.size() > 1)
	{
		for (size_t i = 0, i_e = (src_texture_4check.size() - 1); i < i_e; i++)
		{
			if((src_texture_4check[i]->Width != src_texture_4check[i + 1]->Width) || (src_texture_4check[i]->Height != src_texture_4check[i + 1]->Height) ||
				(src_texture_4check[i]->Depth != src_texture_4check[i + 1]->Depth))
			{
				throw DeadlyImportError("PostprocessHelper_GetTextureID_Or_Create. Source texture must has the same size.");
			}
		}
	}// if(src_texture_4check.size() > 1)

	// set texture attributes
	converted_texture.Width = src_texture_4check[0]->Width;
	converted_texture.Height = src_texture_4check[0]->Height;
	converted_texture.Depth = src_texture_4check[0]->Depth;
	// if one of source texture is tiled then converted texture is tiled too.
	converted_texture.Tiled = false;
	for(uint8_t i = 0; i < src_texture_4check.size(); i++) converted_texture.Tiled |= src_texture_4check[i]->Tiled;

	// Create format hint.
	strcpy(converted_texture.FormatHint, "rgba0000");// copy initial string.
	if(!pID_R.empty()) converted_texture.FormatHint[4] = '8';
	if(!pID_G.empty()) converted_texture.FormatHint[5] = '8';
	if(!pID_B.empty()) converted_texture.FormatHint[6] = '8';
	if(!pID_A.empty()) converted_texture.FormatHint[7] = '8';

	//
	// Сopy data of textures.
	//
	size_t tex_size = 0;
	size_t step = 0;
	size_t off_g = 0;
	size_t off_b = 0;

	// Calculate size of the target array and rule how data will be copied.
    if(!pID_R.empty() && nullptr != src_texture[ 0 ] ) {
        tex_size += src_texture[0]->Data.size(); step++, off_g++, off_b++;
    }
    if(!pID_G.empty() && nullptr != src_texture[ 1 ] ) {
        tex_size += src_texture[1]->Data.size(); step++, off_b++;
    }
    if(!pID_B.empty() && nullptr != src_texture[ 2 ] ) {
        tex_size += src_texture[2]->Data.size(); step++;
    }
    if(!pID_A.empty() && nullptr != src_texture[ 3 ] ) {
        tex_size += src_texture[3]->Data.size(); step++;
    }

    // Create target array.
	converted_texture.Data = new uint8_t[tex_size];
	// And copy data
	auto CopyTextureData = [&](const std::string& pID, const size_t pOffset, const size_t pStep, const uint8_t pSrcTexNum) -> void
	{
		if(!pID.empty())
		{
			for(size_t idx_target = pOffset, idx_src = 0; idx_target < tex_size; idx_target += pStep, idx_src++) {
				CAMFImporter_NodeElement_Texture* tex = src_texture[pSrcTexNum];
				ai_assert(tex);
				converted_texture.Data[idx_target] = tex->Data.at(idx_src);
			}
		}
	};// auto CopyTextureData = [&](const size_t pOffset, const size_t pStep, const uint8_t pSrcTexNum) -> void

	CopyTextureData(pID_R, 0, step, 0);
	CopyTextureData(pID_G, off_g, step, 1);
	CopyTextureData(pID_B, off_b, step, 2);
	CopyTextureData(pID_A, step - 1, step, 3);

	// Store new converted texture ID
	converted_texture.ID = TextureConverted_ID;
	// Store new converted texture
	mTexture_Converted.push_back(converted_texture);

	return TextureConverted_Index;
}

void AMFImporter::PostprocessHelper_SplitFacesByTextureID(std::list<SComplexFace>& pInputList, std::list<std::list<SComplexFace> >& pOutputList_Separated)
{
    auto texmap_is_equal = [](const CAMFImporter_NodeElement_TexMap* pTexMap1, const CAMFImporter_NodeElement_TexMap* pTexMap2) -> bool
    {
	    if((pTexMap1 == nullptr) && (pTexMap2 == nullptr)) return true;
	    if(pTexMap1 == nullptr) return false;
	    if(pTexMap2 == nullptr) return false;

	    if(pTexMap1->TextureID_R != pTexMap2->TextureID_R) return false;
	    if(pTexMap1->TextureID_G != pTexMap2->TextureID_G) return false;
	    if(pTexMap1->TextureID_B != pTexMap2->TextureID_B) return false;
	    if(pTexMap1->TextureID_A != pTexMap2->TextureID_A) return false;

	    return true;
    };

	pOutputList_Separated.clear();
	if(pInputList.size() == 0) return;

	do
	{
		SComplexFace face_start = pInputList.front();
		std::list<SComplexFace> face_list_cur;

		for(std::list<SComplexFace>::iterator it = pInputList.begin(), it_end = pInputList.end(); it != it_end;)
		{
			if(texmap_is_equal(face_start.TexMap, it->TexMap))
			{
				auto it_old = it;

				it++;
				face_list_cur.push_back(*it_old);
				pInputList.erase(it_old);
			}
			else
			{
				it++;
			}
		}

		if(face_list_cur.size() > 0) pOutputList_Separated.push_back(face_list_cur);

	} while(pInputList.size() > 0);
}

void AMFImporter::Postprocess_AddMetadata(const std::list<CAMFImporter_NodeElement_Metadata*>& metadataList, aiNode& sceneNode) const
{
	if ( !metadataList.empty() )
	{
		if(sceneNode.mMetaData != nullptr) throw DeadlyImportError("Postprocess. MetaData member in node are not nullptr. Something went wrong.");

		// copy collected metadata to output node.
        sceneNode.mMetaData = aiMetadata::Alloc( static_cast<unsigned int>(metadataList.size()) );
		size_t meta_idx( 0 );

		for(const CAMFImporter_NodeElement_Metadata& metadata: metadataList)
		{
			sceneNode.mMetaData->Set(static_cast<unsigned int>(meta_idx++), metadata.Type, aiString(metadata.Value));
		}
	}// if(!metadataList.empty())
}

void AMFImporter::Postprocess_BuildNodeAndObject(const CAMFImporter_NodeElement_Object& pNodeElement, std::list<aiMesh*>& pMeshList, aiNode** pSceneNode)
{
CAMFImporter_NodeElement_Color* object_color = nullptr;

	// create new aiNode and set name as <object> has.
	*pSceneNode = new aiNode;
	(*pSceneNode)->mName = pNodeElement.ID;
	// read mesh and color
	for(const CAMFImporter_NodeElement* ne_child: pNodeElement.Child)
	{
		std::vector<aiVector3D> vertex_arr;
		std::vector<CAMFImporter_NodeElement_Color*> color_arr;

		// color for object
		if(ne_child->Type == CAMFImporter_NodeElement::ENET_Color) object_color = (CAMFImporter_NodeElement_Color*)ne_child;

		if(ne_child->Type == CAMFImporter_NodeElement::ENET_Mesh)
		{
			// Create arrays from children of mesh: vertices.
			PostprocessHelper_CreateMeshDataArray(*((CAMFImporter_NodeElement_Mesh*)ne_child), vertex_arr, color_arr);
			// Use this arrays as a source when creating every aiMesh
			Postprocess_BuildMeshSet(*((CAMFImporter_NodeElement_Mesh*)ne_child), vertex_arr, color_arr, object_color, pMeshList, **pSceneNode);
		}
	}// for(const CAMFImporter_NodeElement* ne_child: pNodeElement)
}

void AMFImporter::Postprocess_BuildMeshSet(const CAMFImporter_NodeElement_Mesh& pNodeElement, const std::vector<aiVector3D>& pVertexCoordinateArray,
											const std::vector<CAMFImporter_NodeElement_Color*>& pVertexColorArray,
											const CAMFImporter_NodeElement_Color* pObjectColor, std::list<aiMesh*>& pMeshList, aiNode& pSceneNode)
{
std::list<unsigned int> mesh_idx;

	// all data stored in "volume", search for it.
	for(const CAMFImporter_NodeElement* ne_child: pNodeElement.Child)
	{
		const CAMFImporter_NodeElement_Color* ne_volume_color = nullptr;
		const SPP_Material* cur_mat = nullptr;

		if(ne_child->Type == CAMFImporter_NodeElement::ENET_Volume)
		{
			/******************* Get faces *******************/
			const CAMFImporter_NodeElement_Volume* ne_volume = reinterpret_cast<const CAMFImporter_NodeElement_Volume*>(ne_child);

			std::list<SComplexFace> complex_faces_list;// List of the faces of the volume.
			std::list<std::list<SComplexFace> > complex_faces_toplist;// List of the face list for every mesh.

			// check if volume use material
			if(!ne_volume->MaterialID.empty())
			{
				if(!Find_ConvertedMaterial(ne_volume->MaterialID, &cur_mat)) Throw_ID_NotFound(ne_volume->MaterialID);
			}

			// inside "volume" collect all data and place to arrays or create new objects
			for(const CAMFImporter_NodeElement* ne_volume_child: ne_volume->Child)
			{
				// color for volume
				if(ne_volume_child->Type == CAMFImporter_NodeElement::ENET_Color)
				{
					ne_volume_color = reinterpret_cast<const CAMFImporter_NodeElement_Color*>(ne_volume_child);
				}
				else if(ne_volume_child->Type == CAMFImporter_NodeElement::ENET_Triangle)// triangles, triangles colors
				{
					const CAMFImporter_NodeElement_Triangle& tri_al = *reinterpret_cast<const CAMFImporter_NodeElement_Triangle*>(ne_volume_child);

					SComplexFace complex_face;

					// initialize pointers
					complex_face.Color = nullptr;
					complex_face.TexMap = nullptr;
					// get data from triangle children: color, texture coordinates.
					if(tri_al.Child.size())
					{
						for(const CAMFImporter_NodeElement* ne_triangle_child: tri_al.Child)
						{
							if(ne_triangle_child->Type == CAMFImporter_NodeElement::ENET_Color)
								complex_face.Color = reinterpret_cast<const CAMFImporter_NodeElement_Color*>(ne_triangle_child);
							else if(ne_triangle_child->Type == CAMFImporter_NodeElement::ENET_TexMap)
								complex_face.TexMap = reinterpret_cast<const CAMFImporter_NodeElement_TexMap*>(ne_triangle_child);
						}
					}// if(tri_al.Child.size())

					// create new face and store it.
					complex_face.Face.mNumIndices = 3;
					complex_face.Face.mIndices = new unsigned int[3];
					complex_face.Face.mIndices[0] = static_cast<unsigned int>(tri_al.V[0]);
					complex_face.Face.mIndices[1] = static_cast<unsigned int>(tri_al.V[1]);
					complex_face.Face.mIndices[2] = static_cast<unsigned int>(tri_al.V[2]);
					complex_faces_list.push_back(complex_face);
				}
			}// for(const CAMFImporter_NodeElement* ne_volume_child: ne_volume->Child)

			/**** Split faces list: one list per mesh ****/
			PostprocessHelper_SplitFacesByTextureID(complex_faces_list, complex_faces_toplist);

			/***** Create mesh for every faces list ******/
			for(std::list<SComplexFace>& face_list_cur: complex_faces_toplist)
			{
				auto VertexIndex_GetMinimal = [](const std::list<SComplexFace>& pFaceList, const size_t* pBiggerThan) -> size_t
				{
					size_t rv;

					if(pBiggerThan != nullptr)
					{
						bool found = false;

						for(const SComplexFace& face: pFaceList)
						{
							for(size_t idx_vert = 0; idx_vert < face.Face.mNumIndices; idx_vert++)
							{
								if(face.Face.mIndices[idx_vert] > *pBiggerThan)
								{
									rv = face.Face.mIndices[idx_vert];
									found = true;

									break;
								}
							}

							if(found) break;
						}

						if(!found) return *pBiggerThan;
					}
					else
					{
						rv = pFaceList.front().Face.mIndices[0];
					}// if(pBiggerThan != nullptr) else

					for(const SComplexFace& face: pFaceList)
					{
						for(size_t vi = 0; vi < face.Face.mNumIndices; vi++)
						{
							if(face.Face.mIndices[vi] < rv)
							{
								if(pBiggerThan != nullptr)
								{
									if(face.Face.mIndices[vi] > *pBiggerThan) rv = face.Face.mIndices[vi];
								}
								else
								{
									rv = face.Face.mIndices[vi];
								}
							}
						}
					}// for(const SComplexFace& face: pFaceList)

					return rv;
				};// auto VertexIndex_GetMinimal = [](const std::list<SComplexFace>& pFaceList, const size_t* pBiggerThan) -> size_t

				auto VertexIndex_Replace = [](std::list<SComplexFace>& pFaceList, const size_t pIdx_From, const size_t pIdx_To) -> void
				{
					for(const SComplexFace& face: pFaceList)
					{
						for(size_t vi = 0; vi < face.Face.mNumIndices; vi++)
						{
							if(face.Face.mIndices[vi] == pIdx_From) face.Face.mIndices[vi] = static_cast<unsigned int>(pIdx_To);
						}
					}
				};// auto VertexIndex_Replace = [](std::list<SComplexFace>& pFaceList, const size_t pIdx_From, const size_t pIdx_To) -> void

				auto Vertex_CalculateColor = [&](const size_t pIdx) -> aiColor4D
				{
					// Color priorities(In descending order):
					// 1. triangle color;
					// 2. vertex color;
					// 3. volume color;
					// 4. object color;
					// 5. material;
					// 6. default - invisible coat.
					//
					// Fill vertices colors in color priority list above that's points from 1 to 6.
					if((pIdx < pVertexColorArray.size()) && (pVertexColorArray[pIdx] != nullptr))// check for vertex color
					{
						if(pVertexColorArray[pIdx]->Composed)
							throw DeadlyImportError("IME: vertex color composed");
						else
							return pVertexColorArray[pIdx]->Color;
					}
					else if(ne_volume_color != nullptr)// check for volume color
					{
						if(ne_volume_color->Composed)
							throw DeadlyImportError("IME: volume color composed");
						else
							return ne_volume_color->Color;
					}
					else if(pObjectColor != nullptr)// check for object color
					{
						if(pObjectColor->Composed)
							throw DeadlyImportError("IME: object color composed");
						else
							return pObjectColor->Color;
					}
					else if(cur_mat != nullptr)// check for material
					{
						return cur_mat->GetColor(pVertexCoordinateArray.at(pIdx).x, pVertexCoordinateArray.at(pIdx).y, pVertexCoordinateArray.at(pIdx).z);
					}
					else// set default color.
					{
						return {0, 0, 0, 0};
					}// if((vi < pVertexColorArray.size()) && (pVertexColorArray[vi] != nullptr)) else

				};// auto Vertex_CalculateColor = [&](const size_t pIdx) -> aiColor4D

				aiMesh* tmesh = new aiMesh;

				tmesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;// Only triangles is supported by AMF.
				//
				// set geometry and colors (vertices)
				//
				// copy faces/triangles
				tmesh->mNumFaces = static_cast<unsigned int>(face_list_cur.size());
				tmesh->mFaces = new aiFace[tmesh->mNumFaces];

				// Create vertices list and optimize indices. Optimisation mean following.In AMF all volumes use one big list of vertices. And one volume
				// can use only part of vertices list, for example: vertices list contain few thousands of vertices and volume use vertices 1, 3, 10.
				// Do you need all this thousands of garbage? Of course no. So, optimisation step transformate sparse indices set to continuous.
				size_t VertexCount_Max = tmesh->mNumFaces * 3;// 3 - triangles.
				std::vector<aiVector3D> vert_arr, texcoord_arr;
				std::vector<aiColor4D> col_arr;

				vert_arr.reserve(VertexCount_Max * 2);// "* 2" - see below TODO.
				col_arr.reserve(VertexCount_Max * 2);

				{// fill arrays
					size_t vert_idx_from, vert_idx_to;

					// first iteration.
					vert_idx_to = 0;
					vert_idx_from = VertexIndex_GetMinimal(face_list_cur, nullptr);
					vert_arr.push_back(pVertexCoordinateArray.at(vert_idx_from));
					col_arr.push_back(Vertex_CalculateColor(vert_idx_from));
					if(vert_idx_from != vert_idx_to) VertexIndex_Replace(face_list_cur, vert_idx_from, vert_idx_to);

					// rest iterations
					do
					{
						vert_idx_from = VertexIndex_GetMinimal(face_list_cur, &vert_idx_to);
						if(vert_idx_from == vert_idx_to) break;// all indices are transferred,

						vert_arr.push_back(pVertexCoordinateArray.at(vert_idx_from));
						col_arr.push_back(Vertex_CalculateColor(vert_idx_from));
						vert_idx_to++;
						if(vert_idx_from != vert_idx_to) VertexIndex_Replace(face_list_cur, vert_idx_from, vert_idx_to);

					} while(true);
				}// fill arrays. END.

				//
				// check if triangle colors are used and create additional faces if needed.
				//
				for(const SComplexFace& face_cur: face_list_cur)
				{
					if(face_cur.Color != nullptr)
					{
						aiColor4D face_color;
						size_t vert_idx_new = vert_arr.size();

						if(face_cur.Color->Composed)
							throw DeadlyImportError("IME: face color composed");
						else
							face_color = face_cur.Color->Color;

						for(size_t idx_ind = 0; idx_ind < face_cur.Face.mNumIndices; idx_ind++)
						{
							vert_arr.push_back(vert_arr.at(face_cur.Face.mIndices[idx_ind]));
							col_arr.push_back(face_color);
							face_cur.Face.mIndices[idx_ind] = static_cast<unsigned int>(vert_idx_new++);
						}
					}// if(face_cur.Color != nullptr)
				}// for(const SComplexFace& face_cur: face_list_cur)

				//
				// if texture is used then copy texture coordinates too.
				//
				if(face_list_cur.front().TexMap != nullptr)
				{
					size_t idx_vert_new = vert_arr.size();
					///TODO: clean unused vertices. "* 2": in certain cases - mesh full of triangle colors - vert_arr will contain duplicated vertices for
					/// colored triangles and initial vertices (for colored vertices) which in real became unused. This part need more thinking about
					/// optimisation.
					bool* idx_vert_used;

					idx_vert_used = new bool[VertexCount_Max * 2];
					for(size_t i = 0, i_e = VertexCount_Max * 2; i < i_e; i++) idx_vert_used[i] = false;

					// This ID's will be used when set materials ID in scene.
					tmesh->mMaterialIndex = static_cast<unsigned int>(PostprocessHelper_GetTextureID_Or_Create(face_list_cur.front().TexMap->TextureID_R,
																						face_list_cur.front().TexMap->TextureID_G,
																						face_list_cur.front().TexMap->TextureID_B,
																						face_list_cur.front().TexMap->TextureID_A));
					texcoord_arr.resize(VertexCount_Max * 2);
					for(const SComplexFace& face_cur: face_list_cur)
					{
						for(size_t idx_ind = 0; idx_ind < face_cur.Face.mNumIndices; idx_ind++)
						{
							const size_t idx_vert = face_cur.Face.mIndices[idx_ind];

							if(!idx_vert_used[idx_vert])
							{
								texcoord_arr.at(idx_vert) = face_cur.TexMap->TextureCoordinate[idx_ind];
								idx_vert_used[idx_vert] = true;
							}
							else if(texcoord_arr.at(idx_vert) != face_cur.TexMap->TextureCoordinate[idx_ind])
							{
								// in that case one vertex is shared with many texture coordinates. We need to duplicate vertex with another texture
								// coordinates.
								vert_arr.push_back(vert_arr.at(idx_vert));
								col_arr.push_back(col_arr.at(idx_vert));
								texcoord_arr.at(idx_vert_new) = face_cur.TexMap->TextureCoordinate[idx_ind];
								face_cur.Face.mIndices[idx_ind] = static_cast<unsigned int>(idx_vert_new++);
							}
						}// for(size_t idx_ind = 0; idx_ind < face_cur.Face.mNumIndices; idx_ind++)
					}// for(const SComplexFace& face_cur: face_list_cur)

					delete [] idx_vert_used;
					// shrink array
					texcoord_arr.resize(idx_vert_new);
				}// if(face_list_cur.front().TexMap != nullptr)

				//
				// copy collected data to mesh
				//
				tmesh->mNumVertices = static_cast<unsigned int>(vert_arr.size());
				tmesh->mVertices = new aiVector3D[tmesh->mNumVertices];
				tmesh->mColors[0] = new aiColor4D[tmesh->mNumVertices];

				memcpy(tmesh->mVertices, vert_arr.data(), tmesh->mNumVertices * sizeof(aiVector3D));
				memcpy(tmesh->mColors[0], col_arr.data(), tmesh->mNumVertices * sizeof(aiColor4D));
				if(texcoord_arr.size() > 0)
				{
					tmesh->mTextureCoords[0] = new aiVector3D[tmesh->mNumVertices];
					memcpy(tmesh->mTextureCoords[0], texcoord_arr.data(), tmesh->mNumVertices * sizeof(aiVector3D));
					tmesh->mNumUVComponents[0] = 2;// U and V stored in "x", "y" of aiVector3D.
				}

				size_t idx_face = 0;
				for(const SComplexFace& face_cur: face_list_cur) tmesh->mFaces[idx_face++] = face_cur.Face;

				// store new aiMesh
				mesh_idx.push_back(static_cast<unsigned int>(pMeshList.size()));
				pMeshList.push_back(tmesh);
			}// for(const std::list<SComplexFace>& face_list_cur: complex_faces_toplist)
		}// if(ne_child->Type == CAMFImporter_NodeElement::ENET_Volume)
	}// for(const CAMFImporter_NodeElement* ne_child: pNodeElement.Child)

	// if meshes was created then assign new indices with current aiNode
	if(mesh_idx.size() > 0)
	{
		std::list<unsigned int>::const_iterator mit = mesh_idx.begin();

		pSceneNode.mNumMeshes = static_cast<unsigned int>(mesh_idx.size());
		pSceneNode.mMeshes = new unsigned int[pSceneNode.mNumMeshes];
		for(size_t i = 0; i < pSceneNode.mNumMeshes; i++) pSceneNode.mMeshes[i] = *mit++;
	}// if(mesh_idx.size() > 0)
}

void AMFImporter::Postprocess_BuildMaterial(const CAMFImporter_NodeElement_Material& pMaterial)
{
SPP_Material new_mat;

	new_mat.ID = pMaterial.ID;
	for(const CAMFImporter_NodeElement* mat_child: pMaterial.Child)
	{
		if(mat_child->Type == CAMFImporter_NodeElement::ENET_Color)
		{
			new_mat.Color = (CAMFImporter_NodeElement_Color*)mat_child;
		}
		else if(mat_child->Type == CAMFImporter_NodeElement::ENET_Metadata)
		{
			new_mat.Metadata.push_back((CAMFImporter_NodeElement_Metadata*)mat_child);
		}
	}// for(const CAMFImporter_NodeElement* mat_child; pMaterial.Child)

	// place converted material to special list
	mMaterial_Converted.push_back(new_mat);
}

void AMFImporter::Postprocess_BuildConstellation(CAMFImporter_NodeElement_Constellation& pConstellation, std::list<aiNode*>& pNodeList) const
{
aiNode* con_node;
std::list<aiNode*> ch_node;

	// We will build next hierarchy:
	// aiNode as parent (<constellation>) for set of nodes as a children
	//  |- aiNode for transformation (<instance> -> <delta...>, <r...>) - aiNode for pointing to object ("objectid")
	//  ...
	//  \_ aiNode for transformation (<instance> -> <delta...>, <r...>) - aiNode for pointing to object ("objectid")
	con_node = new aiNode;
	con_node->mName = pConstellation.ID;
	// Walk through children and search for instances of another objects, constellations.
	for(const CAMFImporter_NodeElement* ne: pConstellation.Child)
	{
		aiMatrix4x4 tmat;
		aiNode* t_node;
		aiNode* found_node;

		if(ne->Type == CAMFImporter_NodeElement::ENET_Metadata) continue;
		if(ne->Type != CAMFImporter_NodeElement::ENET_Instance) throw DeadlyImportError("Only <instance> nodes can be in <constellation>.");

		// create alias for conveniance
		CAMFImporter_NodeElement_Instance& als = *((CAMFImporter_NodeElement_Instance*)ne);
		// find referenced object
		if(!Find_ConvertedNode(als.ObjectID, pNodeList, &found_node)) Throw_ID_NotFound(als.ObjectID);

		// create node for applying transformation
		t_node = new aiNode;
		t_node->mParent = con_node;
		// apply transformation
		aiMatrix4x4::Translation(als.Delta, tmat), t_node->mTransformation *= tmat;
		aiMatrix4x4::RotationX(als.Rotation.x, tmat), t_node->mTransformation *= tmat;
		aiMatrix4x4::RotationY(als.Rotation.y, tmat), t_node->mTransformation *= tmat;
		aiMatrix4x4::RotationZ(als.Rotation.z, tmat), t_node->mTransformation *= tmat;
		// create array for one child node
		t_node->mNumChildren = 1;
		t_node->mChildren = new aiNode*[t_node->mNumChildren];
		SceneCombiner::Copy(&t_node->mChildren[0], found_node);
		t_node->mChildren[0]->mParent = t_node;
		ch_node.push_back(t_node);
	}// for(const CAMFImporter_NodeElement* ne: pConstellation.Child)

	// copy found aiNode's as children
	if(ch_node.size() == 0) throw DeadlyImportError("<constellation> must have at least one <instance>.");

	size_t ch_idx = 0;

	con_node->mNumChildren = static_cast<unsigned int>(ch_node.size());
	con_node->mChildren = new aiNode*[con_node->mNumChildren];
	for(aiNode* node: ch_node) con_node->mChildren[ch_idx++] = node;

	// and place "root" of <constellation> node to node list
	pNodeList.push_back(con_node);
}

void AMFImporter::Postprocess_BuildScene(aiScene* pScene)
{
std::list<aiNode*> node_list;
std::list<aiMesh*> mesh_list;
std::list<CAMFImporter_NodeElement_Metadata*> meta_list;

	//
	// Because for AMF "material" is just complex colors mixing so aiMaterial will not be used.
	// For building aiScene we are must to do few steps:
	// at first creating root node for aiScene.
	pScene->mRootNode = new aiNode;
	pScene->mRootNode->mParent = nullptr;
	pScene->mFlags |= AI_SCENE_FLAGS_ALLOW_SHARED;
	// search for root(<amf>) element
	CAMFImporter_NodeElement* root_el = nullptr;

	for(CAMFImporter_NodeElement* ne: mNodeElement_List)
	{
		if(ne->Type != CAMFImporter_NodeElement::ENET_Root) continue;

		root_el = ne;

		break;
	}// for(const CAMFImporter_NodeElement* ne: mNodeElement_List)

	// Check if root element are found.
	if(root_el == nullptr) throw DeadlyImportError("Root(<amf>) element not found.");

	// after that walk through children of root and collect data. Five types of nodes can be placed at top level - in <amf>: <object>, <material>, <texture>,
	// <constellation> and <metadata>. But at first we must read <material> and <texture> because they will be used in <object>. <metadata> can be read
	// at any moment.
	//
	// 1. <material>
	// 2. <texture> will be converted later when processing triangles list. \sa Postprocess_BuildMeshSet
	for(const CAMFImporter_NodeElement* root_child: root_el->Child)
	{
		if(root_child->Type == CAMFImporter_NodeElement::ENET_Material) Postprocess_BuildMaterial(*((CAMFImporter_NodeElement_Material*)root_child));
	}

	// After "appearance" nodes we must read <object> because it will be used in <constellation> -> <instance>.
	//
	// 3. <object>
	for(const CAMFImporter_NodeElement* root_child: root_el->Child)
	{
		if(root_child->Type == CAMFImporter_NodeElement::ENET_Object)
		{
			aiNode* tnode = nullptr;

			// for <object> mesh and node must be built: object ID assigned to aiNode name and will be used in future for <instance>
			Postprocess_BuildNodeAndObject(*((CAMFImporter_NodeElement_Object*)root_child), mesh_list, &tnode);
			if(tnode != nullptr) node_list.push_back(tnode);

		}
	}// for(const CAMFImporter_NodeElement* root_child: root_el->Child)

	// And finally read rest of nodes.
	//
	for(const CAMFImporter_NodeElement* root_child: root_el->Child)
	{
		// 4. <constellation>
		if(root_child->Type == CAMFImporter_NodeElement::ENET_Constellation)
		{
			// <object> and <constellation> at top of self abstraction use aiNode. So we can use only aiNode list for creating new aiNode's.
			Postprocess_BuildConstellation(*((CAMFImporter_NodeElement_Constellation*)root_child), node_list);
		}

		// 5, <metadata>
		if(root_child->Type == CAMFImporter_NodeElement::ENET_Metadata) meta_list.push_back((CAMFImporter_NodeElement_Metadata*)root_child);
	}// for(const CAMFImporter_NodeElement* root_child: root_el->Child)

	// at now we can add collected metadata to root node
	Postprocess_AddMetadata(meta_list, *pScene->mRootNode);
	//
	// Check constellation children
	//
	// As said in specification:
	// "When multiple objects and constellations are defined in a single file, only the top level objects and constellations are available for printing."
	// What that means? For example: if some object is used in constellation then you must show only constellation but not original object.
	// And at this step we are checking that relations.
nl_clean_loop:

	if(node_list.size() > 1)
	{
		// walk through all nodes
		for(std::list<aiNode*>::iterator nl_it = node_list.begin(); nl_it != node_list.end(); nl_it++)
		{
			// and try to find them in another top nodes.
			std::list<aiNode*>::const_iterator next_it = nl_it;

			next_it++;
			for(; next_it != node_list.end(); next_it++)
			{
				if((*next_it)->FindNode((*nl_it)->mName) != nullptr)
				{
					// if current top node(nl_it) found in another top node then erase it from node_list and restart search loop.
					node_list.erase(nl_it);

					goto nl_clean_loop;
				}
			}// for(; next_it != node_list.end(); next_it++)
		}// for(std::list<aiNode*>::const_iterator nl_it = node_list.begin(); nl_it != node_list.end(); nl_it++)
	}

	//
	// move created objects to aiScene
	//
	//
	// Nodes
	if(node_list.size() > 0)
	{
		std::list<aiNode*>::const_iterator nl_it = node_list.begin();

		pScene->mRootNode->mNumChildren = static_cast<unsigned int>(node_list.size());
		pScene->mRootNode->mChildren = new aiNode*[pScene->mRootNode->mNumChildren];
		for(size_t i = 0; i < pScene->mRootNode->mNumChildren; i++)
		{
			// Objects and constellation that must be showed placed at top of hierarchy in <amf> node. So all aiNode's in node_list must have
			// mRootNode only as parent.
			(*nl_it)->mParent = pScene->mRootNode;
			pScene->mRootNode->mChildren[i] = *nl_it++;
		}
	}// if(node_list.size() > 0)

	//
	// Meshes
	if(mesh_list.size() > 0)
	{
		std::list<aiMesh*>::const_iterator ml_it = mesh_list.begin();

		pScene->mNumMeshes = static_cast<unsigned int>(mesh_list.size());
		pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
		for(size_t i = 0; i < pScene->mNumMeshes; i++) pScene->mMeshes[i] = *ml_it++;
	}// if(mesh_list.size() > 0)

	//
	// Textures
	pScene->mNumTextures = static_cast<unsigned int>(mTexture_Converted.size());
	if(pScene->mNumTextures > 0)
	{
		size_t idx;

		idx = 0;
		pScene->mTextures = new aiTexture*[pScene->mNumTextures];
		for(const SPP_Texture& tex_convd: mTexture_Converted)
		{
			pScene->mTextures[idx] = new aiTexture;
			pScene->mTextures[idx]->mWidth = static_cast<unsigned int>(tex_convd.Width);
			pScene->mTextures[idx]->mHeight = static_cast<unsigned int>(tex_convd.Height);
			pScene->mTextures[idx]->pcData = (aiTexel*)tex_convd.Data;
			// texture format description.
			strcpy(pScene->mTextures[idx]->achFormatHint, tex_convd.FormatHint);
			idx++;
		}// for(const SPP_Texture& tex_convd: mTexture_Converted)

		// Create materials for embedded textures.
		idx = 0;
		pScene->mNumMaterials = static_cast<unsigned int>(mTexture_Converted.size());
		pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
		for(const SPP_Texture& tex_convd: mTexture_Converted)
		{
			const aiString texture_id(AI_EMBEDDED_TEXNAME_PREFIX + to_string(idx));
			const int mode = aiTextureOp_Multiply;
			const int repeat = tex_convd.Tiled ? 1 : 0;

			pScene->mMaterials[idx] = new aiMaterial;
			pScene->mMaterials[idx]->AddProperty(&texture_id, AI_MATKEY_TEXTURE_DIFFUSE(0));
			pScene->mMaterials[idx]->AddProperty(&mode, 1, AI_MATKEY_TEXOP_DIFFUSE(0));
			pScene->mMaterials[idx]->AddProperty(&repeat, 1, AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0));
			pScene->mMaterials[idx]->AddProperty(&repeat, 1, AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0));
			idx++;
		}
	}// if(pScene->mNumTextures > 0)
}// END: after that walk through children of root and collect data

}// namespace Assimp

#endif // !ASSIMP_BUILD_NO_AMF_IMPORTER
