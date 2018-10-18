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

#ifndef ASSIMP_BUILD_NO_OGRE_IMPORTER

#include "OgreImporter.h"
#include "OgreBinarySerializer.h"
#include "OgreXmlSerializer.h"
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <memory>

static const aiImporterDesc desc = {
    "Ogre3D Mesh Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportTextFlavour | aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "mesh mesh.xml"
};

namespace Assimp
{
namespace Ogre
{

const aiImporterDesc* OgreImporter::GetInfo() const
{
    return &desc;
}

void OgreImporter::SetupProperties(const Importer* pImp)
{
    m_userDefinedMaterialLibFile = pImp->GetPropertyString(AI_CONFIG_IMPORT_OGRE_MATERIAL_FILE, "Scene.material");
    m_detectTextureTypeFromFilename = pImp->GetPropertyBool(AI_CONFIG_IMPORT_OGRE_TEXTURETYPE_FROM_FILENAME, false);
}

bool OgreImporter::CanRead(const std::string &pFile, Assimp::IOSystem *pIOHandler, bool checkSig) const
{
    if (!checkSig) {
        return EndsWith(pFile, ".mesh.xml", false) || EndsWith(pFile, ".mesh", false);
    }

    if (EndsWith(pFile, ".mesh.xml", false))
    {
        const char* tokens[] = { "<mesh>" };
        return SearchFileHeaderForToken(pIOHandler, pFile, tokens, 1);
    }
    else
    {
        /// @todo Read and validate first header chunk?
        return EndsWith(pFile, ".mesh", false);
    }
}

void OgreImporter::InternReadFile(const std::string &pFile, aiScene *pScene, Assimp::IOSystem *pIOHandler)
{
    // Open source file
    IOStream *f = pIOHandler->Open(pFile, "rb");
    if (!f) {
        throw DeadlyImportError("Failed to open file " + pFile);
    }

    // Binary .mesh import
    if (EndsWith(pFile, ".mesh", false))
    {
        /// @note MemoryStreamReader takes ownership of f.
        MemoryStreamReader reader(f);

        // Import mesh
        std::unique_ptr<Mesh> mesh(OgreBinarySerializer::ImportMesh(&reader));

        // Import skeleton
        OgreBinarySerializer::ImportSkeleton(pIOHandler, mesh.get());

        // Import mesh referenced materials
        ReadMaterials(pFile, pIOHandler, pScene, mesh.get());

        // Convert to Assimp
        mesh->ConvertToAssimpScene(pScene);
    }
    // XML .mesh.xml import
    else
    {
        /// @note XmlReader does not take ownership of f, hence the scoped ptr.
        std::unique_ptr<IOStream> scopedFile(f);
        std::unique_ptr<CIrrXML_IOStreamReader> xmlStream(new CIrrXML_IOStreamReader(scopedFile.get()));
        std::unique_ptr<XmlReader> reader(irr::io::createIrrXMLReader(xmlStream.get()));

        // Import mesh
        std::unique_ptr<MeshXml> mesh(OgreXmlSerializer::ImportMesh(reader.get()));

        // Import skeleton
        OgreXmlSerializer::ImportSkeleton(pIOHandler, mesh.get());

        // Import mesh referenced materials
        ReadMaterials(pFile, pIOHandler, pScene, mesh.get());

        // Convert to Assimp
        mesh->ConvertToAssimpScene(pScene);
    }
}

} // Ogre
} // Assimp

#endif // ASSIMP_BUILD_NO_OGRE_IMPORTER
