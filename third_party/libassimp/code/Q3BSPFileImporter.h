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
#ifndef ASSIMP_Q3BSPFILEIMPORTER_H_INC
#define ASSIMP_Q3BSPFILEIMPORTER_H_INC

#include <assimp/BaseImporter.h>

#include <map>
#include <string>

struct aiMesh;
struct aiNode;
struct aiFace;
struct aiMaterial;
struct aiTexture;

namespace Assimp {

namespace Q3BSP {
    class Q3BSPZipArchive;
    struct Q3BSPModel;
    struct sQ3BSPFace;
}

// ------------------------------------------------------------------------------------------------
/** Loader to import BSP-levels from a PK3 archive or from a unpacked BSP-level.
 */
// ------------------------------------------------------------------------------------------------
class Q3BSPFileImporter : public BaseImporter {
public:
    /// @brief  Default constructor.
    Q3BSPFileImporter();

    /// @brief  Destructor.
    ~Q3BSPFileImporter();

    /// @brief  Returns whether the class can handle the format of the given file.
    /// @remark See BaseImporter::CanRead() for details.
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig ) const;

protected:
    typedef std::map<std::string, std::vector<Q3BSP::sQ3BSPFace*>*> FaceMap;
    typedef std::map<std::string, std::vector<Q3BSP::sQ3BSPFace*>* >::iterator FaceMapIt;
    typedef std::map<std::string, std::vector<Q3BSP::sQ3BSPFace*>*>::const_iterator FaceMapConstIt;

    const aiImporterDesc* GetInfo () const;
    void InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);
    void separateMapName( const std::string &rImportName, std::string &rArchiveName, std::string &rMapName );
    bool findFirstMapInArchive( Q3BSP::Q3BSPZipArchive &rArchive, std::string &rMapName );
    void CreateDataFromImport( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene, Q3BSP::Q3BSPZipArchive *pArchive );
    void CreateNodes( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene, aiNode *pParent );
    aiNode *CreateTopology( const Q3BSP::Q3BSPModel *pModel, unsigned int materialIdx,
        std::vector<Q3BSP::sQ3BSPFace*> &rArray, aiMesh  **pMesh );
    void createTriangleTopology( const Q3BSP::Q3BSPModel *pModel, Q3BSP::sQ3BSPFace *pQ3BSPFace, aiMesh* pMesh, unsigned int &rFaceIdx,
        unsigned int &rVertIdx  );
    void createMaterials( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene, Q3BSP::Q3BSPZipArchive *pArchive );
    size_t countData( const std::vector<Q3BSP::sQ3BSPFace*> &rArray ) const;
    size_t countFaces( const std::vector<Q3BSP::sQ3BSPFace*> &rArray ) const;
    size_t countTriangles( const std::vector<Q3BSP::sQ3BSPFace*> &rArray ) const;
    void createMaterialMap( const Q3BSP::Q3BSPModel *pModel);
    aiFace *getNextFace( aiMesh *pMesh, unsigned int &rFaceIdx );
    bool importTextureFromArchive( const Q3BSP::Q3BSPModel *pModel, Q3BSP::Q3BSPZipArchive *pArchive, aiScene* pScene,
        aiMaterial *pMatHelper, int textureId );
    bool importLightmap( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene, aiMaterial *pMatHelper, int lightmapId );
    bool importEntities( const Q3BSP::Q3BSPModel *pModel, aiScene* pScene );
    bool expandFile(  Q3BSP::Q3BSPZipArchive *pArchive, const std::string &rFilename, const std::vector<std::string> &rExtList,
        std::string &rFile, std::string &rExt );

private:
    aiMesh *m_pCurrentMesh;
    aiFace *m_pCurrentFace;
    FaceMap m_MaterialLookupMap;
    std::vector<aiTexture*> mTextures;
};

// ------------------------------------------------------------------------------------------------

} // Namespace Assimp

#endif // ASSIMP_Q3BSPFILEIMPORTER_H_INC
