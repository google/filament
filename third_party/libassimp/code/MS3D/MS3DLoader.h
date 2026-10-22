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

/** @file  MS3DLoader.h
 *  @brief Declaration of the MS3D importer class.
 */
#ifndef AI_MS3DLOADER_H_INCLUDED
#define AI_MS3DLOADER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include <assimp/StreamReader.h>
struct aiNode;

namespace Assimp    {

// ----------------------------------------------------------------------------------------------
/** Milkshape 3D importer implementation */
// ----------------------------------------------------------------------------------------------
class MS3DImporter
    : public BaseImporter
{

public:

    MS3DImporter();
    ~MS3DImporter();

public:

    // -------------------------------------------------------------------
    /** Returns whether the class can handle the format of the given file.
    * See BaseImporter::CanRead() for details.  */
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    /** Return importer meta information.
     * See #BaseImporter::GetInfo for the details */
    const aiImporterDesc* GetInfo () const;


    // -------------------------------------------------------------------
    /** Imports the given file into the given scene structure.
    * See BaseImporter::InternReadFile() for details */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);


private:

    struct TempJoint;
    void CollectChildJoints(const std::vector<TempJoint>& joints, std::vector<bool>& hadit, aiNode* nd,const aiMatrix4x4& absTrafo);
    void CollectChildJoints(const std::vector<TempJoint>& joints, aiNode* nd);

    template<typename T> void ReadComments(StreamReaderLE& stream, std::vector<T>& outp);
private:

    aiScene* mScene;

private:

    struct TempVertex
    {
        aiVector3D pos;
        unsigned int bone_id[4], ref_cnt;
        float weights[4];
    };

    struct TempTriangle
    {
        unsigned int indices[3];
        aiVector3D normals[3];
        aiVector2D uv[3];

        unsigned int sg, group;
    };

    struct TempGroup
    {
        char name[33]; // +0
        std::vector<unsigned int> triangles;
        unsigned int mat; // 0xff is no material
        std::string comment;
    };

    struct TempMaterial
    {
        // again, add an extra 0 character to all strings -
        char name[33];
        char texture[129];
        char alphamap[129];

        aiColor4D diffuse,specular,ambient,emissive;
        float shininess,transparency;
        std::string comment;
    };

    struct TempKeyFrame
    {
        float time;
        aiVector3D value;
    };

    struct TempJoint
    {
        char name[33];
        char parentName[33];
        aiVector3D rotation, position;

        std::vector<TempKeyFrame> rotFrames;
        std::vector<TempKeyFrame> posFrames;
        std::string comment;
    };

    //struct TempModel {
    //  std::string comment;
    //};
};

}
#endif
