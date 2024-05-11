/*
Assimp2Json
Copyright (c) 2011, Alexander C. Gessler

Licensed under a 3-clause BSD license. See the LICENSE file for more information.

*/

#ifndef INCLUDED_MESH_SPLITTER
#define INCLUDED_MESH_SPLITTER

// ----------------------------------------------------------------------------
// Note: this is largely based on assimp's SplitLargeMeshes_Vertex process.
// it is refactored and the coding style is slightly improved, though.
// ----------------------------------------------------------------------------

#include <vector>

struct aiScene;
struct aiMesh;
struct aiNode;

// ---------------------------------------------------------------------------
/** Splits meshes of unique vertices into meshes with no more vertices than
 *  a given, configurable threshold value. 
 */
class MeshSplitter 
{

public:
	
	void SetLimit(unsigned int l) {
		LIMIT = l;
	}

	unsigned int GetLimit() const {
		return LIMIT;
	}

public:

	// -------------------------------------------------------------------
	/** Executes the post processing step on the given imported data.
	 * At the moment a process is not supposed to fail.
	 * @param pScene The imported data to work at.
	 */
	void Execute( aiScene* pScene);


private:

	void UpdateNode(aiNode* pcNode, const std::vector<std::pair<aiMesh*, unsigned int> >& source_mesh_map);
	void SplitMesh (unsigned int index, aiMesh* mesh, std::vector<std::pair<aiMesh*, unsigned int> >& source_mesh_map);

public:

	unsigned int LIMIT;
};

#endif // INCLUDED_MESH_SPLITTER

