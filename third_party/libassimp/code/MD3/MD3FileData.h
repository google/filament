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

/** @file Md3FileData.h
 *
 *  @brief Defines helper data structures for importing MD3 files.
 *  http://linux.ucla.edu/~phaethon/q3/formats/md3format.html
 */
#ifndef AI_MD3FILEHELPER_H_INC
#define AI_MD3FILEHELPER_H_INC

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>

#include <assimp/types.h>
#include <assimp/mesh.h>
#include <assimp/anim.h>

#include <assimp/Compiler/pushpack1.h>

namespace Assimp {
namespace MD3   {

// to make it easier for us, we test the magic word against both "endianesses"
#define AI_MD3_MAGIC_NUMBER_BE  AI_MAKE_MAGIC("IDP3")
#define AI_MD3_MAGIC_NUMBER_LE  AI_MAKE_MAGIC("3PDI")

// common limitations
#define AI_MD3_VERSION          15
#define AI_MD3_MAXQPATH         64
#define AI_MD3_MAXFRAME         16
#define AI_MD3_MAX_FRAMES       1024
#define AI_MD3_MAX_TAGS         16
#define AI_MD3_MAX_SURFACES     32
#define AI_MD3_MAX_SHADERS      256
#define AI_MD3_MAX_VERTS        4096
#define AI_MD3_MAX_TRIANGLES    8192

// master scale factor for all vertices in a MD3 model
#define AI_MD3_XYZ_SCALE        (1.0f/64.0f)

// -------------------------------------------------------------------------------
/** @brief Data structure for the MD3 main header
 */
struct Header
{
    //! magic number
    uint32_t IDENT;

    //! file format version
    uint32_t VERSION;

    //! original name in .pak archive
    char NAME[ AI_MD3_MAXQPATH ];

    //! unknown
    int32_t FLAGS;

    //! number of frames in the file
    uint32_t NUM_FRAMES;

    //! number of tags in the file
    uint32_t NUM_TAGS;

    //! number of surfaces in the file
    uint32_t NUM_SURFACES;

    //! number of skins in the file
    uint32_t NUM_SKINS;

    //! offset of the first frame
    uint32_t OFS_FRAMES;

    //! offset of the first tag
    uint32_t OFS_TAGS;

    //! offset of the first surface
    uint32_t OFS_SURFACES;

    //! end of file
    uint32_t OFS_EOF;
} PACK_STRUCT;


// -------------------------------------------------------------------------------
/** @brief Data structure for the frame header
 */
struct Frame
{
    //! minimum bounds
    aiVector3D min;

    //! maximum bounds
    aiVector3D max;

    //! local origin for this frame
    aiVector3D origin;

    //! radius of bounding sphere
    ai_real radius;

    //! name of frame
    char name[ AI_MD3_MAXFRAME ];

} /* PACK_STRUCT */;


// -------------------------------------------------------------------------------
/**
 * @brief Data structure for the tag header
 */
struct Tag {
    //! name of the tag
    char NAME[ AI_MD3_MAXQPATH ];

    //! Local tag origin and orientation
    aiVector3D  origin;
    ai_real  orientation[3][3];

} /* PACK_STRUCT */;


// -------------------------------------------------------------------------------
/** @brief Data structure for the surface header
 */
struct Surface {
    //! magic number
    int32_t IDENT;

    //! original name of the surface
    char NAME[ AI_MD3_MAXQPATH ];

    //! unknown
    int32_t FLAGS;

    //! number of frames in the surface
    uint32_t NUM_FRAMES;

    //! number of shaders in the surface
    uint32_t NUM_SHADER;

    //! number of vertices in the surface
    uint32_t NUM_VERTICES;

    //! number of triangles in the surface
    uint32_t NUM_TRIANGLES;

    //! offset to the triangle data
    uint32_t OFS_TRIANGLES;

    //! offset to the shader data
    uint32_t OFS_SHADERS;

