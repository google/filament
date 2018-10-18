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


/**
 * @file  MDLFileData.h
 * @brief Definition of in-memory structures for the MDL file format.
 *
 * The specification has been taken from various sources on the internet.
 * - http://tfc.duke.free.fr/coding/mdl-specs-en.html
 * - Conitec's MED SDK
 * - Many quite long HEX-editor sessions
 */

#ifndef AI_MDLFILEHELPER_H_INC
#define AI_MDLFILEHELPER_H_INC

#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/Compiler/pushpack1.h>
#include <assimp/ByteSwapper.h>
#include <stdint.h>
#include <vector>

struct aiMaterial;

namespace Assimp    {
namespace MDL   {

// -------------------------------------------------------------------------------------
// to make it easier for us, we test the magic word against both "endianesses"

// magic bytes used in Quake 1 MDL meshes
#define AI_MDL_MAGIC_NUMBER_BE  AI_MAKE_MAGIC("IDPO")
#define AI_MDL_MAGIC_NUMBER_LE  AI_MAKE_MAGIC("OPDI")

// magic bytes used in GameStudio A<very  low> MDL meshes
#define AI_MDL_MAGIC_NUMBER_BE_GS3  AI_MAKE_MAGIC("MDL2")
#define AI_MDL_MAGIC_NUMBER_LE_GS3  AI_MAKE_MAGIC("2LDM")

// magic bytes used in GameStudio A4 MDL meshes
#define AI_MDL_MAGIC_NUMBER_BE_GS4  AI_MAKE_MAGIC("MDL3")
#define AI_MDL_MAGIC_NUMBER_LE_GS4  AI_MAKE_MAGIC("3LDM")

// magic bytes used in GameStudio A5+ MDL meshes
#define AI_MDL_MAGIC_NUMBER_BE_GS5a AI_MAKE_MAGIC("MDL4")
#define AI_MDL_MAGIC_NUMBER_LE_GS5a AI_MAKE_MAGIC("4LDM")
#define AI_MDL_MAGIC_NUMBER_BE_GS5b AI_MAKE_MAGIC("MDL5")
#define AI_MDL_MAGIC_NUMBER_LE_GS5b AI_MAKE_MAGIC("5LDM")

// magic bytes used in GameStudio A7+ MDL meshes
#define AI_MDL_MAGIC_NUMBER_BE_GS7  AI_MAKE_MAGIC("MDL7")
#define AI_MDL_MAGIC_NUMBER_LE_GS7  AI_MAKE_MAGIC("7LDM")

// common limitations for Quake1 meshes. The loader does not check them,
// (however it warns) but models should not exceed these limits.
#if (!defined AI_MDL_VERSION)
#   define AI_MDL_VERSION               6
#endif
#if (!defined AI_MDL_MAX_FRAMES)
#   define AI_MDL_MAX_FRAMES            256
#endif
#if (!defined AI_MDL_MAX_UVS)
#   define AI_MDL_MAX_UVS               1024
#endif
#if (!defined AI_MDL_MAX_VERTS)
#   define AI_MDL_MAX_VERTS             1024
#endif
#if (!defined AI_MDL_MAX_TRIANGLES)
#   define AI_MDL_MAX_TRIANGLES         2048
#endif

// material key that is set for dummy materials that are
// just referencing another material
#if (!defined AI_MDL7_REFERRER_MATERIAL)
#   define AI_MDL7_REFERRER_MATERIAL "&&&referrer&&&",0,0
#endif

// -------------------------------------------------------------------------------------
/** \struct Header
 *  \brief Data structure for the MDL main header
 */
struct Header {
    //! magic number: "IDPO"
    uint32_t ident;

    //! version number: 6
    int32_t version;

    //! scale factors for each axis
    ai_real scale[3];

    //! translation factors for each axis
    ai_real translate[3];

    //! bounding radius of the mesh
    float boundingradius;

    //! Position of the viewer's exe. Ignored
    ai_real vEyePos[3];

    //! Number of textures
    int32_t num_skins;

    //! Texture width in pixels
    int32_t skinwidth;

    //! Texture height in pixels
    int32_t skinheight;

    //! Number of vertices contained in the file
    int32_t num_verts;

    //! Number of triangles contained in the file
    int32_t num_tris;

    //! Number of frames contained in the file
    int32_t num_frames;

