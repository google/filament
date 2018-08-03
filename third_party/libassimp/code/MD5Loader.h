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


/** @file   MD5Loader.h
 *  @brief Definition of the .MD5 importer class.
 *  http://www.modwiki.net/wiki/MD5_(file_format)
*/
#ifndef AI_MD5LOADER_H_INCLUDED
#define AI_MD5LOADER_H_INCLUDED

#include "BaseImporter.h"
#include "MD5Parser.h"

#include <assimp/types.h>

struct aiNode;
struct aiNodeAnim;

namespace Assimp    {

class IOStream;
using namespace Assimp::MD5;

// ---------------------------------------------------------------------------
/** Importer class for the MD5 file format
*/
class MD5Importer : public BaseImporter
{
public:
    MD5Importer();
    ~MD5Importer();


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
    /** Called prior to ReadFile().
     * The function is a request to the importer to update its configuration
     * basing on the Importer's configuration property list.
     */
    void SetupProperties(const Importer* pImp);

    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
     * See BaseImporter::InternReadFile() for details
     */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

protected:


    // -------------------------------------------------------------------
    /** Load a *.MD5MESH file.
     */
    void LoadMD5MeshFile ();

    // -------------------------------------------------------------------
    /** Load a *.MD5ANIM file.
     */
    void LoadMD5AnimFile ();

    // -------------------------------------------------------------------
    /** Load a *.MD5CAMERA file.
     */
    void LoadMD5CameraFile ();

    // -------------------------------------------------------------------
    /** Construct node hierarchy from a given MD5ANIM
     *  @param iParentID Current parent ID
     *  @param piParent Parent node to attach to
     *  @param bones Input bones
     *  @param node_anims Generated node animations
    */
    void AttachChilds_Anim(int iParentID,aiNode* piParent,
        AnimBoneList& bones,const aiNodeAnim** node_anims);

    // -------------------------------------------------------------------
    /** Construct node hierarchy from a given MD5MESH
     *  @param iParentID Current parent ID
     *  @param piParent Parent node to attach to
     *  @param bones Input bones
    */
    void AttachChilds_Mesh(int iParentID,aiNode* piParent,BoneList& bones);

    // -------------------------------------------------------------------
    /** Build unique vertex buffers from a given MD5ANIM
     *  @param meshSrc Input data
     */
    void MakeDataUnique (MD5::MeshDesc& meshSrc);

    // -------------------------------------------------------------------
    /** Load the contents of a specific file into memory and
     *  alocates a buffer to keep it.
     *
     *  mBuffer is modified to point to this buffer.
     *  @param pFile File stream to be read
    */
    void LoadFileIntoMemory (IOStream* pFile);
    void UnloadFileFromMemory ();


    /** IOSystem to be used to access files */
    IOSystem* mIOHandler;

    /** Path to the file, excluding the file extension but
        with the dot */
    std::string mFile;

    /** Buffer to hold the loaded file */
    char* mBuffer;

    /** Size of the file */
    unsigned int fileSize;

    /** Current line number. For debugging purposes */
    unsigned int iLineNumber;

    /** Scene to be filled */
    aiScene* pScene;

    /** (Custom) I/O handler implementation */
    IOSystem* pIOHandler;

    /** true if a MD5MESH file has already been parsed */
    bool bHadMD5Mesh;

    /** true if a MD5ANIM file has already been parsed */
    bool bHadMD5Anim;

    /** true if a MD5CAMERA file has already been parsed */
    bool bHadMD5Camera;

    /** configuration option: prevent anim autoload */
    bool configNoAutoLoad;
};

} // end of namespace Assimp

#endif // AI_3DSIMPORTER_H_INC
