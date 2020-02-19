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

/** @file  SIBImporter.h
 *  @brief Declaration of the SIB importer class.
 */
#ifndef AI_SIBIMPORTER_H_INCLUDED
#define AI_SIBIMPORTER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include <assimp/types.h>
#include <vector>

namespace Assimp    {

// ---------------------------------------------------------------------------
/** Importer class for the Nevercenter Silo SIB scene format
*/
class ASSIMP_API SIBImporter : public BaseImporter
{
public:
    SIBImporter();
    ~SIBImporter();

public:
    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
     * See BaseImporter::CanRead() for details.
     */
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
    struct MeshInformation
    {
        explicit MeshInformation(const std::string& _name)
            : name(_name)
        {
            vertices.reserve(100);
            colors.reserve(100);
        }

        std::string name;

        std::vector<aiVector3D> vertices;
        std::vector<aiColor4D> colors;
    };

    struct GroupInformation
    {
        explicit GroupInformation(const std::string& _name)
            : name(_name)
        {
            meshes.reserve(10);
        }

        std::string name;
        std::vector<MeshInformation> meshes;
    };
};

} // end of namespace Assimp

#endif // AI_SIBIMPORTER_H_INCLUDED