    //! 0 = synchron, 1 = random . Ignored
    //! (MDLn formats: number of texture coordinates)
    int32_t synctype;

    //! State flag
    int32_t flags;

    //! Could be the total size of the file (and not a float)
    float size;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
/** \struct Header_MDL7
 *  \brief Data structure for the MDL 7 main header
 */
struct Header_MDL7 {
    //! magic number: "MDL7"
    char    ident[4];

    //! Version number. Ignored
    int32_t version;

    //! Number of bones in file
    uint32_t    bones_num;

    //! Number of groups in file
    uint32_t    groups_num;

    //! Size of data in the file
    uint32_t    data_size;

    //! Ignored. Used to store entity specific information
    int32_t entlump_size;

    //! Ignored. Used to store MED related data
    int32_t medlump_size;

    //! Size of the Bone_MDL7 data structure used in the file
    uint16_t bone_stc_size;

    //! Size of the Skin_MDL 7 data structure used in the file
    uint16_t skin_stc_size;

    //! Size of a single color (e.g. in a material)
    uint16_t colorvalue_stc_size;

    //! Size of the Material_MDL7 data structure used in the file
    uint16_t material_stc_size;

    //! Size of a texture coordinate set in the file
    uint16_t skinpoint_stc_size;

    //! Size of a triangle in the file
    uint16_t triangle_stc_size;

    //! Size of a normal vertex in the file
    uint16_t mainvertex_stc_size;

    //! Size of a per-frame animated vertex in the file
    //! (this is not supported)
    uint16_t framevertex_stc_size;

    //! Size of a bone animation matrix
    uint16_t bonetrans_stc_size;

    //! Size of the Frame_MDL7 data structure used in the file
    uint16_t frame_stc_size;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
/** \struct Bone_MDL7
 *  \brief Data structure for a bone in a MDL7 file
 */
struct Bone_MDL7 {
    //! Index of the parent bone of *this* bone. 0xffff means:
    //! "hey, I have no parent, I'm an orphan"
    uint16_t parent_index;
    uint8_t _unused_[2];

    //! Relative position of the bone (relative to the
    //! parent bone)
    float x,y,z;

    //! Optional name of the bone
    char name[1 /* DUMMY SIZE */];
} PACK_STRUCT;

#if (!defined AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_20_CHARS)
#   define AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_20_CHARS (16 + 20)
#endif

#if (!defined AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_32_CHARS)
#   define AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_32_CHARS (16 + 32)
#endif

#if (!defined AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_NOT_THERE)
#   define AI_MDL7_BONE_STRUCT_SIZE__NAME_IS_NOT_THERE (16)
#endif

#if (!defined AI_MDL7_MAX_GROUPNAMESIZE)
#   define AI_MDL7_MAX_GROUPNAMESIZE    16
#endif // ! AI_MDL7_MAX_GROUPNAMESIZE

// -------------------------------------------------------------------------------------
/** \struct Group_MDL7
 *  \brief Group in a MDL7 file
 */
struct Group_MDL7 {
    //! = '1' -> triangle based Mesh
    unsigned char   typ;

    int8_t  deformers;
    int8_t  max_weights;
    int8_t  _unused_;

    //! size of data for this group in bytes ( MD7_GROUP stc. included).
    int32_t groupdata_size;
    char    name[AI_MDL7_MAX_GROUPNAMESIZE];

    //! Number of skins
    int32_t numskins;

    //! Number of texture coordinates
    int32_t num_stpts;

    //! Number of triangles
    int32_t numtris;

    //! Number of vertices
    int32_t numverts;

