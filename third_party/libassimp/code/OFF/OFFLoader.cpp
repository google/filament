/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team



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


// skip blank space, lines and comments
static void NextToken(const char **car, const char* end) {
  SkipSpacesAndLineEnd(car);
  while (*car < end && (**car == '#' || **car == '\n' || **car == '\r')) {
    SkipLine(car);
    SkipSpacesAndLineEnd(car);
  }
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void OFFImporter::InternReadFile( const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler) {
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == nullptr) {
        throw DeadlyImportError( "Failed to open OFF file " + pFile + ".");
    }

    // allocate storage and copy the contents of the file to a memory buffer
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    // Proper OFF header parser. We only implement normal loading for now.
    bool hasTexCoord = false, hasNormals = false, hasColors = false;
    bool hasHomogenous = false, hasDimension = false;
    unsigned int dimensions = 3;
    const char* car = buffer;
    const char* end = buffer + mBuffer2.size();
    NextToken(&car, end);
    
    if (car < end - 2 && car[0] == 'S' && car[1] == 'T') {
      hasTexCoord = true; car += 2;
    }
    if (car < end - 1 && car[0] == 'C') {
      hasColors = true; car++;
    }
    if (car < end- 1 && car[0] == 'N') {
      hasNormals = true; car++;
    }
    if (car < end - 1 && car[0] == '4') {
      hasHomogenous = true; car++;
    }
    if (car < end - 1 && car[0] == 'n') {
      hasDimension = true; car++;
    }
    if (car < end - 3 && car[0] == 'O' && car[1] == 'F' && car[2] == 'F') {
        car += 3;
	NextToken(&car, end);
    } else {
      // in case there is no OFF header (which is allowed by the
      // specification...), then we might have unintentionally read an
      // additional dimension from the primitive count fields
      dimensions = 3;
      hasHomogenous = false;
      NextToken(&car, end);
      
      // at this point the next token should be an integer number
      if (car >= end - 1 || *car < '0' || *car > '9') {
	throw DeadlyImportError("OFF: Header is invalid");
      }
    }
    if (hasDimension) {
        dimensions = strtoul10(car, &car);
	NextToken(&car, end);
    }
    if (dimensions > 3) {
        throw DeadlyImportError
	  ("OFF: Number of vertex coordinates higher than 3 unsupported");
    }

    NextToken(&car, end);
    const unsigned int numVertices = strtoul10(car, &car);
    NextToken(&car, end);
    const unsigned int numFaces = strtoul10(car, &car);
    NextToken(&car, end);
    strtoul10(car, &car);  // skip edge count
    NextToken(&car, end);

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
    aiFace* faces = new aiFace[mesh->mNumFaces];
    mesh->mFaces = faces;

    mesh->mNumVertices = numVertices;
    mesh->mVertices = new aiVector3D[numVertices];
    mesh->mNormals = hasNormals ? new aiVector3D[numVertices] : nullptr;
    mesh->mColors[0] = hasColors ? new aiColor4D[numVertices] : nullptr;

    if (hasTexCoord) {
        mesh->mNumUVComponents[0] = 2;
        mesh->mTextureCoords[0] = new aiVector3D[numVertices];
    }
    char line[4096];
    buffer = car;
    const char *sz = car;

    // now read all vertex lines
    for (unsigned int i = 0; i < numVertices; ++i) {
        if(!GetNextLine(buffer, line)) {
            ASSIMP_LOG_ERROR("OFF: The number of verts in the header is incorrect");
            break;
        }
        aiVector3D& v = mesh->mVertices[i];	
        sz = line;

	// helper array to write a for loop over possible dimension values
	ai_real* vec[3] = {&v.x, &v.y, &v.z};

	// stop at dimensions: this allows loading 1D or 2D coordinate vertices
        for (unsigned int dim = 0; dim < dimensions; ++dim ) {
	    SkipSpaces(&sz);
	    sz = fast_atoreal_move<ai_real>(sz, *vec[dim]);
	}

	// if has homogenous coordinate, divide others by this one
	if (hasHomogenous) {
	    SkipSpaces(&sz);
	    ai_real w = 1.;
	    sz = fast_atoreal_move<ai_real>(sz, w);
            for (unsigned int dim = 0; dim < dimensions; ++dim ) {
	        *(vec[dim]) /= w;
	    }
	}

	// read optional normals
	if (hasNormals) {
	    aiVector3D& n = mesh->mNormals[i];
	    SkipSpaces(&sz);
	    sz = fast_atoreal_move<ai_real>(sz,(ai_real&)n.x);
	    SkipSpaces(&sz);
	    sz = fast_atoreal_move<ai_real>(sz,(ai_real&)n.y);
	    SkipSpaces(&sz);
	    fast_atoreal_move<ai_real>(sz,(ai_real&)n.z);
	}
	
	// reading colors is a pain because the specification says it can be
	// integers or floats, and any number of them between 1 and 4 included,
	// until the next comment or end of line
	// in theory should be testing type !
	if (hasColors) {
	    aiColor4D& c = mesh->mColors[0][i];
	    SkipSpaces(&sz);
	    sz = fast_atoreal_move<ai_real>(sz,(ai_real&)c.r);
            if (*sz != '#' && *sz != '\n' && *sz != '\r') {
	        SkipSpaces(&sz);
	        sz = fast_atoreal_move<ai_real>(sz,(ai_real&)c.g);
            } else {
	        c.g = 0.;
	    }
            if (*sz != '#' && *sz != '\n' && *sz != '\r') {
	        SkipSpaces(&sz);
	        sz = fast_atoreal_move<ai_real>(sz,(ai_real&)c.b);
            } else {
	        c.b = 0.;
	    }
            if (*sz != '#' && *sz != '\n' && *sz != '\r') {
	        SkipSpaces(&sz);
	        sz = fast_atoreal_move<ai_real>(sz,(ai_real&)c.a);
            } else {
	        c.a = 1.;
	    }
	}
        if (hasTexCoord) {
	    aiVector3D& t = mesh->mTextureCoords[0][i];
	    SkipSpaces(&sz);
	    sz = fast_atoreal_move<ai_real>(sz,(ai_real&)t.x);
	    SkipSpaces(&sz);
	    fast_atoreal_move<ai_real>(sz,(ai_real&)t.y);
	}
    }

    // load faces with their indices
    faces = mesh->mFaces;
    for (unsigned int i = 0; i < numFaces; ) {
        if(!GetNextLine(buffer,line)) {
            ASSIMP_LOG_ERROR("OFF: The number of faces in the header is incorrect");
            break;
        }
        unsigned int idx;
        sz = line; SkipSpaces(&sz);
        idx = strtoul10(sz,&sz);
        if(!idx || idx > 9) {
	    ASSIMP_LOG_ERROR("OFF: Faces with zero indices aren't allowed");
            --mesh->mNumFaces;
            continue;
	}
	faces->mNumIndices = idx;
        faces->mIndices = new unsigned int[faces->mNumIndices];
        for (unsigned int m = 0; m < faces->mNumIndices;++m) {
            SkipSpaces(&sz);
            idx = strtoul10(sz,&sz);
            if (idx >= numVertices) {
                ASSIMP_LOG_ERROR("OFF: Vertex index is out of range");
                idx = numVertices - 1;
            }
            faces->mIndices[m] = idx;
        }
        ++i;
        ++faces;
    }
    
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

    const int twosided = 1;
    pcMat->AddProperty(&twosided, 1, AI_MATKEY_TWOSIDED);
}

#endif // !! ASSIMP_BUILD_NO_OFF_IMPORTER
