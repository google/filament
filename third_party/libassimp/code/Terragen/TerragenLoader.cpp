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

/** @file Implementation of the Terragen importer class */



#ifndef ASSIMP_BUILD_NO_TERRAGEN_IMPORTER

#include "TerragenLoader.h"
#include <assimp/StreamReader.h>
#include <assimp/Importer.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "Terragen Heightmap Importer",
    "",
    "",
    "http://www.planetside.co.uk/",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "ter"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
TerragenImporter::TerragenImporter()
: configComputeUVs (false)
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
TerragenImporter::~TerragenImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool TerragenImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    // check file extension
    std::string extension = GetExtension(pFile);

    if( extension == "ter")
        return true;

    if(  !extension.length() || checkSig)   {
        /*  If CanRead() is called in order to check whether we
         *  support a specific file extension in general pIOHandler
         *  might be NULL and it's our duty to return true here.
         */
        if (!pIOHandler)return true;
        const char* tokens[] = {"terragen"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Build a string of all file extensions supported
const aiImporterDesc* TerragenImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Setup import properties
void TerragenImporter::SetupProperties(const Importer* pImp)
{
    // AI_CONFIG_IMPORT_TER_MAKE_UVS
    configComputeUVs = ( 0 != pImp->GetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS,0) );
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void TerragenImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    IOStream* file = pIOHandler->Open( pFile, "rb");

    // Check whether we can read from the file
    if( file == NULL)
        throw DeadlyImportError( "Failed to open TERRAGEN TERRAIN file " + pFile + ".");

    // Construct a stream reader to read all data in the correct endianness
    StreamReaderLE reader(file);
    if(reader.GetRemainingSize() < 16)
        throw DeadlyImportError( "TER: file is too small" );

    // Check for the existence of the two magic strings 'TERRAGEN' and 'TERRAIN '
    if (::strncmp((const char*)reader.GetPtr(),AI_TERR_BASE_STRING,8))
        throw DeadlyImportError( "TER: Magic string \'TERRAGEN\' not found" );

    if (::strncmp((const char*)reader.GetPtr()+8,AI_TERR_TERRAIN_STRING,8))
        throw DeadlyImportError( "TER: Magic string \'TERRAIN\' not found" );

    unsigned int x = 0,y = 0,mode = 0;

    aiNode* root = pScene->mRootNode = new aiNode();
    root->mName.Set("<TERRAGEN.TERRAIN>");

    // Default scaling is 30
    root->mTransformation.a1 = root->mTransformation.b2 = root->mTransformation.c3 = 30.f;

    // Now read all chunks until we're finished or an EOF marker is encountered
    reader.IncPtr(16);
    while (reader.GetRemainingSize() >= 4)
    {
        const char* head = (const char*)reader.GetPtr();
        reader.IncPtr(4);

        // EOF, break in every case
        if (!::strncmp(head,AI_TERR_EOF_STRING,4))
            break;

        // Number of x-data points
        if (!::strncmp(head,AI_TERR_CHUNK_XPTS,4))
        {
            x = (uint16_t)reader.GetI2();
        }
        // Number of y-data points
        else if (!::strncmp(head,AI_TERR_CHUNK_YPTS,4))
        {
            y = (uint16_t)reader.GetI2();
        }
        // Squared terrains width-1.
        else if (!::strncmp(head,AI_TERR_CHUNK_SIZE,4))
        {
            x = y = (uint16_t)reader.GetI2()+1;
        }
        // terrain scaling
        else if (!::strncmp(head,AI_TERR_CHUNK_SCAL,4))
        {
            root->mTransformation.a1 = reader.GetF4();
            root->mTransformation.b2 = reader.GetF4();
            root->mTransformation.c3 = reader.GetF4();
        }
        // mapping == 1: earth radius
        else if (!::strncmp(head,AI_TERR_CHUNK_CRAD,4))
        {
            reader.GetF4();
        }
        // mapping mode
        else if (!::strncmp(head,AI_TERR_CHUNK_CRVM,4))
        {
            mode = reader.GetI1();
            if (0 != mode)
                ASSIMP_LOG_ERROR("TER: Unsupported mapping mode, a flat terrain is returned");
        }
        // actual terrain data
        else if (!::strncmp(head,AI_TERR_CHUNK_ALTW,4))
        {
            float hscale  = (float)reader.GetI2()  / 65536;
            float bheight = (float)reader.GetI2();

            if (!hscale)hscale = 1;

            // Ensure we have enough data
            if (reader.GetRemainingSize() < x*y*2)
                throw DeadlyImportError("TER: ALTW chunk is too small");

            if (x <= 1 || y <= 1)
                throw DeadlyImportError("TER: Invalid terrain size");

            // Allocate the output mesh
            pScene->mMeshes = new aiMesh*[pScene->mNumMeshes = 1];
            aiMesh* m = pScene->mMeshes[0] = new aiMesh();

            // We return quads
            aiFace* f = m->mFaces = new aiFace[m->mNumFaces = (x-1)*(y-1)];
            aiVector3D* pv = m->mVertices = new aiVector3D[m->mNumVertices = m->mNumFaces*4];

            aiVector3D *uv( NULL );
            float step_y( 0.0f ), step_x( 0.0f );
            if (configComputeUVs) {
                uv = m->mTextureCoords[0] = new aiVector3D[m->mNumVertices];
                step_y = 1.f/y;
                step_x = 1.f/x;
            }
            const int16_t* data = (const int16_t*)reader.GetPtr();

            for (unsigned int yy = 0, t = 0; yy < y-1;++yy) {
                for (unsigned int xx = 0; xx < x-1;++xx,++f)    {

                    // make verts
                    const float fy = (float)yy, fx = (float)xx;
                    unsigned tmp,tmp2;
                    *pv++ = aiVector3D(fx,fy,    (float)data[(tmp2=x*yy)    + xx] * hscale + bheight);
                    *pv++ = aiVector3D(fx,fy+1,  (float)data[(tmp=x*(yy+1)) + xx] * hscale + bheight);
                    *pv++ = aiVector3D(fx+1,fy+1,(float)data[tmp  + xx+1]         * hscale + bheight);
                    *pv++ = aiVector3D(fx+1,fy,  (float)data[tmp2 + xx+1]         * hscale + bheight);

                    // also make texture coordinates, if necessary
                    if (configComputeUVs) {
                        *uv++ = aiVector3D( step_x*xx,     step_y*yy,     0.f );
                        *uv++ = aiVector3D( step_x*xx,     step_y*(yy+1), 0.f );
                        *uv++ = aiVector3D( step_x*(xx+1), step_y*(yy+1), 0.f );
                        *uv++ = aiVector3D( step_x*(xx+1), step_y*yy,     0.f );
                    }

                    // make indices
                    f->mIndices = new unsigned int[f->mNumIndices = 4];
                    for (unsigned int i = 0; i < 4;++i)
                        f->mIndices[i] = t++;
                }
            }

            // Add the mesh to the root node
            root->mMeshes = new unsigned int[root->mNumMeshes = 1];
            root->mMeshes[0] = 0;
        }

        // Get to the next chunk (4 byte aligned)
        unsigned dtt;
        if ((dtt = reader.GetCurrentPos() & 0x3))
            reader.IncPtr(4-dtt);
    }

    // Check whether we have a mesh now
    if (pScene->mNumMeshes != 1)
        throw DeadlyImportError("TER: Unable to load terrain");

    // Set the AI_SCENE_FLAGS_TERRAIN bit
    pScene->mFlags |= AI_SCENE_FLAGS_TERRAIN;
}

#endif // !! ASSIMP_BUILD_NO_TERRAGEN_IMPORTER
