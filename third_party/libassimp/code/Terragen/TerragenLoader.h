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

/** @file  TerragenLoader.h
 *  @brief Declaration of the .ter importer class.
 */
#ifndef INCLUDED_AI_TERRAGEN_TERRAIN_LOADER_H
#define INCLUDED_AI_TERRAGEN_TERRAIN_LOADER_H

#include <assimp/BaseImporter.h>
namespace Assimp    {

// Magic strings
#define AI_TERR_BASE_STRING         "TERRAGEN"
#define AI_TERR_TERRAIN_STRING      "TERRAIN "
#define AI_TERR_EOF_STRING          "EOF "

// Chunka
#define AI_TERR_CHUNK_XPTS          "XPTS"
#define AI_TERR_CHUNK_YPTS          "YPTS"
#define AI_TERR_CHUNK_SIZE          "SIZE"
#define AI_TERR_CHUNK_SCAL          "SCAL"
#define AI_TERR_CHUNK_CRAD          "CRAD"
#define AI_TERR_CHUNK_CRVM          "CRVM"
#define AI_TERR_CHUNK_ALTW          "ALTW"

// ---------------------------------------------------------------------------
/** @brief Importer class to load Terragen (0.9) terrain files.
 *
 *  The loader is basing on the information found here:
 *  http://www.planetside.co.uk/terragen/dev/tgterrain.html#chunks
*/
class TerragenImporter : public BaseImporter
{
public:
    TerragenImporter();
    ~TerragenImporter();


public:

    // -------------------------------------------------------------------
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

    // -------------------------------------------------------------------
    void SetupProperties(const Importer* pImp);

private:

    bool configComputeUVs;

}; //! class TerragenImporter

} // end of namespace Assimp

#endif // AI_AC3DIMPORTER_H_INC
