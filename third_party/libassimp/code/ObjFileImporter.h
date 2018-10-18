/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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
#ifndef OBJ_FILE_IMPORTER_H_INC
#define OBJ_FILE_IMPORTER_H_INC

#include <assimp/BaseImporter.h>
#include <assimp/material.h>
#include <vector>

struct aiMesh;
struct aiNode;

namespace Assimp {

namespace ObjFile {
    struct Object;
    struct Model;
}

// ------------------------------------------------------------------------------------------------
/// \class  ObjFileImporter
/// \brief  Imports a waveform obj file
// ------------------------------------------------------------------------------------------------
class ObjFileImporter : public BaseImporter {
public:
    /// \brief  Default constructor
    ObjFileImporter();

    /// \brief  Destructor
    ~ObjFileImporter();

public:
    /// \brief  Returns whether the class can handle the format of the given file.
    /// \remark See BaseImporter::CanRead() for details.
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const;

private:
    //! \brief  Appends the supported extension.
    const aiImporterDesc* GetInfo () const;

    //! \brief  File import implementation.
    void InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);

    //! \brief  Create the data from imported content.
    void CreateDataFromImport(const ObjFile::Model* pModel, aiScene* pScene);

    //! \brief  Creates all nodes stored in imported content.
    aiNode *createNodes(const ObjFile::Model* pModel, const ObjFile::Object* pData,
        aiNode *pParent, aiScene* pScene, std::vector<aiMesh*> &MeshArray);

    //! \brief  Creates topology data like faces and meshes for the geometry.
    aiMesh *createTopology( const ObjFile::Model* pModel, const ObjFile::Object* pData,
        unsigned int uiMeshIndex );

    //! \brief  Creates vertices from model.
    void createVertexArray(const ObjFile::Model* pModel, const ObjFile::Object* pCurrentObject,
        unsigned int uiMeshIndex, aiMesh* pMesh, unsigned int numIndices );

    //! \brief  Object counter helper method.
    void countObjects(const std::vector<ObjFile::Object*> &rObjects, int &iNumMeshes);

    //! \brief  Material creation.
    void createMaterials(const ObjFile::Model* pModel, aiScene* pScene);

    /// @brief  Adds special property for the used texture mapping mode of the model.
    void addTextureMappingModeProperty(aiMaterial* mat, aiTextureType type, int clampMode = 1, int index = 0);

    //! \brief  Appends a child node to a parent node and updates the data structures.
    void appendChildToParentNode(aiNode *pParent, aiNode *pChild);

private:
    //! Data buffer
    std::vector<char> m_Buffer;
    //! Pointer to root object instance
    ObjFile::Object *m_pRootObject;
    //! Absolute pathname of model in file system
    std::string m_strAbsPath;
};

// ------------------------------------------------------------------------------------------------

} // Namespace Assimp

#endif
