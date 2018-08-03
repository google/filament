/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

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

#ifndef AI_OGREIMPORTER_H_INC
#define AI_OGREIMPORTER_H_INC

#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

#include "BaseImporter.h"

#include "OgreStructs.h"
#include "OgreParsingUtils.h"

#include <assimp/material.h>

namespace Assimp
{
namespace Ogre
{

/** Importer for Ogre mesh, skeleton and material formats.
    @todo Support vertex colors.
    @todo Support poses/animations from the mesh file.
    Currently only skeleton file animations are supported. */
class OgreImporter : public BaseImporter
{
public:
    /// BaseImporter override.
    virtual bool CanRead(const std::string &pFile, IOSystem *pIOHandler, bool checkSig) const;

    /// BaseImporter override.
    virtual void InternReadFile(const std::string &pFile, aiScene *pScene, IOSystem *pIOHandler);

    /// BaseImporter override.
    virtual const aiImporterDesc *GetInfo() const;

    /// BaseImporter override.
    virtual void SetupProperties(const Importer *pImp);

private:
    /// Read materials referenced by the @c mesh to @c pScene.
    void ReadMaterials(const std::string &pFile, Assimp::IOSystem *pIOHandler, aiScene *pScene, Mesh *mesh);
    void ReadMaterials(const std::string &pFile, Assimp::IOSystem *pIOHandler, aiScene *pScene, MeshXml *mesh);
    void AssignMaterials(aiScene *pScene, std::vector<aiMaterial*> &materials);

    /// Reads material
    aiMaterial* ReadMaterial(const std::string &pFile, Assimp::IOSystem *pIOHandler, const std::string &MaterialName);

    // These functions parse blocks from a material file from @c ss. Starting parsing from "{" and ending it to "}".
    bool ReadTechnique(const std::string &techniqueName, std::stringstream &ss, aiMaterial *material);
    bool ReadPass(const std::string &passName, std::stringstream &ss, aiMaterial *material);
    bool ReadTextureUnit(const std::string &textureUnitName, std::stringstream &ss, aiMaterial *material);

    std::string m_userDefinedMaterialLibFile;
    bool m_detectTextureTypeFromFilename;

    std::map<aiTextureType, unsigned int> m_textures;
};
} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
#endif // AI_OGREIMPORTER_H_INC