    //! Number of frames
    int32_t numframes;
} PACK_STRUCT;

#define AI_MDL7_SKINTYPE_MIPFLAG                0x08
#define AI_MDL7_SKINTYPE_MATERIAL               0x10
#define AI_MDL7_SKINTYPE_MATERIAL_ASCDEF        0x20
#define AI_MDL7_SKINTYPE_RGBFLAG                0x80

#if (!defined AI_MDL7_MAX_BONENAMESIZE)
#   define AI_MDL7_MAX_BONENAMESIZE 20
#endif // !! AI_MDL7_MAX_BONENAMESIZE

// -------------------------------------------------------------------------------------
/** \struct Deformer_MDL7
 *  \brief Deformer in a MDL7 file
 */
struct Deformer_MDL7 {
    int8_t  deformer_version;       // 0
    int8_t  deformer_typ;           // 0 - bones
    int8_t  _unused_[2];
    int32_t group_index;
    int32_t elements;
    int32_t deformerdata_size;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
/** \struct DeformerElement_MDL7
 *  \brief Deformer element in a MDL7 file
 */
struct DeformerElement_MDL7 {
    //! bei deformer_typ==0 (==bones) element_index == bone index
    int32_t element_index;
    char    element_name[AI_MDL7_MAX_BONENAMESIZE];
    int32_t weights;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct DeformerWeight_MDL7
 *  \brief Deformer weight in a MDL7 file
 */
struct DeformerWeight_MDL7 {
    //! for deformer_typ==0 (==bones) index == vertex index
    int32_t index;
    float   weight;
} PACK_STRUCT;

// don't know why this was in the original headers ...
typedef int32_t MD7_MATERIAL_ASCDEFSIZE;

// -------------------------------------------------------------------------------------
/** \struct ColorValue_MDL7
 *  \brief Data structure for a color value in a MDL7 file
 */
struct ColorValue_MDL7 {
    float r,g,b,a;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Material_MDL7
 *  \brief Data structure for a Material in a MDL7 file
 */
struct Material_MDL7 {
    //! Diffuse base color of the material
    ColorValue_MDL7 Diffuse;

    //! Ambient base color of the material
    ColorValue_MDL7 Ambient;

    //! Specular base color of the material
    ColorValue_MDL7 Specular;

    //! Emissive base color of the material
    ColorValue_MDL7 Emissive;

    //! Phong power
    float           Power;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Skin
 *  \brief Skin data structure #1 - used by Quake1, MDL2, MDL3 and MDL4
 */
struct Skin {
    //! 0 = single (Skin), 1 = group (GroupSkin)
    //! For MDL3-5: Defines the type of the skin and there
    //! fore the size of the data to skip:
    //-------------------------------------------------------
    //! 2 for 565 RGB,
    //! 3 for 4444 ARGB,
    //! 10 for 565 mipmapped,
    //! 11 for 4444 mipmapped (bpp = 2),
    //! 12 for 888 RGB mipmapped (bpp = 3),
    //! 13 for 8888 ARGB mipmapped (bpp = 4)
    //-------------------------------------------------------
    int32_t group;

    //! Texture data
    uint8_t *data;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
/** \struct Skin
 *  \brief Skin data structure #2 - used by MDL5, MDL6 and MDL7
 *  \see Skin
 */
struct Skin_MDL5 {
    int32_t size, width, height;
    uint8_t *data;
} PACK_STRUCT;

// maximum length of texture file name
#if (!defined AI_MDL7_MAX_TEXNAMESIZE)
#   define AI_MDL7_MAX_TEXNAMESIZE      0x10
#endif

// ---------------------------------------------------------------------------
/** \struct Skin_MDL7
 *  \brief Skin data structure #3 - used by MDL7 and HMP7
 */
struct Skin_MDL7 {
    uint8_t         typ;
    int8_t          _unused_[3];
    int32_t         width;
    int32_t         height;
    char            texture_name[AI_MDL7_MAX_TEXNAMESIZE];
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct RGB565
 *  \brief Data structure for a RGB565 pixel in a texture
 */
struct RGB565 {
    uint16_t r : 5;
    uint16_t g : 6;
    uint16_t b : 5;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct ARGB4
 *  \brief Data structure for a ARGB4444 pixel in a texture
 */
struct ARGB4 {
    uint16_t a : 4;
    uint16_t r : 4;
    uint16_t g : 4;
    uint16_t b : 4;
} /*PACK_STRUCT*/;

// -------------------------------------------------------------------------------------
/** \struct GroupSkin
 *  \brief Skin data structure #2 (group of pictures)
 */
struct GroupSkin {
    //! 0 = single (Skin), 1 = group (GroupSkin)
    int32_t group;

    //! Number of images
    int32_t nb;

    //! Time for each image
    float *time;

    //! Data of each image
    uint8_t **data;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct TexCoord
 *  \brief Texture coordinate data structure used by the Quake1 MDL format
 */
struct TexCoord {
    //! Is the vertex on the noundary between front and back piece?
    int32_t onseam;

    //! Texture coordinate in the tx direction
    int32_t s;

