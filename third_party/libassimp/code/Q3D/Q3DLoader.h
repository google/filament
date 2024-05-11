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

/** @file  Q3DLoader.h
 *  @brief Declaration of the Q3D importer class.
 */
#ifndef AI_Q3DLOADER_H_INCLUDED
#define AI_Q3DLOADER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include <assimp/types.h>
#include <vector>
#include <stdint.h>

namespace Assimp    {

// ---------------------------------------------------------------------------
/** Importer class for the Quick3D Object and Scene formats.
*/
class Q3DImporter : public BaseImporter
{
public:
    Q3DImporter();
    ~Q3DImporter();


public:

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
    * See BaseImporter::CanRead() for details.  */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details
    */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

private:

    struct Material
    {
        Material()
            :   diffuse         (0.6f,0.6f,0.6f)
            ,   transparency    (0.f)
            ,   texIdx          (UINT_MAX)
        {}

        aiString name;
        aiColor3D ambient, diffuse, specular;
        float transparency;

        unsigned int texIdx;
    };

    struct Face
    {
        explicit Face(unsigned int s)
            :   indices   (s)
            ,   uvindices (s)
            ,   mat       (0)
        {
        }

        std::vector<unsigned int> indices;
        std::vector<unsigned int> uvindices;
        unsigned int mat;
    };

    struct Mesh
    {

        std::vector<aiVector3D> verts;
        std::vector<aiVector3D> normals;
        std::vector<aiVector3D> uv;
        std::vector<Face>       faces;

        uint32_t prevUVIdx;
    };
};

} // end of namespace Assimp

#endif // AI_Q3DIMPORTER_H_IN
