/*
Assimp2Json
Copyright (c) 2011, Alexander C. Gessler

Licensed under a 3-clause BSD license. See the LICENSE file for more information.

*/

#include "mesh_splitter.h"

#include <assimp/scene.h>

// ----------------------------------------------------------------------------
// Note: this is largely based on assimp's SplitLargeMeshes_Vertex process.
// it is refactored and the coding style is slightly improved, though.
// ----------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// Executes the post processing step on the given imported data.
void MeshSplitter::Execute( aiScene* pScene) {
	std::vector<std::pair<aiMesh*, unsigned int> > source_mesh_map;

	for( unsigned int a = 0; a < pScene->mNumMeshes; a++) {
		SplitMesh(a, pScene->mMeshes[a],source_mesh_map);
	}

	const unsigned int size = static_cast<unsigned int>(source_mesh_map.size());
	if (size != pScene->mNumMeshes) {
		// it seems something has been split. rebuild the mesh list
		delete[] pScene->mMeshes;
		pScene->mNumMeshes = size;
		pScene->mMeshes = new aiMesh*[size]();

		for (unsigned int i = 0; i < size;++i) {
			pScene->mMeshes[i] = source_mesh_map[i].first;
		}

		// now we need to update all nodes
		UpdateNode(pScene->mRootNode,source_mesh_map);
	}
}


// ------------------------------------------------------------------------------------------------
void MeshSplitter::UpdateNode(aiNode* pcNode, const std::vector<std::pair<aiMesh*, unsigned int> >& source_mesh_map) {
	// TODO: should better use std::(multi)set for source_mesh_map.

	// for every index in out list build a new entry
	std::vector<unsigned int> aiEntries;
	aiEntries.reserve(pcNode->mNumMeshes + 1);
	for (unsigned int i = 0; i < pcNode->mNumMeshes;++i)	{
		for (unsigned int a = 0, end = static_cast<unsigned int>(source_mesh_map.size()); a < end;++a)	{
			if (source_mesh_map[a].second == pcNode->mMeshes[i])	{
				aiEntries.push_back(a);
			}
		}
	}

	// now build the new list
	delete pcNode->mMeshes;
	pcNode->mNumMeshes = static_cast<unsigned int>(aiEntries.size());
	pcNode->mMeshes = new unsigned int[pcNode->mNumMeshes];

	for (unsigned int b = 0; b < pcNode->mNumMeshes;++b) {
		pcNode->mMeshes[b] = aiEntries[b];
	}

	// recursively update children
	for (unsigned int i = 0, end = pcNode->mNumChildren; i < end;++i)	{
		UpdateNode ( pcNode->mChildren[i], source_mesh_map );
	}
	return;
}

#define WAS_NOT_COPIED 0xffffffff

typedef std::pair <unsigned int,float> PerVertexWeight;
typedef std::vector	<PerVertexWeight> VertexWeightTable;

// ------------------------------------------------------------------------------------------------
VertexWeightTable* ComputeVertexBoneWeightTable(const aiMesh* pMesh) {
	if (!pMesh || !pMesh->mNumVertices || !pMesh->mNumBones) {
		return nullptr;
	}

	VertexWeightTable* const avPerVertexWeights = new VertexWeightTable[pMesh->mNumVertices];
	for (unsigned int i = 0; i < pMesh->mNumBones;++i)	{

		aiBone* bone = pMesh->mBones[i];
		for (unsigned int a = 0; a < bone->mNumWeights;++a)	{
			const aiVertexWeight& weight = bone->mWeights[a];
			avPerVertexWeights[weight.mVertexId].push_back( std::make_pair(i,weight.mWeight) );
		}
	}
	return avPerVertexWeights;
}

