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

/** @file  UnrealLoader.h
 *  @brief Declaration of the .3d (UNREAL) importer class.
 */
#ifndef INCLUDED_AI_3D_LOADER_H
#define INCLUDED_AI_3D_LOADER_H

#include <assimp/BaseImporter.h>
#include <stdint.h>

namespace Assimp    {
namespace Unreal {

    /*
    0 = Normal one-sided
    1 = Normal two-sided
    2 = Translucent two-sided
    3 = Masked two-sided
    4 = Modulation blended two-sided
    8 = Placeholder triangle for weapon positioning (invisible)
    */
enum MeshFlags {
    MF_NORMAL_OS            = 0,
    MF_NORMAL_TS            = 1,
    MF_NORMAL_TRANS_TS      = 2,
    MF_NORMAL_MASKED_TS     = 3,
    MF_NORMAL_MOD_TS        = 4,
    MF_WEAPON_PLACEHOLDER   = 8
};

    // a single triangle
struct Triangle {
   uint16_t mVertex[3];       // Vertex indices
   char mType;                // James' Mesh Type
   char mColor;               // Color for flat and Gourand Shaded
   unsigned char mTex[3][2];  // Texture UV coordinates
   unsigned char mTextureNum; // Source texture offset
   char mFlags;               // Unreal Mesh Flags (unused)

   unsigned int matIndex;
};

// temporary representation for a material
struct TempMat  {
    TempMat()
        :   type()
        ,   tex()
        ,   numFaces    (0)
    {}

    explicit TempMat(const Triangle& in)
        :   type        ((Unreal::MeshFlags)in.mType)
        ,   tex         (in.mTextureNum)
        ,   numFaces    (0)
    {}

    // type of mesh
    Unreal::MeshFlags type;

    // index of texture
    unsigned int tex;

    // number of faces using us
    unsigned int numFaces;

    // for std::find
    bool operator == (const TempMat& o )    {
        return (tex == o.tex && type == o.type);
    }
};

struct Vertex
{
    int32_t X : 11;
    int32_t Y : 11;
    int32_t Z : 10;
};

    // UNREAL vertex compression
inline void CompressVertex(const aiVector3D& v, uint32_t& out)
{
    union {
        Vertex n;
        int32_t t;
    };
    n.X = (int32_t)v.x;
    n.Y = (int32_t)v.y;
    n.Z = (int32_t)v.z;
    ::memcpy( &out, &t, sizeof(int32_t));
    //out = t;
}

    // UNREAL vertex decompression
inline void DecompressVertex(aiVector3D& v, int32_t in)
{
    union {
        Vertex n;
        int32_t i;
    };
    i = in;

    v.x = (float)n.X;
    v.y = (float)n.Y;
    v.z = (float)n.Z;
}

} // end namespace Unreal

// ---------------------------------------------------------------------------
/** @brief Importer class to load UNREAL files (*.3d)
*/
class UnrealImporter : public BaseImporter
{
public:
    UnrealImporter();
    ~UnrealImporter();


public:

    // -------------------------------------------------------------------
    /** @brief Returns whether we can handle the format of the given file
     *
     *  See BaseImporter::CanRead() for details.
     **/
    bool CanRead( const std::string& pFile, IOSystem* pIOHandler,
        bool checkSig) const;

protected:

    // -------------------------------------------------------------------
    /** @brief Called by Importer::GetExtensionList()
     *
     * See #BaseImporter::GetInfo for the details
     */
    const aiImporterDesc* GetInfo () const;


    // -------------------------------------------------------------------
    /** @brief Setup properties for the importer
     *
     * See BaseImporter::SetupProperties() for details
     */
    void SetupProperties(const Importer* pImp);


    // -------------------------------------------------------------------
    /** @brief Imports the given file into the given scene structure.
     *
     * See BaseImporter::InternReadFile() for details
     */
    void InternReadFile( const std::string& pFile, aiScene* pScene,
        IOSystem* pIOHandler);

private:

    //! frame to be loaded
    uint32_t configFrameID;

    //! process surface flags
    bool configHandleFlags;

}; // !class UnrealImporter

} // end of namespace Assimp
#endif // AI_UNREALIMPORTER_H_INC