    //! Texture coordinate in the ty direction
    int32_t t;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct TexCoord_MDL3
 *  \brief Data structure for an UV coordinate in the 3DGS MDL3 format
 */
struct TexCoord_MDL3 {
    //! position, horizontally in range 0..skinwidth-1
    int16_t u;

    //! position, vertically in range 0..skinheight-1
    int16_t v;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct TexCoord_MDL7
 *  \brief Data structure for an UV coordinate in the 3DGS MDL7 format
 */
struct TexCoord_MDL7 {
    //! position, horizontally in range 0..1
    float u;

    //! position, vertically in range 0..1
    float v;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct SkinSet_MDL7
 *  \brief Skin set data structure for the 3DGS MDL7 format
 * MDL7 references UV coordinates per face via an index list.
 * This allows the use of multiple skins per face with just one
 * UV coordinate set.
 */
struct SkinSet_MDL7
{
    //! Index into the UV coordinate list
    uint16_t    st_index[3]; // size 6

    //! Material index
    int32_t     material;    // size 4
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Triangle
 *  \brief Triangle data structure for the Quake1 MDL format
 */
struct Triangle
{
    //! 0 = backface, 1 = frontface
    int32_t facesfront;

    //! Vertex indices
    int32_t vertex[3];
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Triangle_MDL3
 *  \brief Triangle data structure for the 3DGS MDL3 format
 */
struct Triangle_MDL3
{
    //!  Index of 3 3D vertices in range 0..numverts
    uint16_t index_xyz[3];

    //! Index of 3 skin vertices in range 0..numskinverts
    uint16_t index_uv[3];
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Triangle_MDL7
 *  \brief Triangle data structure for the 3DGS MDL7 format
 */
struct Triangle_MDL7
{
    //! Vertex indices
    uint16_t   v_index[3];  // size 6

    //! Two skinsets. The second will be used for multi-texturing
    SkinSet_MDL7  skinsets[2];
} PACK_STRUCT;

#if (!defined AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV)
#   define AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV (6+sizeof(SkinSet_MDL7)-sizeof(uint32_t))
#endif
#if (!defined AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV_WITH_MATINDEX)
#   define AI_MDL7_TRIANGLE_STD_SIZE_ONE_UV_WITH_MATINDEX (6+sizeof(SkinSet_MDL7))
#endif
#if (!defined AI_MDL7_TRIANGLE_STD_SIZE_TWO_UV)
#   define AI_MDL7_TRIANGLE_STD_SIZE_TWO_UV (6+2*sizeof(SkinSet_MDL7))
#endif

// Helper constants for Triangle::facesfront
#if (!defined AI_MDL_BACKFACE)
#   define AI_MDL_BACKFACE          0x0
#endif
#if (!defined  AI_MDL_FRONTFACE)
#   define AI_MDL_FRONTFACE         0x1
#endif

// -------------------------------------------------------------------------------------
/** \struct Vertex
 *  \brief Vertex data structure
 */
struct Vertex
{
    uint8_t v[3];
    uint8_t normalIndex;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
struct Vertex_MDL4
{
    uint16_t v[3];
    uint8_t normalIndex;
    uint8_t unused;
} PACK_STRUCT;

#define AI_MDL7_FRAMEVERTEX120503_STCSIZE       16
#define AI_MDL7_FRAMEVERTEX030305_STCSIZE       26

// -------------------------------------------------------------------------------------
/** \struct Vertex_MDL7
 *  \brief Vertex data structure used in MDL7 files
 */
struct Vertex_MDL7
{
    float   x,y,z;
    uint16_t vertindex; // = bone index
    union {
        uint8_t norm162index;
        float norm[3];
    };
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct BoneTransform_MDL7
 *  \brief bone transformation matrix structure used in MDL7 files
 */
struct BoneTransform_MDL7
{
    //! 4*3
    float   m [4*4];

    //! the index of this vertex, 0.. header::bones_num - 1
    uint16_t bone_index;

