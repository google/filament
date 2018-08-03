/*
Open Asset Import Library (ASSIMP)
----------------------------------------------------------------------

Copyright (c) 2006-2010, ASSIMP Development Team
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

* Neither the name of the ASSIMP team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the ASSIMP Development Team.

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

/** @file Defines the helper data structures for importing MD4 files  */
#ifndef AI_MD4FILEHELPER_H_INC
#define AI_MD4FILEHELPER_H_INC

#include <string>
#include <vector>
#include <sstream>

#include "../include/assimp/types.h"
#include "../include/assimp/mesh.h"
#include "../include/assimp/anim.h"

#if defined(_MSC_VER) ||  defined(__BORLANDC__) ||  defined (__BCPLUSPLUS__)
#   pragma pack(push,1)
#   define PACK_STRUCT
#elif defined( __GNUC__ )
#   define PACK_STRUCT  __attribute__((packed))
#else
#   error Compiler not supported
#endif


namespace Assimp
{
// http://gongo.quakedev.com/md4.html
namespace MD4
{

#define AI_MD4_MAGIC_NUMBER_BE  'IDP4'
#define AI_MD4_MAGIC_NUMBER_LE  '4PDI'

// common limitations
#define AI_MD4_VERSION          4
#define AI_MD4_MAXQPATH         64
#define AI_MD4_MAX_FRAMES       2028
#define AI_MD4_MAX_SURFACES     32
#define AI_MD4_MAX_BONES        256
#define AI_MD4_MAX_VERTS        4096
#define AI_MD4_MAX_TRIANGLES    8192

// ---------------------------------------------------------------------------
/** \brief Data structure for the MD4 main header
 */
// ---------------------------------------------------------------------------
struct Header
{
    //! magic number
    int32_t magic;

    //! file format version
    int32_t version;

    //! original name in .pak archive
    unsigned char name[ AI_MD4_MAXQPATH ];

    //! number of frames in the file
    int32_t NUM_FRAMES;

    //! number of bones in the file
    int32_t NUM_BONES;

    //! number of surfaces in the file
    int32_t NUM_SURFACES;

    //! offset of the first frame
    int32_t OFS_FRAMES;

    //! offset of the first bone
    int32_t OFS_BONES;

    //! offset of the first surface
    int32_t OFS_SURFACES;

    //! end of file
    int32_t OFS_EOF;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Stores the local transformation matrix of a bone
 */
// ---------------------------------------------------------------------------
struct BoneFrame
{
    float matrix[3][4];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Stores the name / parent index / flag of a node
 */
// ---------------------------------------------------------------------------
struct  BoneName
{
    char name[32] ;
    int parent ;
    int flags ;
}  PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a surface in a MD4 file
 */
// ---------------------------------------------------------------------------
struct Surface
{
    int32_t ident;
    char name[64];
    char shader[64];
    int32_t shaderIndex;
    int32_t lodBias;
    int32_t minLod;
    int32_t ofsHeader;
    int32_t numVerts;
    int32_t ofsVerts;
    int32_t numTris;
    int32_t ofsTris;
    int32_t numBoneRefs;
    int32_t ofsBoneRefs;
    int32_t ofsCollapseMap;
    int32_t ofsEnd;
} PACK_STRUCT;


// ---------------------------------------------------------------------------
/** \brief Data structure for a MD4 vertex' weight
 */
// ---------------------------------------------------------------------------
struct Weight
{
    int32_t boneIndex;
    float boneWeight;
    float offset[3];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a vertex in a MD4 file
 */
// ---------------------------------------------------------------------------
struct Vertex
{
    float vertex[3];
    float normal[3];
    float texCoords[2];
    int32_t numWeights;
    Weight weights[1];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a triangle in a MD4 file
 */
// ---------------------------------------------------------------------------
struct Triangle
{
    int32_t indexes[3];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD4 frame
 */
// ---------------------------------------------------------------------------
struct Frame
{
    float bounds[3][2];
    float localOrigin[3];
    float radius;
    BoneFrame bones[1];
} PACK_STRUCT;


// reset packing to the original value
#if defined(_MSC_VER) ||  defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#   pragma pack( pop )
#endif
#undef PACK_STRUCT


};
};

#endif // !! AI_MD4FILEHELPER_H_INC
