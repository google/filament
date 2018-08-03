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

/** @file  MD2Loader.h
 *  @brief Declaration of the .MD2 importer class.
 */
#ifndef AI_MD2LOADER_H_INCLUDED
#define AI_MD2LOADER_H_INCLUDED

#include "BaseImporter.h"
#include <assimp/types.h>
#include "ByteSwapper.h"
#include "MD2FileData.h"

struct aiNode;

namespace Assimp  {

using namespace MD2;

// ---------------------------------------------------------------------------
/** Importer class for MD2
*/
class MD2Importer : public BaseImporter
{
public:
    MD2Importer();
    ~MD2Importer();


public:

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
    * See BaseImporter::CanRead() for details.  */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;


    // -------------------------------------------------------------------
    /** Called prior to ReadFile().
    * The function is a request to the importer to update its configuration
    * basing on the Importer's configuration property list.
    */
    void SetupProperties(const Importer* pImp);

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


    // -------------------------------------------------------------------
    /** Validate the header of the file
    */
    void ValidateHeader();

protected:

    /** Configuration option: frame to be loaded */
    unsigned int configFrameID;

    /** Header of the MD2 file */
    BE_NCONST MD2::Header* m_pcHeader;

    /** Buffer to hold the loaded file */
    BE_NCONST uint8_t* mBuffer;

    /** Size of the file, in bytes */
    unsigned int fileSize;
};

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_INC
