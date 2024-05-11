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



#if !defined(ASSIMP_BUILD_NO_EXPORT) && !defined(ASSIMP_BUILD_NO_STL_EXPORTER)

#include "STLExporter.h"
#include <assimp/version.h>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <memory>
#include <assimp/Exceptional.h>
#include <assimp/ByteSwapper.h>

using namespace Assimp;

namespace Assimp    {

// ------------------------------------------------------------------------------------------------
// Worker function for exporting a scene to Stereolithograpy. Prototyped and registered in Exporter.cpp
void ExportSceneSTL(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* pProperties )
{
    bool exportPointClouds = pProperties->GetPropertyBool(AI_CONFIG_EXPORT_POINT_CLOUDS);

    // invoke the exporter
    STLExporter exporter(pFile, pScene, exportPointClouds );

    if (exporter.mOutput.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }
    
    // we're still here - export successfully completed. Write the file.
    std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wt"));
    if(outfile == NULL) {
        throw DeadlyExportError("could not open output .stl file: " + std::string(pFile));
    }

    outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
}
void ExportSceneSTLBinary(const char* pFile,IOSystem* pIOSystem, const aiScene* pScene, const ExportProperties* pProperties )
{
    bool exportPointClouds = pProperties->GetPropertyBool(AI_CONFIG_EXPORT_POINT_CLOUDS);

    // invoke the exporter
    STLExporter exporter(pFile, pScene, exportPointClouds, true);

    if (exporter.mOutput.fail()) {
        throw DeadlyExportError("output data creation failed. Most likely the file became too large: " + std::string(pFile));
    }
    
    // we're still here - export successfully completed. Write the file.
    std::unique_ptr<IOStream> outfile (pIOSystem->Open(pFile,"wb"));
    if(outfile == NULL) {
        throw DeadlyExportError("could not open output .stl file: " + std::string(pFile));
    }

    outfile->Write( exporter.mOutput.str().c_str(), static_cast<size_t>(exporter.mOutput.tellp()),1);
}

} // end of namespace Assimp

static const char *SolidToken = "solid";
static const char *EndSolidToken = "endsolid";

// ------------------------------------------------------------------------------------------------
STLExporter::STLExporter(const char* _filename, const aiScene* pScene, bool exportPointClouds, bool binary)
: filename(_filename)
, endl("\n")
{
    // make sure that all formatting happens using the standard, C locale and not the user's current locale
    const std::locale& l = std::locale("C");
    mOutput.imbue(l);
    mOutput.precision(ASSIMP_AI_REAL_TEXT_PRECISION);
    if (binary) {
        char buf[80] = {0} ;
        buf[0] = 'A'; buf[1] = 's'; buf[2] = 's'; buf[3] = 'i'; buf[4] = 'm'; buf[5] = 'p';
        buf[6] = 'S'; buf[7] = 'c'; buf[8] = 'e'; buf[9] = 'n'; buf[10] = 'e';
        mOutput.write(buf, 80);
        unsigned int meshnum = 0;
        for(unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
            for (unsigned int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j) {
                meshnum++;
            }
        }
        AI_SWAP4(meshnum);
        mOutput.write((char *)&meshnum, 4);

        if (exportPointClouds) {
            throw DeadlyExportError("This functionality is not yet implemented for binary output.");
        }

        for(unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
            WriteMeshBinary(pScene->mMeshes[i]);
        }
    } else {

        // Exporting only point clouds
        if (exportPointClouds) {
            WritePointCloud("Assimp_Pointcloud", pScene );
            return;
        } 

        // Export the assimp mesh 
        const std::string name = "AssimpScene";
        mOutput << SolidToken << " " << name << endl;
        for(unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
            WriteMesh(pScene->mMeshes[ i ]);
        }
        mOutput << EndSolidToken << name << endl;
    }
}

// ------------------------------------------------------------------------------------------------
void STLExporter::WritePointCloud(const std::string &name, const aiScene* pScene) {
    mOutput << " " << SolidToken << " " << name << endl;
    aiVector3D nor;
    mOutput << " facet normal " << nor.x << " " << nor.y << " " << nor.z << endl;
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        aiMesh *mesh = pScene->mMeshes[i];
        if (nullptr == mesh) {
            continue;
        }

        for (unsigned int a = 0; a < mesh->mNumVertices; ++a) {
            const aiVector3D& v = mesh->mVertices[a];
            mOutput << "  vertex " << v.x << " " << v.y << " " << v.z << endl;
            mOutput << "  vertex " << v.x << " " << v.y << " " << v.z << endl;
            mOutput << "  vertex " << v.x << " " << v.y << " " << v.z << endl;
        }
    }
    mOutput << EndSolidToken << " " << name << endl;
}

// ------------------------------------------------------------------------------------------------
void STLExporter::WriteMesh(const aiMesh* m)
{
    for (unsigned int i = 0; i < m->mNumFaces; ++i) {
        const aiFace& f = m->mFaces[i];

        // we need per-face normals. We specified aiProcess_GenNormals as pre-requisite for this exporter,
        // but nonetheless we have to expect per-vertex normals.
        aiVector3D nor;
        if (m->mNormals) {
            for(unsigned int a = 0; a < f.mNumIndices; ++a) {
                nor += m->mNormals[f.mIndices[a]];
            }
            nor.NormalizeSafe();
        }
        mOutput << " facet normal " << nor.x << " " << nor.y << " " << nor.z << endl;
        mOutput << "  outer loop" << endl;
        for(unsigned int a = 0; a < f.mNumIndices; ++a) {
            const aiVector3D& v  = m->mVertices[f.mIndices[a]];
            mOutput << "  vertex " << v.x << " " << v.y << " " << v.z << endl;
        }

        mOutput << "  endloop" << endl;
        mOutput << " endfacet" << endl << endl;
    }
}

void STLExporter::WriteMeshBinary(const aiMesh* m)
{
    for (unsigned int i = 0; i < m->mNumFaces; ++i) {
        const aiFace& f = m->mFaces[i];
        // we need per-face normals. We specified aiProcess_GenNormals as pre-requisite for this exporter,
        // but nonetheless we have to expect per-vertex normals.
        aiVector3D nor;
        if (m->mNormals) {
            for(unsigned int a = 0; a < f.mNumIndices; ++a) {
                nor += m->mNormals[f.mIndices[a]];
            }
            nor.Normalize();
        }
        // STL binary files use 4-byte floats. This may possibly cause loss of precision
        // for clients using 8-byte doubles
        float nx = (float) nor.x;
        float ny = (float) nor.y;
        float nz = (float) nor.z;
        AI_SWAP4(nx); AI_SWAP4(ny); AI_SWAP4(nz);
        mOutput.write((char *)&nx, 4); mOutput.write((char *)&ny, 4); mOutput.write((char *)&nz, 4);
        for(unsigned int a = 0; a < f.mNumIndices; ++a) {
            const aiVector3D& v  = m->mVertices[f.mIndices[a]];
            float vx = (float) v.x, vy = (float) v.y, vz = (float) v.z;
            AI_SWAP4(vx); AI_SWAP4(vy); AI_SWAP4(vz);
            mOutput.write((char *)&vx, 4); mOutput.write((char *)&vy, 4); mOutput.write((char *)&vz, 4);
        }
        char dummy[2] = {0};
        mOutput.write(dummy, 2);
    }
}

#endif
