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

/** @file Defines helper data structures for the import of 3DS files */

#ifndef AI_3DSFILEHELPER_H_INC
#define AI_3DSFILEHELPER_H_INC

#include "SpatialSort.h"
#include "SmoothingGroups.h"
#include "StringUtils.h"
#include "qnan.h"
#include <assimp/material.h>
#include <assimp/camera.h>
#include <assimp/light.h>
#include <assimp/anim.h>
#include <stdio.h> //sprintf

namespace Assimp    {
namespace D3DS  {

#include "./../include/assimp/Compiler/pushpack1.h"

// ---------------------------------------------------------------------------
/** Discreet3DS class: Helper class for loading 3ds files. Defines chunks
*  and data structures.
*/
class Discreet3DS {
private:
    Discreet3DS() {
        // empty
    }

    ~Discreet3DS() {
        // empty
    }

public:
    //! data structure for a single chunk in a .3ds file
    struct Chunk {
        uint16_t    Flag;
        uint32_t    Size;
    } PACK_STRUCT;


    //! Used for shading field in material3ds structure
    //! From AutoDesk 3ds SDK
    typedef enum
    {
        // translated to gouraud shading with wireframe active
        Wire = 0x0,

        // if this material is set, no vertex normals will
        // be calculated for the model. Face normals + gouraud
        Flat = 0x1,

        // standard gouraud shading
        Gouraud = 0x2,

        // phong shading
        Phong = 0x3,

        // cooktorrance or anistropic phong shading ...
        // the exact meaning is unknown, if you know it
        // feel free to tell me ;-)
        Metal = 0x4,

        // required by the ASE loader
        Blinn = 0x5
    } shadetype3ds;

    // Flags for animated keys
    enum
    {
        KEY_USE_TENS         = 0x1,
        KEY_USE_CONT         = 0x2,
        KEY_USE_BIAS         = 0x4,
        KEY_USE_EASE_TO      = 0x8,
        KEY_USE_EASE_FROM    = 0x10
    } ;

    enum
    {

        // ********************************************************************
        // Basic chunks which can be found everywhere in the file
        CHUNK_VERSION   = 0x0002,
        CHUNK_RGBF      = 0x0010,       // float4 R; float4 G; float4 B
        CHUNK_RGBB      = 0x0011,       // int1 R; int1 G; int B

        // Linear color values (gamma = 2.2?)
        CHUNK_LINRGBF      = 0x0013,    // float4 R; float4 G; float4 B
        CHUNK_LINRGBB      = 0x0012,    // int1 R; int1 G; int B

        CHUNK_PERCENTW  = 0x0030,       // int2   percentage
        CHUNK_PERCENTF  = 0x0031,       // float4  percentage
        CHUNK_PERCENTD  = 0x0032,       // float8  percentage
        // ********************************************************************

        // Prj master chunk
        CHUNK_PRJ       = 0xC23D,

        // MDLI master chunk
        CHUNK_MLI       = 0x3DAA,

        // Primary main chunk of the .3ds file
        CHUNK_MAIN      = 0x4D4D,

        // Mesh main chunk
        CHUNK_OBJMESH   = 0x3D3D,

        // Specifies the background color of the .3ds file
        // This is passed through the material system for
        // viewing purposes.
        CHUNK_BKGCOLOR  = 0x1200,

        // Specifies the ambient base color of the scene.
        // This is added to all materials in the file
        CHUNK_AMBCOLOR  = 0x2100,

        // Specifies the background image for the whole scene
        // This value is passed through the material system
        // to the viewer
        CHUNK_BIT_MAP   = 0x1100,
        CHUNK_BIT_MAP_EXISTS  = 0x1101,

        // ********************************************************************
        // Viewport related stuff. Ignored
        CHUNK_DEFAULT_VIEW = 0x3000,
        CHUNK_VIEW_TOP = 0x3010,
        CHUNK_VIEW_BOTTOM = 0x3020,
        CHUNK_VIEW_LEFT = 0x3030,
        CHUNK_VIEW_RIGHT = 0x3040,
        CHUNK_VIEW_FRONT = 0x3050,
        CHUNK_VIEW_BACK = 0x3060,
        CHUNK_VIEW_USER = 0x3070,
        CHUNK_VIEW_CAMERA = 0x3080,
        // ********************************************************************

