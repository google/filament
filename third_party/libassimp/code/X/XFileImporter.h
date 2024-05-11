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

/** @file  XFileImporter.h
 *  @brief Definition of the XFile importer class.
 */
#ifndef AI_XFILEIMPORTER_H_INC
#define AI_XFILEIMPORTER_H_INC

#include <map>

#include "XFileHelper.h"
#include <assimp/BaseImporter.h>

#include <assimp/types.h>

struct aiNode;

namespace Assimp    {

namespace XFile {
    struct Scene;
    struct Node;
}

// ---------------------------------------------------------------------------
/** The XFileImporter is a worker class capable of importing a scene from a
 *   DirectX file .x
 */
class XFileImporter : public BaseImporter {
public:
    XFileImporter();
    ~XFileImporter();

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details. */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool CheckSig) const;

protected:

    // -------------------------------------------------------------------
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details
     */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

    // -------------------------------------------------------------------
    /** Constructs the return data structure out of the imported data.
     * @param pScene The scene to construct the return data in.
     * @param pData The imported data in the internal temporary
     *   representation.
     */
    void CreateDataRepresentationFromImport( aiScene* pScene, XFile::Scene* pData);

    // -------------------------------------------------------------------
    /** Recursively creates scene nodes from the imported hierarchy.
     * The meshes and materials of the nodes will be extracted on the way.
     * @param pScene The scene to construct the return data in.
     * @param pParent The parent node where to create new child nodes
     * @param pNode The temporary node to copy.
     * @return The created node
     */
    aiNode* CreateNodes( aiScene* pScene, aiNode* pParent,
        const XFile::Node* pNode);

    // -------------------------------------------------------------------
    /** Converts all meshes in the given mesh array. Each mesh is split
     * up per material, the indices of the generated meshes are stored in
     * the node structure.
     * @param pScene The scene to construct the return data in.
     * @param pNode The target node structure that references the
     *   constructed meshes.
     * @param pMeshes The array of meshes to convert
     */
    void CreateMeshes( aiScene* pScene, aiNode* pNode,
        const std::vector<XFile::Mesh*>& pMeshes);

    // -------------------------------------------------------------------
    /** Converts the animations from the given imported data and creates
    *  them in the scene.
     * @param pScene The scene to hold to converted animations
     * @param pData The data to read the animations from
     */
    void CreateAnimations( aiScene* pScene, const XFile::Scene* pData);

    // -------------------------------------------------------------------
    /** Converts all materials in the given array and stores them in the
     *  scene's material list.
     * @param pScene The scene to hold the converted materials.
     * @param pMaterials The material array to convert.
     */
    void ConvertMaterials( aiScene* pScene, std::vector<XFile::Material>& pMaterials);

protected:
    /** Buffer to hold the loaded file */
    std::vector<char> mBuffer;
};

} // end of namespace Assimp

#endif // AI_BASEIMPORTER_H_INC