// ------------------------------------------------------------------------------------------------
void MeshSplitter :: SplitMesh(unsigned int a, aiMesh* in_mesh, std::vector<std::pair<aiMesh*, unsigned int> >& source_mesh_map) {
	// TODO: should better use std::(multi)set for source_mesh_map.

	if (in_mesh->mNumVertices <= LIMIT)	{
		source_mesh_map.push_back(std::make_pair(in_mesh,a));
		return;
	}

	// build a per-vertex weight list if necessary
	VertexWeightTable* avPerVertexWeights = ComputeVertexBoneWeightTable(in_mesh);

	// we need to split this mesh into sub meshes. Estimate submesh size
	const unsigned int sub_meshes = (in_mesh->mNumVertices / LIMIT) + 1;

	// create a std::vector<unsigned int> to remember which vertices have already 
	// been copied and to which position (i.e. output index)
	std::vector<unsigned int> was_copied_to;
	was_copied_to.resize(in_mesh->mNumVertices,WAS_NOT_COPIED);

	// Try to find a good estimate for the number of output faces
	// per mesh. Add 12.5% as buffer
	unsigned int size_estimated = in_mesh->mNumFaces / sub_meshes;
	size_estimated += size_estimated / 8;

	// now generate all submeshes
	unsigned int base = 0;
	while (true) {
		const unsigned int out_vertex_index = LIMIT;

		aiMesh* out_mesh = new aiMesh();			
		out_mesh->mNumVertices = 0;
		out_mesh->mMaterialIndex = in_mesh->mMaterialIndex;

		// the name carries the adjacency information between the meshes
		out_mesh->mName = in_mesh->mName;

		typedef std::vector<aiVertexWeight> BoneWeightList;
		if (in_mesh->HasBones())	{
			out_mesh->mBones = new aiBone*[in_mesh->mNumBones]();
		}

		// clear the temporary helper array
		if (base)	{
			std::fill(was_copied_to.begin(), was_copied_to.end(), WAS_NOT_COPIED);
		}

		std::vector<aiFace> vFaces;

		// reserve enough storage for most cases
		if (in_mesh->HasPositions()) {
			out_mesh->mVertices = new aiVector3D[out_vertex_index];
		}

		if (in_mesh->HasNormals()) {
			out_mesh->mNormals = new aiVector3D[out_vertex_index];
		}

		if (in_mesh->HasTangentsAndBitangents())	{
			out_mesh->mTangents = new aiVector3D[out_vertex_index];
			out_mesh->mBitangents = new aiVector3D[out_vertex_index];
		}

		for (unsigned int c = 0; in_mesh->HasVertexColors(c);++c)	{
			out_mesh->mColors[c] = new aiColor4D[out_vertex_index];
		}

		for (unsigned int c = 0; in_mesh->HasTextureCoords(c);++c)	{
			out_mesh->mNumUVComponents[c] = in_mesh->mNumUVComponents[c];
			out_mesh->mTextureCoords[c] = new aiVector3D[out_vertex_index];
		}
		vFaces.reserve(size_estimated);

		// (we will also need to copy the array of indices)
		while (base < in_mesh->mNumFaces) {
			const unsigned int iNumIndices = in_mesh->mFaces[base].mNumIndices;

			// doesn't catch degenerates but is quite fast
			unsigned int iNeed = 0;
			for (unsigned int v = 0; v < iNumIndices;++v)	{
				unsigned int index = in_mesh->mFaces[base].mIndices[v];

				// check whether we do already have this vertex
				if (WAS_NOT_COPIED == was_copied_to[index])	{
					iNeed++; 
				}
			}
			if (out_mesh->mNumVertices + iNeed > out_vertex_index)	{
				// don't use this face
				break;
			}

			vFaces.push_back(aiFace());
			aiFace& rFace = vFaces.back();

			// setup face type and number of indices
			rFace.mNumIndices = iNumIndices;
			rFace.mIndices = new unsigned int[iNumIndices];

			// need to update the output primitive types
			switch (rFace.mNumIndices)
			{
			case 1:
				out_mesh->mPrimitiveTypes |= aiPrimitiveType_POINT;
				break;
			case 2:
				out_mesh->mPrimitiveTypes |= aiPrimitiveType_LINE;
				break;
			case 3:
				out_mesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
				break;
			default:
				out_mesh->mPrimitiveTypes |= aiPrimitiveType_POLYGON;
			}

			// and copy the contents of the old array, offset them by current base
			for (unsigned int v = 0; v < iNumIndices;++v) {
				const unsigned int index = in_mesh->mFaces[base].mIndices[v];

				// check whether we do already have this vertex
				if (WAS_NOT_COPIED != was_copied_to[index]) {
					rFace.mIndices[v] = was_copied_to[index];
					continue;
				}

				// copy positions
				out_mesh->mVertices[out_mesh->mNumVertices] = (in_mesh->mVertices[index]);

				// copy normals
				if (in_mesh->HasNormals()) {
					out_mesh->mNormals[out_mesh->mNumVertices] = (in_mesh->mNormals[index]);
				}

				// copy tangents/bi-tangents
				if (in_mesh->HasTangentsAndBitangents()) {
					out_mesh->mTangents[out_mesh->mNumVertices] = (in_mesh->mTangents[index]);
					out_mesh->mBitangents[out_mesh->mNumVertices] = (in_mesh->mBitangents[index]);
				}

				// texture coordinates
				for (unsigned int c = 0;  c < AI_MAX_NUMBER_OF_TEXTURECOORDS;++c) {
					if (in_mesh->HasTextureCoords( c)) {
						out_mesh->mTextureCoords[c][out_mesh->mNumVertices] = in_mesh->mTextureCoords[c][index];
					}
				}
				// vertex colors 
				for (unsigned int c = 0;  c < AI_MAX_NUMBER_OF_COLOR_SETS;++c) {
					if (in_mesh->HasVertexColors( c)) {
						out_mesh->mColors[c][out_mesh->mNumVertices] = in_mesh->mColors[c][index];
					}
				}
				// check whether we have bone weights assigned to this vertex
				rFace.mIndices[v] = out_mesh->mNumVertices;
				if (avPerVertexWeights) {
					VertexWeightTable& table = avPerVertexWeights[ out_mesh->mNumVertices ];
					for (VertexWeightTable::const_iterator iter = table.begin(), end = table.end(); iter != end;++iter) {
						// allocate the bone weight array if necessary and store it in the mBones field (HACK!)
						BoneWeightList* weight_list = reinterpret_cast<BoneWeightList*>(out_mesh->mBones[(*iter).first]);
						if (!weight_list) {
							weight_list = new BoneWeightList();
							out_mesh->mBones[(*iter).first] = reinterpret_cast<aiBone*>(weight_list);
						}
						weight_list->push_back(aiVertexWeight(out_mesh->mNumVertices,(*iter).second));
					}
				}

				was_copied_to[index] = out_mesh->mNumVertices;
				out_mesh->mNumVertices++;
			}
			base++;
			if(out_mesh->mNumVertices == out_vertex_index) {
				// break here. The face is only added if it was complete
				break;
			}
		}

		// check which bones we'll need to create for this submesh
		if (in_mesh->HasBones()) {
			aiBone** ppCurrent = out_mesh->mBones;
			for (unsigned int k = 0; k < in_mesh->mNumBones;++k) {
				// check whether the bone exists
				BoneWeightList* const weight_list = reinterpret_cast<BoneWeightList*>(out_mesh->mBones[k]);

				if (weight_list) {
					const aiBone* const bone_in = in_mesh->mBones[k];
					aiBone* const bone_out = new aiBone();
					*ppCurrent++ = bone_out;
					bone_out->mName = aiString(bone_in->mName);
					bone_out->mOffsetMatrix =bone_in->mOffsetMatrix;
					bone_out->mNumWeights = (unsigned int)weight_list->size();
					bone_out->mWeights = new aiVertexWeight[bone_out->mNumWeights];

					// copy the vertex weights
					::memcpy(bone_out->mWeights, &(*weight_list)[0],bone_out->mNumWeights * sizeof(aiVertexWeight));

					delete weight_list;
					out_mesh->mNumBones++;
				}
			}
		}

		// copy the face list to the mesh
		out_mesh->mFaces = new aiFace[vFaces.size()];
		out_mesh->mNumFaces = (unsigned int)vFaces.size();

		for (unsigned int p = 0; p < out_mesh->mNumFaces;++p) {
			out_mesh->mFaces[p] = vFaces[p];
		}

		// add the newly created mesh to the list
		source_mesh_map.push_back(std::make_pair(out_mesh,a));

		if (base == in_mesh->mNumFaces) {
			break;
		}
	}

	// delete the per-vertex weight list again
	delete[] avPerVertexWeights;

	// now delete the old mesh data
	delete in_mesh;
}
