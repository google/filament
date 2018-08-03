/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2008, assimp team
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

/** @file NDOLoader.h
 *  Declaration of the Nendo importer class.
 */
#ifndef INCLUDED_AI_NDO_LOADER_H
#define INCLUDED_AI_NDO_LOADER_H

#include <assimp/vector3.h>
#include "BaseImporter.h"
#include <stdint.h>
#include <string>
#include <vector>


struct aiImporterDesc;
struct aiScene;

namespace Assimp    {
    class IOSystem;
    class Importer;

// ---------------------------------------------------------------------------
/** @brief Importer class to load meshes from Nendo.
 *
 *  Basing on
 *  <blender>/blender/release/scripts/nendo_import.py by Anthony D'Agostino.
*/
class NDOImporter : public BaseImporter
{
public:
    NDOImporter();
    ~NDOImporter();


public:

    //! Represents a single edge
    struct Edge
    {
        unsigned int edge[8];
        unsigned int hard;
        uint8_t color[8];
    };

    //! Represents a single face
    struct Face
    {
        unsigned int elem;
    };

    struct Vertex
    {
        unsigned int num;
        aiVector3D val;
    };

    //! Represents a single object
    struct Object
    {
        std::string name;

        std::vector<Edge> edges;
        std::vector<Face> faces;
        std::vector<Vertex> vertices;
    };

    // -------------------------------------------------------------------
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    void SetupProperties(const Importer* pImp);

    // -------------------------------------------------------------------
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

private:

}; // end of class NDOImporter
} // end of namespace Assimp
#endif // INCLUDED_AI_NDO_LOADER_H
