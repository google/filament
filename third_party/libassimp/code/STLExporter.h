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

/** @file STLExporter.h
 * Declares the exporter class to write a scene to a Stereolithography (STL) file
 */
#ifndef AI_STLEXPORTER_H_INC
#define AI_STLEXPORTER_H_INC

#include <sstream>

struct aiScene;
struct aiNode;
struct aiMesh;

namespace Assimp
{

// ------------------------------------------------------------------------------------------------
/** Helper class to export a given scene to a STL file. */
// ------------------------------------------------------------------------------------------------
class STLExporter
{
public:
    /// Constructor for a specific scene to export
    STLExporter(const char* filename, const aiScene* pScene, bool binary = false);

public:

    /// public stringstreams to write all output into
    std::ostringstream mOutput;

private:

    void WriteMesh(const aiMesh* m);
    void WriteMeshBinary(const aiMesh* m);

private:

    const std::string filename;

    // this endl() doesn't flush() the stream
    const std::string endl;
};

}

#endif
