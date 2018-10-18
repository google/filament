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

/** @file Defines the helper data structures for importing MDC files

**********************************************************************
File format specification:
http://themdcfile.planetwolfenstein.gamespy.com/MDC_File_Format.pdf
**********************************************************************

*/
#ifndef AI_MDCFILEHELPER_H_INC
#define AI_MDCFILEHELPER_H_INC

#include <assimp/types.h>
#include <assimp/mesh.h>
#include <assimp/anim.h>

#include <assimp/Compiler/pushpack1.h>
#include <stdint.h>

namespace Assimp {
namespace MDC {

// to make it easier for us, we test the magic word against both "endianesses"
#define AI_MDC_MAGIC_NUMBER_BE  AI_MAKE_MAGIC("CPDI")
#define AI_MDC_MAGIC_NUMBER_LE  AI_MAKE_MAGIC("IDPC")

// common limitations
#define AI_MDC_VERSION          2
#define AI_MDC_MAXQPATH         64
#define AI_MDC_MAX_BONES        128

#define AI_MDC_CVERT_BIAS       127.0f
#define AI_MDC_DELTA_SCALING    4.0f
#define AI_MDC_BASE_SCALING     (1.0f / 64.0f)


// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC file's main header
 */
struct Header {
    uint32_t ulIdent ;
    uint32_t ulVersion ;
    char ucName [ AI_MDC_MAXQPATH ] ;
    uint32_t ulFlags ;
    uint32_t ulNumFrames ;
    uint32_t ulNumTags ;
    uint32_t ulNumSurfaces ;
    uint32_t ulNumSkins ;
    uint32_t ulOffsetBorderFrames ;
    uint32_t ulOffsetTagNames ;
    uint32_t ulOffsetTagFrames ;
    uint32_t ulOffsetSurfaces ;
    uint32_t ulOffsetEnd ;
} PACK_STRUCT ;


// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC file's surface header
 */
struct Surface {
    uint32_t ulIdent ;
    char ucName [ AI_MDC_MAXQPATH ] ;
    uint32_t ulFlags ;
    uint32_t ulNumCompFrames ;
    uint32_t ulNumBaseFrames ;
    uint32_t ulNumShaders ;
    uint32_t ulNumVertices ;
    uint32_t ulNumTriangles ;
    uint32_t ulOffsetTriangles ;
    uint32_t ulOffsetShaders ;
    uint32_t ulOffsetTexCoords ;
    uint32_t ulOffsetBaseVerts ;
    uint32_t ulOffsetCompVerts ;
    uint32_t ulOffsetFrameBaseFrames ;
    uint32_t ulOffsetFrameCompFrames ;
    uint32_t ulOffsetEnd;
    Surface() AI_NO_EXCEPT
    : ulIdent()
    , ulFlags()
    , ulNumCompFrames()
    , ulNumBaseFrames()
    , ulNumShaders() 
    , ulNumVertices()
    , ulNumTriangles()
    , ulOffsetTriangles()
    , ulOffsetShaders()
    , ulOffsetTexCoords()
    , ulOffsetBaseVerts() 
    , ulOffsetCompVerts()
    , ulOffsetFrameBaseFrames()
    , ulOffsetFrameCompFrames()
    , ulOffsetEnd() {
        ucName[AI_MDC_MAXQPATH-1] = '\0';
    }
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC frame
 */
struct Frame {
    //! bounding box minimum coords
    aiVector3D bboxMin ;

    //! bounding box maximum coords
    aiVector3D bboxMax ;

    //! local origin of the frame
    aiVector3D localOrigin ;

    //! radius of the BB
    float radius ;

    //! Name of the frame
    char name [ 16 ] ;
} /*PACK_STRUCT*/;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC triangle
 */
struct Triangle {
    uint32_t aiIndices[3];
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC texture coordinate
 */
struct TexturCoord {
    float u,v;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC base vertex
 */
struct BaseVertex {
    int16_t x,y,z;
    uint16_t normal;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC compressed vertex
 */
struct CompressedVertex {
    uint8_t xd,yd,zd,nd;
} PACK_STRUCT;

// ---------------------------------------------------------------------------
/** \brief Data structure for a MDC shader
 */
struct Shader {
    char ucName [ AI_MDC_MAXQPATH ] ;
    uint32_t ulPath;
} PACK_STRUCT;

#include <assimp/Compiler/poppack1.h>

// ---------------------------------------------------------------------------
/** Build a floating point vertex from the compressed data in MDC files
 */
void BuildVertex(const Frame& frame,
    const BaseVertex& bvert,
    const CompressedVertex& cvert,
    aiVector3D& vXYZOut,
    aiVector3D& vNorOut);
}
}

#endif // !! AI_MDCFILEHELPER_H_INC