    //! offset to the texture coordinate data
    uint32_t OFS_ST;

    //! offset to the vertex/normal data
    uint32_t OFS_XYZNORMAL;

    //! offset to the end of the Surface object
    int32_t OFS_END;
} /*PACK_STRUCT*/;

// -------------------------------------------------------------------------------
/** @brief Data structure for a shader defined in there
 */
struct Shader {
    //! filename of the shader
    char NAME[ AI_MD3_MAXQPATH ];

    //! index of the shader
    uint32_t SHADER_INDEX;
} /*PACK_STRUCT*/;


// -------------------------------------------------------------------------------
/** @brief Data structure for a triangle
 */
struct Triangle
{
    //! triangle indices
    uint32_t INDEXES[3];
} /*PACK_STRUCT*/;


// -------------------------------------------------------------------------------
/** @brief Data structure for an UV coord
 */
struct TexCoord
{
    //! UV coordinates
    ai_real U,V;
} /*PACK_STRUCT*/;


// -------------------------------------------------------------------------------
/** @brief Data structure for a vertex
 */
struct Vertex
{
    //! X/Y/Z coordinates
    int16_t X,Y,Z;

    //! encoded normal vector
    uint16_t  NORMAL;
} /*PACK_STRUCT*/;

#include <assimp/Compiler/poppack1.h>

// -------------------------------------------------------------------------------
/** @brief Unpack a Q3 16 bit vector to its full float3 representation
 *
 *  @param p_iNormal Input normal vector in latitude/longitude form
 *  @param p_afOut Pointer to an array of three floats to receive the result
 *
 *  @note This has been taken from q3 source (misc_model.c)
 */
inline void LatLngNormalToVec3(uint16_t p_iNormal, ai_real* p_afOut)
{
    ai_real lat = (ai_real)(( p_iNormal >> 8u ) & 0xff);
    ai_real lng = (ai_real)(( p_iNormal & 0xff ));
    const ai_real invVal( ai_real( 1.0 ) / ai_real( 128.0 ) );
    lat *= ai_real( 3.141926 ) * invVal;
    lng *= ai_real( 3.141926 ) * invVal;

    p_afOut[ 0 ] = std::cos(lat) * std::sin(lng);
    p_afOut[ 1 ] = std::sin(lat) * std::sin(lng);
    p_afOut[ 2 ] = std::cos(lng);
}


// -------------------------------------------------------------------------------
/** @brief Pack a Q3 normal into 16bit latitute/longitude representation
 *  @param p_vIn Input vector
 *  @param p_iOut Output normal
 *
 *  @note This has been taken from q3 source (mathlib.c)
 */
inline void Vec3NormalToLatLng( const aiVector3D& p_vIn, uint16_t& p_iOut )
{
    // check for singularities
    if ( 0.0f == p_vIn[0] && 0.0f == p_vIn[1] )
    {
        if ( p_vIn[2] > 0.0f )
        {
            ((unsigned char*)&p_iOut)[0] = 0;
            ((unsigned char*)&p_iOut)[1] = 0;       // lat = 0, long = 0
        }
        else
        {
            ((unsigned char*)&p_iOut)[0] = 128;
            ((unsigned char*)&p_iOut)[1] = 0;       // lat = 0, long = 128
        }
    }
    else
    {
        int a, b;

        a = int(57.2957795f * ( std::atan2( p_vIn[1], p_vIn[0] ) ) * (255.0f / 360.0f ));
        a &= 0xff;

        b = int(57.2957795f * ( std::acos( p_vIn[2] ) ) * ( 255.0f / 360.0f ));
        b &= 0xff;

        ((unsigned char*)&p_iOut)[0] = (unsigned char) b;   // longitude
        ((unsigned char*)&p_iOut)[1] = (unsigned char) a;   // latitude
    }
}

} // Namespace MD3
} // Namespace Assimp

#endif // !! AI_MD3FILEHELPER_H_INC
