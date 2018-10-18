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

/** @file  OFFLoader.cpp
 *  @brief Implementation of the OFF importer class
 */


#ifndef ASSIMP_BUILD_NO_OFF_IMPORTER

// internal headers
#include "OFFLoader.h"
#include <assimp/ParsingUtils.h>
#include <assimp/fast_atof.h>
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "OFF Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "off"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
OFFImporter::OFFImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
OFFImporter::~OFFImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool OFFImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    if (extension == "off")
        return true;
    else if (!extension.length() || checkSig)
    {
        if (!pIOHandler)return true;
        const char* tokens[] = {"off"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1,3);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* OFFImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void OFFImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open OFF file " + pFile + ".");
    }

    // allocate storage and copy the contents of the file to a memory buffer
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    char line[4096];
    GetNextLine(buffer,line);
    if ('O' == line[0]) {
        GetNextLine(buffer,line); // skip the 'OFF' line
    }

    const char* sz = line; SkipSpaces(&sz);
    const unsigned int numVertices = strtoul10(sz,&sz);SkipSpaces(&sz);
    const unsigned int numFaces = strtoul10(sz,&sz);

    if (!numVertices) {
        throw DeadlyImportError("OFF: There are no valid vertices");
    }
    if (!numFaces) {
        throw DeadlyImportError("OFF: There are no valid faces");
    }

    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[ pScene->mNumMeshes ];

    aiMesh* mesh = new aiMesh();
    pScene->mMeshes[0] = mesh;

    mesh->mNumFaces = numFaces;
    aiFace* faces = new aiFace [mesh->mNumFaces];
    mesh->mFaces = faces;

    std::vector<aiVector3D> tempPositions(numVertices);

    // now read all vertex lines
    for (unsigned int i = 0; i< numVertices;++i)
    {
        if(!GetNextLine(buffer,line))
        {
            ASSIMP_LOG_ERROR("OFF: The number of verts in the header is incorrect");
            break;
        }
        aiVector3D& v = tempPositions[i];

        sz = line; SkipSpaces(&sz);
        sz = fast_atoreal_move<ai_real>(sz,(ai_real&)v.x); SkipSpaces(&sz);
        sz = fast_atoreal_move<ai_real>(sz,(ai_real&)v.y); SkipSpaces(&sz);
        fast_atoreal_move<ai_real>(sz,(ai_real&)v.z);
    }


    // First find out how many vertices we'll need
    const char* old = buffer;
    for (unsigned int i = 0; i< mesh->mNumFaces;++i)
    {
        if(!GetNextLine(buffer,line))
        {
            ASSIMP_LOG_ERROR("OFF: The number of faces in the header is incorrect");
            break;
        }
        sz = line;SkipSpaces(&sz);
        faces->mNumIndices = strtoul10(sz,&sz);
        if(!(faces->mNumIndices) || faces->mNumIndices > 9)
        {
            ASSIMP_LOG_ERROR("OFF: Faces with zero indices aren't allowed");
            --mesh->mNumFaces;
            continue;
        }
        mesh->mNumVertices += faces->mNumIndices;
        ++faces;
    }

    if (!mesh->mNumVertices)
        throw DeadlyImportError("OFF: There are no valid faces");

    // allocate storage for the output vertices
    std::vector<aiVector3D> verts;
    verts.reserve(mesh->mNumVertices);

    // second: now parse all face indices
    buffer = old;
    faces = mesh->mFaces;
    for (unsigned int i = 0, p = 0; i< mesh->mNumFaces;)
    {
        if(!GetNextLine(buffer,line))break;

        unsigned int idx;
        sz = line;SkipSpaces(&sz);
        idx = strtoul10(sz,&sz);
        if(!(idx) || idx > 9)
            continue;

        faces->mIndices = new unsigned int [faces->mNumIndices];
        for (unsigned int m = 0; m < faces->mNumIndices;++m)
        {
            SkipSpaces(&sz);
            idx = strtoul10(sz,&sz);
            if ((idx) >= numVertices)
            {
                ASSIMP_LOG_ERROR("OFF: Vertex index is out of range");
                idx = numVertices-1;
            }
            faces->mIndices[m] = p++;
            verts.push_back(tempPositions[idx]);
        }
        ++i;
        ++faces;
    }

    if (mesh->mNumVertices != verts.size()) {
        throw DeadlyImportError("OFF: Vertex count mismatch");
    }
    mesh->mVertices = new aiVector3D[verts.size()];
    memcpy(mesh->mVertices, &verts[0], verts.size() * sizeof(aiVector3D));
    // generate the output node graph
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mName.Set("<OFFRoot>");
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int [pScene->mRootNode->mNumMeshes];
    pScene->mRootNode->mMeshes[0] = 0;

    // generate a default material
    pScene->mNumMaterials = 1;
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    aiMaterial* pcMat = new aiMaterial();

    aiColor4D clr( ai_real( 0.6 ), ai_real( 0.6 ), ai_real( 0.6 ), ai_real( 1.0 ) );
    pcMat->AddProperty(&clr,1,AI_MATKEY_COLOR_DIFFUSE);
    pScene->mMaterials[0] = pcMat;

    const int twosided =1;
    pcMat->AddProperty(&twosided,1,AI_MATKEY_TWOSIDED);
}

#endif // !! ASSIMP_BUILD_NO_OFF_IMPORTER
