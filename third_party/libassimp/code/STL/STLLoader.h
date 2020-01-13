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

/** @file STLLoader.h
 *  Declaration of the STL importer class.
 */
#ifndef AI_STLLOADER_H_INCLUDED
#define AI_STLLOADER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include <assimp/types.h>

// Forward declarations
struct aiNode;

namespace Assimp {


// ---------------------------------------------------------------------------
/**
 * @brief   Importer class for the sterolithography STL file format.
 */
class STLImporter : public BaseImporter {
public:
    /**
     * @brief STLImporter, the class default constructor.
     */
    STLImporter();

    /**
     * @brief   The class destructor.
     */
    ~STLImporter();

    /**
     * @brief   Returns whether the class can handle the format of the given file.
     *  See BaseImporter::CanRead() for details.
     */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const;

protected:

    /**
     * @brief   Return importer meta information.
     *  See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;

    /**
     * @brief   Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details
    */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

    /**
     * @brief   Loads a binary .stl file
     * @return true if the default vertex color must be used as material color
     */
    bool LoadBinaryFile();

    /**
     * @brief   Loads a ASCII text .stl file
     */
    void LoadASCIIFile( aiNode *root );

    void pushMeshesToNode( std::vector<unsigned int> &meshIndices, aiNode *node );

protected:

    /** Buffer to hold the loaded file */
    const char* mBuffer;

    /** Size of the file, in bytes */
    unsigned int fileSize;

    /** Output scene */
    aiScene* pScene;

    /** Default vertex color */
    aiColor4D clrColorDefault;
};

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_IN
