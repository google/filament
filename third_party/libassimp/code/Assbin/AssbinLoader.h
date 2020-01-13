
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

/** @file  AssbinLoader.h
 *  @brief .assbin File format loader
 */
#ifndef AI_ASSBINIMPORTER_H_INC
#define AI_ASSBINIMPORTER_H_INC

#include <assimp/BaseImporter.h>

struct aiMesh;
struct aiNode;
struct aiBone;
struct aiMaterial;
struct aiMaterialProperty;
struct aiNodeAnim;
struct aiAnimation;
struct aiTexture;
struct aiLight;
struct aiCamera;

#ifndef ASSIMP_BUILD_NO_ASSBIN_IMPORTER

namespace Assimp    {

// ---------------------------------------------------------------------------------
/** Importer class for 3D Studio r3 and r4 3DS files
 */
class AssbinImporter : public BaseImporter
{
private:
    bool shortened;
    bool compressed;

public:
    virtual bool CanRead(
        const std::string& pFile,
        IOSystem* pIOHandler,
        bool checkSig
    ) const;
    virtual const aiImporterDesc* GetInfo() const;
    virtual void InternReadFile(
    const std::string& pFile,
        aiScene* pScene,
        IOSystem* pIOHandler
    );
    void ReadHeader();
    void ReadBinaryScene( IOStream * stream, aiScene* pScene );
    void ReadBinaryNode( IOStream * stream, aiNode** mRootNode, aiNode* parent );
    void ReadBinaryMesh( IOStream * stream, aiMesh* mesh );
    void ReadBinaryBone( IOStream * stream, aiBone* bone );
    void ReadBinaryMaterial(IOStream * stream, aiMaterial* mat);
    void ReadBinaryMaterialProperty(IOStream * stream, aiMaterialProperty* prop);
    void ReadBinaryNodeAnim(IOStream * stream, aiNodeAnim* nd);
    void ReadBinaryAnim( IOStream * stream, aiAnimation* anim );
    void ReadBinaryTexture(IOStream * stream, aiTexture* tex);
    void ReadBinaryLight( IOStream * stream, aiLight* l );
    void ReadBinaryCamera( IOStream * stream, aiCamera* cam );
};

} // end of namespace Assimp

#endif // !! ASSIMP_BUILD_NO_ASSBIN_IMPORTER

#endif // AI_ASSBINIMPORTER_H_INC
