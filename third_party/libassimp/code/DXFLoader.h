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

/** @file  DXFLoader.h
 *  @brief Declaration of the .dxf importer class.
 */
#ifndef AI_DXFLOADER_H_INCLUDED
#define AI_DXFLOADER_H_INCLUDED

#include "BaseImporter.h"
#include <map>

namespace Assimp    {
    namespace DXF {

        class LineReader;
        struct FileData;
        struct PolyLine;
        struct Block;
        struct InsertBlock;

        typedef std::map<std::string, const DXF::Block*> BlockMap;
    }


// ---------------------------------------------------------------------------
/** DXF importer implementation.
 *
*/
class DXFImporter : public BaseImporter
{
public:
    DXFImporter();
    ~DXFImporter();



public:

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
    * See BaseImporter::CanRead() for details.  */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details*/
    const aiImporterDesc* GetInfo () const;

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details */
    void InternReadFile( const std::string& pFile,
        aiScene* pScene,
        IOSystem* pIOHandler);

private:

    // -----------------------------------------------------
    void SkipSection(DXF::LineReader& reader);

    // -----------------------------------------------------
    void ParseHeader(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParseEntities(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParseBlocks(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParseBlock(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParseInsertion(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParsePolyLine(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ParsePolyLineVertex(DXF::LineReader& reader,
        DXF::PolyLine& line);

    // -----------------------------------------------------
    void Parse3DFace(DXF::LineReader& reader,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ConvertMeshes(aiScene* pScene,
        DXF::FileData& output);

    // -----------------------------------------------------
    void GenerateHierarchy(aiScene* pScene,
        DXF::FileData& output);

    // -----------------------------------------------------
    void GenerateMaterials(aiScene* pScene,
        DXF::FileData& output);

    // -----------------------------------------------------
    void ExpandBlockReferences(DXF::Block& bl,
        const DXF::BlockMap& blocks_by_name);
};

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_INC