        // Mesh chunks
        CHUNK_OBJBLOCK  = 0x4000,
        CHUNK_TRIMESH   = 0x4100,
        CHUNK_VERTLIST  = 0x4110,
        CHUNK_VERTFLAGS = 0x4111,
        CHUNK_FACELIST  = 0x4120,
        CHUNK_FACEMAT   = 0x4130,
        CHUNK_MAPLIST   = 0x4140,
        CHUNK_SMOOLIST  = 0x4150,
        CHUNK_TRMATRIX  = 0x4160,
        CHUNK_MESHCOLOR = 0x4165,
        CHUNK_TXTINFO   = 0x4170,
        CHUNK_LIGHT     = 0x4600,
        CHUNK_CAMERA    = 0x4700,
        CHUNK_HIERARCHY = 0x4F00,

        // Specifies the global scaling factor. This is applied
        // to the root node's transformation matrix
        CHUNK_MASTER_SCALE    = 0x0100,

        // ********************************************************************
        // Material chunks
        CHUNK_MAT_MATERIAL  = 0xAFFF,

            // asciiz containing the name of the material
            CHUNK_MAT_MATNAME   = 0xA000,
            CHUNK_MAT_AMBIENT   = 0xA010, // followed by color chunk
            CHUNK_MAT_DIFFUSE   = 0xA020, // followed by color chunk
            CHUNK_MAT_SPECULAR  = 0xA030, // followed by color chunk

            // Specifies the shininess of the material
            // followed by percentage chunk
            CHUNK_MAT_SHININESS  = 0xA040,
            CHUNK_MAT_SHININESS_PERCENT  = 0xA041 ,

            // Specifies the shading mode to be used
            // followed by a short
            CHUNK_MAT_SHADING  = 0xA100,

            // NOTE: Emissive color (self illumination) seems not
            // to be a color but a single value, type is unknown.
            // Make the parser accept both of them.
            // followed by percentage chunk (?)
            CHUNK_MAT_SELF_ILLUM = 0xA080,

            // Always followed by percentage chunk  (?)
            CHUNK_MAT_SELF_ILPCT = 0xA084,

            // Always followed by percentage chunk
            CHUNK_MAT_TRANSPARENCY = 0xA050,

            // Diffuse texture channel 0
            CHUNK_MAT_TEXTURE   = 0xA200,

            // Contains opacity information for each texel
            CHUNK_MAT_OPACMAP = 0xA210,

            // Contains a reflection map to be used to reflect
            // the environment. This is partially supported.
            CHUNK_MAT_REFLMAP = 0xA220,

            // Self Illumination map (emissive colors)
            CHUNK_MAT_SELFIMAP = 0xA33d,

            // Bumpmap. Not specified whether it is a heightmap
            // or a normal map. Assme it is a heightmap since
            // artist normally prefer this format.
            CHUNK_MAT_BUMPMAP = 0xA230,

            // Specular map. Seems to influence the specular color
            CHUNK_MAT_SPECMAP = 0xA204,

            // Holds shininess data.
            CHUNK_MAT_MAT_SHINMAP = 0xA33C,

            // Scaling in U/V direction.
            // (need to gen separate UV coordinate set
            // and do this by hand)
            CHUNK_MAT_MAP_USCALE      = 0xA354,
            CHUNK_MAT_MAP_VSCALE      = 0xA356,

            // Translation in U/V direction.
            // (need to gen separate UV coordinate set
            // and do this by hand)
            CHUNK_MAT_MAP_UOFFSET     = 0xA358,
            CHUNK_MAT_MAP_VOFFSET     = 0xA35a,

            // UV-coordinates rotation around the z-axis
            // Assumed to be in radians.
            CHUNK_MAT_MAP_ANG = 0xA35C,

            // Tiling flags for 3DS files
            CHUNK_MAT_MAP_TILING = 0xa351,

            // Specifies the file name of a texture
            CHUNK_MAPFILE   = 0xA300,

            // Specifies whether a materail requires two-sided rendering
            CHUNK_MAT_TWO_SIDE = 0xA081,
        // ********************************************************************

        // Main keyframer chunk. Contains translation/rotation/scaling data
        CHUNK_KEYFRAMER     = 0xB000,

        // Supported sub chunks
        CHUNK_TRACKINFO     = 0xB002,
        CHUNK_TRACKOBJNAME  = 0xB010,
        CHUNK_TRACKDUMMYOBJNAME  = 0xB011,
        CHUNK_TRACKPIVOT    = 0xB013,
        CHUNK_TRACKPOS      = 0xB020,
        CHUNK_TRACKROTATE   = 0xB021,
        CHUNK_TRACKSCALE    = 0xB022,

