/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2018, assimp team


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
/** @file  HMPLoader.h
 *  @brief Declaration of the HMP importer class
 */

#ifndef AI_HMPLOADER_H_INCLUDED
#define AI_HMPLOADER_H_INCLUDED

// internal headers
#include <assimp/BaseImporter.h>
#include "MDLLoader.h"
#include "HMPFileData.h"

namespace Assimp {
using namespace HMP;

// ---------------------------------------------------------------------------
/** Used to load 3D GameStudio HMP files (terrains)
*/
class HMPImporter : public MDLImporter
{
public:
    HMPImporter();
    ~HMPImporter();


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

protected:

    // -------------------------------------------------------------------
    /** Import a HMP4 file
    */
    void InternReadFile_HMP4( );

    // -------------------------------------------------------------------
    /** Import a HMP5 file
    */
    void InternReadFile_HMP5( );

    // -------------------------------------------------------------------
    /** Import a HMP7 file
    */
    void InternReadFile_HMP7( );

    // -------------------------------------------------------------------
    /** Validate a HMP 5,4,7 file header
    */
    void ValidateHeader_HMP457( );

    // -------------------------------------------------------------------
    /** Try to load one material from the file, if this fails create
     * a default material
    */
    void CreateMaterial(const unsigned char* szCurrent,
        const unsigned char** szCurrentOut);

    // -------------------------------------------------------------------
    /** Build a list of output faces and vertices. The function
     *  triangulates the height map read from the file
     * \param width Width of the height field
     * \param width Height of the height field
    */
    void CreateOutputFaceList(unsigned int width,unsigned int height);

    // -------------------------------------------------------------------
    /** Generate planar texture coordinates for a terrain
     * \param width Width of the terrain, in vertices
     * \param height Height of the terrain, in vertices
    */
    void GenerateTextureCoords(const unsigned int width,
        const unsigned int height);

    // -------------------------------------------------------------------
    /** Read the first skin from the file and skip all others ...
     *  \param iNumSkins Number of skins in the file
     *  \param szCursor Position of the first skin (offset 84)
    */
    void ReadFirstSkin(unsigned int iNumSkins, const unsigned char* szCursor,
        const unsigned char** szCursorOut);

private:

};

} // end of namespace Assimp

#endif // AI_HMPIMPORTER_H_INC