    //! I HATE 3DGS AND THE SILLY DEVELOPER WHO DESIGNED
    //! THIS STUPID FILE FORMAT!
    int8_t _unused_[2];
} PACK_STRUCT;


#define AI_MDL7_MAX_FRAMENAMESIZE       16

// -------------------------------------------------------------------------------------
/** \struct Frame_MDL7
 *  \brief Frame data structure used by MDL7 files
 */
struct Frame_MDL7
{
    char    frame_name[AI_MDL7_MAX_FRAMENAMESIZE];
    uint32_t    vertices_count;
    uint32_t    transmatrix_count;
};


// -------------------------------------------------------------------------------------
/** \struct SimpleFrame
 *  \brief Data structure for a simple frame
 */
struct SimpleFrame
{
    //! Minimum vertex of the bounding box
    Vertex bboxmin;

    //! Maximum vertex of the bounding box
    Vertex bboxmax;

    //! Name of the frame
    char name[16];

    //! Vertex list of the frame
    Vertex *verts;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct Frame
 *  \brief Model frame data structure
 */
struct Frame
{
    //! 0 = simple frame, !0 = group frame
    int32_t type;

    //! Frame data
    SimpleFrame frame;
} PACK_STRUCT;


// -------------------------------------------------------------------------------------
struct SimpleFrame_MDLn_SP
{
    //! Minimum vertex of the bounding box
    Vertex_MDL4 bboxmin;

    //! Maximum vertex of the bounding box
    Vertex_MDL4 bboxmax;

    //! Name of the frame
    char name[16];

    //! Vertex list of the frame
    Vertex_MDL4 *verts;
} PACK_STRUCT;

// -------------------------------------------------------------------------------------
/** \struct GroupFrame
 *  \brief Data structure for a group of frames
 */
struct GroupFrame
{
    //! 0 = simple frame, !0 = group frame
    int32_t type;

    //! Minimum vertex for all single frames
    Vertex min;

    //! Maximum vertex for all single frames
    Vertex max;

    //! Time for all single frames
    float *time;

    //! List of single frames
    SimpleFrame *frames;
} PACK_STRUCT;

#include <assimp/Compiler/poppack1.h>

// -------------------------------------------------------------------------------------
/** \struct IntFace_MDL7
 *  \brief Internal data structure to temporarily represent a face
 */
struct IntFace_MDL7 {
    // provide a constructor for our own convenience
    IntFace_MDL7() AI_NO_EXCEPT {
        ::memset( mIndices, 0, sizeof(uint32_t) *3);
        ::memset( iMatIndex, 0, sizeof( unsigned int) *2);
    }

    //! Vertex indices
    uint32_t mIndices[3];

    //! Material index (maximally two channels, which are joined later)
    unsigned int iMatIndex[2];
};

// -------------------------------------------------------------------------------------
/** \struct IntMaterial_MDL7
 *  \brief Internal data structure to temporarily represent a material
 *  which has been created from two single materials along with the
 *  original material indices.
 */
struct IntMaterial_MDL7 {
    // provide a constructor for our own convenience
    IntMaterial_MDL7() AI_NO_EXCEPT
    : pcMat( nullptr ) {
        ::memset( iOldMatIndices, 0, sizeof(unsigned int) *2);
    }

    //! Material instance
    aiMaterial* pcMat;

    //! Old material indices
    unsigned int iOldMatIndices[2];
};

// -------------------------------------------------------------------------------------
/** \struct IntBone_MDL7
 *  \brief Internal data structure to represent a bone in a MDL7 file with
 *  all of its animation channels assigned to it.
 */
struct IntBone_MDL7 : aiBone
{
    //! Default constructor
    IntBone_MDL7() AI_NO_EXCEPT : iParent (0xffff)
    {
        pkeyPositions.reserve(30);
        pkeyScalings.reserve(30);
        pkeyRotations.reserve(30);
    }

    //! Parent bone of the bone
    uint64_t iParent;

    //! Relative position of the bone
    aiVector3D vPosition;

    //! Array of position keys
    std::vector<aiVectorKey> pkeyPositions;

    //! Array of scaling keys
    std::vector<aiVectorKey> pkeyScalings;

    //! Array of rotation keys
    std::vector<aiQuatKey> pkeyRotations;
};

// -------------------------------------------------------------------------------------
//! Describes a MDL7 frame
struct IntFrameInfo_MDL7
{
    //! Construction from an existing frame header
    IntFrameInfo_MDL7(BE_NCONST MDL::Frame_MDL7* _pcFrame,unsigned int _iIndex)
        : iIndex(_iIndex)
        , pcFrame(_pcFrame)
    {}

    //! Index of the frame
    unsigned int iIndex;

