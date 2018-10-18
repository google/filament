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
//!
//! @file Data structures for the 3D Game Studio Heightmap format (HMP)
//!

namespace Assimp    {
namespace HMP   {

#include <assimp/Compiler/pushpack1.h>
#include <stdint.h>

// to make it easier for us, we test the magic word against both "endianesses"
#define AI_HMP_MAGIC_NUMBER_BE_4    AI_MAKE_MAGIC("HMP4")
#define AI_HMP_MAGIC_NUMBER_LE_4    AI_MAKE_MAGIC("4PMH")

#define AI_HMP_MAGIC_NUMBER_BE_5    AI_MAKE_MAGIC("HMP5")
#define AI_HMP_MAGIC_NUMBER_LE_5    AI_MAKE_MAGIC("5PMH")

#define AI_HMP_MAGIC_NUMBER_BE_7    AI_MAKE_MAGIC("HMP7")
#define AI_HMP_MAGIC_NUMBER_LE_7    AI_MAKE_MAGIC("7PMH")

// ---------------------------------------------------------------------------
/** Data structure for the header of a HMP5 file.
 *  This is also used by HMP4 and HMP7, but with modifications
*/
struct Header_HMP5
{
    int8_t  ident[4]; // "HMP5"
    int32_t     version;

    // ignored
    float   scale[3];
    float   scale_origin[3];
    float   boundingradius;

    //! Size of one triangle in x direction
    float   ftrisize_x;
    //! Size of one triangle in y direction
    float   ftrisize_y;
    //! Number of vertices in x direction
    float   fnumverts_x;

    //! Number of skins in the file
    int32_t     numskins;

    // can ignore this?
    int32_t     skinwidth;
    int32_t     skinheight;

    //!Number of vertices in the file
    int32_t     numverts;

    // ignored and zero
    int32_t     numtris;

    //! only one supported ...
    int32_t     numframes;

    //! Always 0 ...
    int32_t     num_stverts;
    int32_t     flags;
    float   size;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** Data structure for a terrain vertex in a HMP4 file
*/
struct Vertex_HMP4
{
    uint16_t p_pos[3];
    uint8_t normals162index;
    uint8_t pad;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** Data structure for a terrain vertex in a HMP5 file
*/
struct Vertex_HMP5
{
    uint16_t z;
    uint8_t normals162index;
    uint8_t pad;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** Data structure for a terrain vertex in a HMP7 file
*/
struct Vertex_HMP7
{
    uint16_t     z;
    int8_t normal_x,normal_y;
} PACK_STRUCT;

#include <assimp/Compiler/poppack1.h>

} //! namespace HMP
} //! namespace Assimp
