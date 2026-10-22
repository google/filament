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
#pragma once

#include <memory>
#include <sstream>
#include <vector>
#include <assimp/vector3.h>

struct aiScene;
struct aiNode;
struct aiMaterial;
struct aiMesh;

struct zip_t;

namespace Assimp {

class IOStream;

namespace D3MF {

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_3MF_EXPORTER

struct OpcPackageRelationship;

class D3MFExporter {
public:
    D3MFExporter( const char* pFile, const aiScene* pScene );
    ~D3MFExporter();
    bool validate();
    bool exportArchive( const char *file );
    bool exportContentTypes();
    bool exportRelations();
    bool export3DModel();

protected:
    void writeHeader();
    void writeMetaData();
    void writeBaseMaterials();
    void writeObjects();
    void writeMesh( aiMesh *mesh );
    void writeVertex( const aiVector3D &pos );
    void writeFaces( aiMesh *mesh, unsigned int matIdx );
    void writeBuild();
    void exportContentTyp( const std::string &filename );
    void writeModelToArchive( const std::string &folder, const std::string &modelName );
    void writeRelInfoToFile( const std::string &folder, const std::string &relName );

private:
    std::string mArchiveName;
    zip_t *m_zipArchive;
    const aiScene *mScene;
    std::ostringstream mModelOutput;
    std::ostringstream mRelOutput;
    std::ostringstream mContentOutput;
    std::vector<unsigned int> mBuildItems;
    std::vector<OpcPackageRelationship*> mRelations;
};

#endif // ASSIMP_BUILD_NO_3MF_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT

} // Namespace D3MF
} // Namespace Assimp

