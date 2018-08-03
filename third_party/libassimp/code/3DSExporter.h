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

/** @file 3DSExporter.h
 * 3DS Exporter Main Header
 */
#ifndef AI_3DSEXPORTER_H_INC
#define AI_3DSEXPORTER_H_INC

#include <map>
#include <memory>

#include "StreamWriter.h"
#include <assimp/material.h>

struct aiScene;
struct aiNode;
struct aiMaterial;
struct aiMesh;

namespace Assimp
{

// ------------------------------------------------------------------------------------------------
/**
 *  @brief  Helper class to export a given scene to a 3DS file.
 */
// ------------------------------------------------------------------------------------------------
class Discreet3DSExporter {
public:
    Discreet3DSExporter(std::shared_ptr<IOStream> outfile, const aiScene* pScene);
    ~Discreet3DSExporter();

private:
    void WriteMeshes();
    void WriteMaterials();
    void WriteTexture(const aiMaterial& mat, aiTextureType type, uint16_t chunk_flags);
    void WriteFaceMaterialChunk(const aiMesh& mesh);
    int WriteHierarchy(const aiNode& node, int level, int sibling_level);
    void WriteString(const std::string& s);
    void WriteString(const aiString& s);
    void WriteColor(const aiColor3D& color);
    void WritePercentChunk(float f);
    void WritePercentChunk(double f);

private:
    const aiScene* const scene;
    StreamWriterLE writer;

    std::map<const aiNode*, aiMatrix4x4> trafos;

    typedef std::multimap<const aiNode*, unsigned int> MeshesByNodeMap;
    MeshesByNodeMap meshes;

};

} // Namespace Assimp

#endif // AI_3DSEXPORTER_H_INC