    //! Points to the header of the frame
    BE_NCONST MDL::Frame_MDL7*  pcFrame;
};

// -------------------------------------------------------------------------------------
//! Describes a MDL7 mesh group
struct IntGroupInfo_MDL7
{
    //! Default constructor
    IntGroupInfo_MDL7() AI_NO_EXCEPT
        :   iIndex(0)
        ,   pcGroup(nullptr)
        ,   pcGroupUVs(nullptr)
        ,   pcGroupTris(nullptr)
        ,   pcGroupVerts(nullptr)
        {}

    //! Construction from an existing group header
    IntGroupInfo_MDL7(BE_NCONST MDL::Group_MDL7* _pcGroup, unsigned int _iIndex)
        :   iIndex(_iIndex)
        ,   pcGroup(_pcGroup)
        ,   pcGroupUVs()
        ,   pcGroupTris()
        ,   pcGroupVerts()
    {}

    //! Index of the group
    unsigned int iIndex;

    //! Points to the header of the group
    BE_NCONST MDL::Group_MDL7*      pcGroup;

    //! Points to the beginning of the uv coordinate section
    BE_NCONST MDL::TexCoord_MDL7*   pcGroupUVs;

    //! Points to the beginning of the triangle section
    MDL::Triangle_MDL7* pcGroupTris;

    //! Points to the beginning of the vertex section
    BE_NCONST MDL::Vertex_MDL7*     pcGroupVerts;
};

// -------------------------------------------------------------------------------------
//! Holds the data that belongs to a MDL7 mesh group
struct IntGroupData_MDL7
{
    IntGroupData_MDL7() AI_NO_EXCEPT
        : bNeed2UV(false)
    {}

    //! Array of faces that belong to the group
    std::vector<MDL::IntFace_MDL7> pcFaces;

    //! Array of vertex positions
    std::vector<aiVector3D>     vPositions;

    //! Array of vertex normals
    std::vector<aiVector3D>     vNormals;

    //! Array of bones indices
    std::vector<unsigned int>   aiBones;

    //! First UV coordinate set
    std::vector<aiVector3D>     vTextureCoords1;

    //! Optional second UV coordinate set
    std::vector<aiVector3D>     vTextureCoords2;

    //! Specifies whether there are two texture
    //! coordinate sets required
    bool bNeed2UV;
};

// -------------------------------------------------------------------------------------
//! Holds data from an MDL7 file that is shared by all mesh groups
struct IntSharedData_MDL7 {
    //! Default constructor
    IntSharedData_MDL7() AI_NO_EXCEPT
        : apcOutBones(),
        iNum()
    {
        abNeedMaterials.reserve(10);
    }

    //! Destruction: properly delete all allocated resources
    ~IntSharedData_MDL7()
    {
        // kill all bones
        if (this->apcOutBones)
        {
            for (unsigned int m = 0; m < iNum;++m)
                delete this->apcOutBones[m];
            delete[] this->apcOutBones;
        }
    }

    //! Specifies which materials are used
    std::vector<bool> abNeedMaterials;

    //! List of all materials
    std::vector<aiMaterial*> pcMats;

    //! List of all bones
    IntBone_MDL7** apcOutBones;

    //! number of bones
    unsigned int iNum;
};

// -------------------------------------------------------------------------------------
//! Contains input data for GenerateOutputMeshes_3DGS_MDL7
struct IntSplitGroupData_MDL7
{
    //! Construction from a given shared data set
    IntSplitGroupData_MDL7(IntSharedData_MDL7& _shared,
        std::vector<aiMesh*>& _avOutList)

        : aiSplit(), shared(_shared), avOutList(_avOutList)
    {
    }

    //! Destruction: properly delete all allocated resources
    ~IntSplitGroupData_MDL7()
    {
        // kill all face lists
        if(this->aiSplit)
        {
            for (unsigned int m = 0; m < shared.pcMats.size();++m)
                delete this->aiSplit[m];
            delete[] this->aiSplit;
        }
    }

    //! Contains a list of all faces per material
    std::vector<unsigned int>** aiSplit;

    //! Shared data for all groups of the model
    IntSharedData_MDL7& shared;

    //! List of meshes
    std::vector<aiMesh*>& avOutList;
};


}
} // end namespaces

#endif // !! AI_MDLFILEHELPER_H_INC
