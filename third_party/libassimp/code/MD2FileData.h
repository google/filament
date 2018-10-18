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

/** @file  MD2FileData.h
 *  @brief Defines helper data structures for importing MD2 files
 *  http://linux.ucla.edu/~phaethon/q3/formats/md2-schoenblum.html
 */
#ifndef AI_MD2FILEHELPER_H_INC
#define AI_MD2FILEHELPER_H_INC

#include <assimp/types.h>
#include <stdint.h>

#include <assimp/Compiler/pushpack1.h>

namespace Assimp    {
namespace MD2   {

// to make it easier for us, we test the magic word against both "endianesses"
#define AI_MD2_MAGIC_NUMBER_BE  AI_MAKE_MAGIC("IDP2")
#define AI_MD2_MAGIC_NUMBER_LE  AI_MAKE_MAGIC("2PDI")

// common limitations
#define AI_MD2_VERSION          15
#define AI_MD2_MAXQPATH         64
#define AI_MD2_MAX_FRAMES       512
#define AI_MD2_MAX_SKINS        32
#define AI_MD2_MAX_VERTS        2048
#define AI_MD2_MAX_TRIANGLES    4096

// ---------------------------------------------------------------------------
/** \brief Data structure for the MD2 main header
 */
struct Header
{
    uint32_t magic;
    uint32_t version;
    uint32_t skinWidth;
    uint32_t skinHeight;
    uint32_t frameSize;
    uint32_t numSkins;
    uint32_t numVertices;
    uint32_t numTexCoords;
    uint32_t numTriangles;
    uint32_t numGlCommands;
    uint32_t numFrames;
    uint32_t offsetSkins;
    uint32_t offsetTexCoords;
    uint32_t offsetTriangles;
    uint32_t offsetFrames;
    uint32_t offsetGlCommands;
    uint32_t offsetEnd;

} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 OpenGl draw command
 */
struct GLCommand
{
   float s, t;
   uint32_t vertexIndex;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 triangle
 */
struct Triangle
{
    uint16_t vertexIndices[3];
    uint16_t textureIndices[3];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 vertex
 */
struct Vertex
{
    uint8_t vertex[3];
    uint8_t lightNormalIndex;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 frame
 */
struct Frame
{
    float scale[3];
    float translate[3];
    char name[16];
    Vertex vertices[1];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 texture coordinate
 */
struct TexCoord
{
    uint16_t s;
    uint16_t t;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MD2 skin
 */
struct Skin
{
    char name[AI_MD2_MAXQPATH];              /* texture file name */
} PACK_STRUCT;

#include <assimp/Compiler/poppack1.h>


// ---------------------------------------------------------------------------
//! Lookup a normal vector from Quake's normal lookup table
//! \param index Input index (0-161)
//! \param vOut Receives the output normal
void LookupNormalIndex(uint8_t index,aiVector3D& vOut);


}
}

#endif // !! include guard