        // ********************************************************************
        // Keyframes for various other stuff in the file
        // Partially ignored
        CHUNK_AMBIENTKEY    = 0xB001,
        CHUNK_TRACKMORPH    = 0xB026,
        CHUNK_TRACKHIDE     = 0xB029,
        CHUNK_OBJNUMBER     = 0xB030,
        CHUNK_TRACKCAMERA   = 0xB003,
        CHUNK_TRACKFOV      = 0xB023,
        CHUNK_TRACKROLL     = 0xB024,
        CHUNK_TRACKCAMTGT   = 0xB004,
        CHUNK_TRACKLIGHT    = 0xB005,
        CHUNK_TRACKLIGTGT   = 0xB006,
        CHUNK_TRACKSPOTL    = 0xB007,
        CHUNK_FRAMES        = 0xB008,
        // ********************************************************************

        // light sub-chunks
        CHUNK_DL_OFF                 = 0x4620,
        CHUNK_DL_OUTER_RANGE         = 0x465A,
        CHUNK_DL_INNER_RANGE         = 0x4659,
        CHUNK_DL_MULTIPLIER          = 0x465B,
        CHUNK_DL_EXCLUDE             = 0x4654,
        CHUNK_DL_ATTENUATE           = 0x4625,
        CHUNK_DL_SPOTLIGHT           = 0x4610,

        // camera sub-chunks
        CHUNK_CAM_RANGES             = 0x4720
    };
};

// ---------------------------------------------------------------------------
/** Helper structure representing a 3ds mesh face */
struct Face : public FaceWithSmoothingGroup
{
};

// ---------------------------------------------------------------------------
/** Helper structure representing a texture */
struct Texture
{
    //! Default constructor
    Texture()
        : mOffsetU  (0.0)
        , mOffsetV  (0.0)
        , mScaleU   (1.0)
        , mScaleV   (1.0)
        , mRotation (0.0)
        , mMapMode  (aiTextureMapMode_Wrap)
        , bPrivate()
        , iUVSrc    (0)
    {
        mTextureBlend = get_qnan();
    }

    //! Specifies the blend factor for the texture
    ai_real mTextureBlend;

    //! Specifies the filename of the texture
    std::string mMapName;

    //! Specifies texture coordinate offsets/scaling/rotations
    ai_real mOffsetU;
    ai_real mOffsetV;
    ai_real mScaleU;
    ai_real mScaleV;
    ai_real mRotation;

    //! Specifies the mapping mode to be used for the texture
    aiTextureMapMode mMapMode;

    //! Used internally
    bool bPrivate;
    int iUVSrc;
};

#include "./../include/assimp/Compiler/poppack1.h"

// ---------------------------------------------------------------------------
/** Helper structure representing a 3ds material */
struct Material
{
    //! Default constructor. Builds a default name for the material
    Material()
    : mDiffuse            ( ai_real( 0.6 ), ai_real( 0.6 ), ai_real( 0.6 ) ) // FIX ... we won't want object to be black
    , mSpecularExponent   ( ai_real( 0.0 ) )
    , mShininessStrength  ( ai_real( 1.0 ) )
    , mShading(Discreet3DS::Gouraud)
    , mTransparency       ( ai_real( 1.0 ) )
    , mBumpHeight         ( ai_real( 1.0 ) )
    , mTwoSided           (false)
    {
        static int iCnt = 0;

        char szTemp[128];
        ai_snprintf(szTemp, 128, "UNNAMED_%i",iCnt++);
        mName = szTemp;
    }

    //! Name of the material
    std::string mName;
    //! Diffuse color of the material
    aiColor3D mDiffuse;
    //! Specular exponent
    ai_real mSpecularExponent;
    //! Shininess strength, in percent
    ai_real mShininessStrength;
    //! Specular color of the material
    aiColor3D mSpecular;
    //! Ambient color of the material
    aiColor3D mAmbient;
    //! Shading type to be used
    Discreet3DS::shadetype3ds mShading;
    //! Opacity of the material
    ai_real mTransparency;
    //! Diffuse texture channel
    Texture sTexDiffuse;
    //! Opacity texture channel
    Texture sTexOpacity;
    //! Specular texture channel
    Texture sTexSpecular;
    //! Reflective texture channel
    Texture sTexReflective;
    //! Bump texture channel
    Texture sTexBump;
    //! Emissive texture channel
    Texture sTexEmissive;
    //! Shininess texture channel
    Texture sTexShininess;
    //! Scaling factor for the bump values
    ai_real mBumpHeight;
    //! Emissive color
    aiColor3D mEmissive;
    //! Ambient texture channel
    //! (used by the ASE format)
    Texture sTexAmbient;
    //! True if the material must be rendered from two sides
    bool mTwoSided;
};

// ---------------------------------------------------------------------------
/** Helper structure to represent a 3ds file mesh */
struct Mesh : public MeshWithSmoothingGroups<D3DS::Face>
{
    //! Default constructor
    Mesh()
    {
        static int iCnt = 0;

        // Generate a default name for the mesh
        char szTemp[128];
        ai_snprintf(szTemp, 128, "UNNAMED_%i",iCnt++);
        mName = szTemp;
    }

    //! Name of the mesh
    std::string mName;

    //! Texture coordinates
    std::vector<aiVector3D> mTexCoords;

    //! Face materials
    std::vector<unsigned int> mFaceMaterials;

    //! Local transformation matrix
    aiMatrix4x4 mMat;
};

// ---------------------------------------------------------------------------
/** Float key - quite similar to aiVectorKey and aiQuatKey. Both are in the
    C-API, so it would be difficult to make them a template. */
struct aiFloatKey
{
    double mTime;      ///< The time of this key
    ai_real mValue;   ///< The value of this key

#ifdef __cplusplus

    // time is not compared
    bool operator == (const aiFloatKey& o) const
        {return o.mValue == this->mValue;}

    bool operator != (const aiFloatKey& o) const
        {return o.mValue != this->mValue;}

    // Only time is compared. This operator is defined
    // for use with std::sort
    bool operator < (const aiFloatKey& o) const
        {return mTime < o.mTime;}

    bool operator > (const aiFloatKey& o) const
        {return mTime > o.mTime;}

#endif
};

// ---------------------------------------------------------------------------
/** Helper structure to represent a 3ds file node */
struct Node
{
    Node():
    	mParent(NULL)
		,	mInstanceNumber(0)
		,	mHierarchyPos		(0)
		,	mHierarchyIndex		(0)
		,	mInstanceCount		(1)
    {
        static int iCnt = 0;

        // Generate a default name for the node
        char szTemp[128];
        ::ai_snprintf(szTemp, 128, "UNNAMED_%i",iCnt++);
        mName = szTemp;

        aRotationKeys.reserve (20);
        aPositionKeys.reserve (20);
        aScalingKeys.reserve  (20);
    }

    ~Node()
    {
        for (unsigned int i = 0; i < mChildren.size();++i)
            delete mChildren[i];
    }

    //! Pointer to the parent node
    Node* mParent;

    //! Holds all child nodes
    std::vector<Node*> mChildren;

    //! Name of the node
    std::string mName;

    //! InstanceNumber of the node
    int32_t mInstanceNumber;

    //! Dummy nodes: real name to be combined with the $$$DUMMY
    std::string mDummyName;

    //! Position of the node in the hierarchy (tree depth)
    int16_t mHierarchyPos;

    //! Index of the node
    int16_t mHierarchyIndex;

    //! Rotation keys loaded from the file
    std::vector<aiQuatKey> aRotationKeys;

    //! Position keys loaded from the file
    std::vector<aiVectorKey> aPositionKeys;

    //! Scaling keys loaded from the file
    std::vector<aiVectorKey> aScalingKeys;


    // For target lights (spot lights and directional lights):
    // The position of the target
    std::vector< aiVectorKey > aTargetPositionKeys;

    // For cameras: the camera roll angle
    std::vector< aiFloatKey > aCameraRollKeys;

    //! Pivot position loaded from the file
    aiVector3D vPivot;

    //instance count, will be kept only for the first node
    int32_t mInstanceCount;

    //! Add a child node, setup the right parent node for it
    //! \param pc Node to be 'adopted'
    inline Node& push_back(Node* pc)
    {
        mChildren.push_back(pc);
        pc->mParent = this;
        return *this;
    }
};
// ---------------------------------------------------------------------------
/** Helper structure analogue to aiScene */
struct Scene
{
    //! List of all materials loaded
    //! NOTE: 3ds references materials globally
    std::vector<Material> mMaterials;

    //! List of all meshes loaded
    std::vector<Mesh> mMeshes;

    //! List of all cameras loaded
    std::vector<aiCamera*> mCameras;

    //! List of all lights loaded
    std::vector<aiLight*> mLights;

    //! Pointer to the root node of the scene
    // --- moved to main class
    // Node* pcRootNode;
};


} // end of namespace D3DS
} // end of namespace Assimp

#endif // AI_XFILEHELPER_H_INC
