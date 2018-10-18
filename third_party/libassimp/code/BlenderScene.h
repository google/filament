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

/** @file  BlenderScene.h
 *  @brief Intermediate representation of a BLEND scene.
 */
#ifndef INCLUDED_AI_BLEND_SCENE_H
#define INCLUDED_AI_BLEND_SCENE_H

#include "BlenderDNA.h"

namespace Assimp    {
namespace Blender {

// Minor parts of this file are extracts from blender data structures,
// declared in the ./source/blender/makesdna directory.
// Stuff that is not used by Assimp is commented.


// NOTE
// this file serves as input data to the `./scripts/genblenddna.py`
// script. This script generates the actual binding code to read a
// blender file with a possibly different DNA into our structures.
// Only `struct` declarations are considered and the following
// rules must be obeyed in order for the script to work properly:
//
// * C++ style comments only
//
// * Structures may include the primitive types char, int, short,
//   float, double. Signed specifiers are not allowed on
//   integers. Enum types are allowed, but they must have been
//   defined in this header.
//
// * Structures may aggregate other structures, unless not defined
//   in this header.
//
// * Pointers to other structures or primitive types are allowed.
//   No references or double pointers or arrays of pointers.
//   A pointer to a T is normally written as std::shared_ptr, while a
//   pointer to an array of elements is written as boost::
//   shared_array. To avoid cyclic pointers, use raw pointers in
//   one direction.
//
// * Arrays can have maximally two-dimensions. Any non-pointer
//   type can form them.
//
// * Multiple fields can be declare in a single line (i.e `int a,b;`)
//   provided they are neither pointers nor arrays.
//
// * One of WARN, FAIL can be appended to the declaration (
//   prior to the semicolon to specify the error handling policy if
//   this field is missing in the input DNA). If none of those
//   is specified the default policy is to substitute a default
//   value for the field.
//

// warn if field is missing, substitute default value
#ifdef WARN
#  undef WARN
#endif
#define WARN 

// fail the import if the field does not exist
#ifdef FAIL
#  undef FAIL
#endif
#define FAIL 

struct Object;
struct MTex;
struct Image;

#include <memory>

#define AI_BLEND_MESH_MAX_VERTS 2000000000L

static const size_t MaxNameLen = 1024;

// -------------------------------------------------------------------------------
struct ID : ElemBase {
    char name[ MaxNameLen ] WARN;
    short flag;
};

// -------------------------------------------------------------------------------
struct ListBase : ElemBase {
    std::shared_ptr<ElemBase> first;
    std::shared_ptr<ElemBase> last;
};


// -------------------------------------------------------------------------------
struct PackedFile : ElemBase {
     int size WARN;
     int seek WARN;
     std::shared_ptr< FileOffset > data WARN;
};

// -------------------------------------------------------------------------------
struct GroupObject : ElemBase {
    std::shared_ptr<GroupObject> prev,next FAIL;
    std::shared_ptr<Object> ob;
};

// -------------------------------------------------------------------------------
struct Group : ElemBase {
    ID id FAIL;
    int layer;

    std::shared_ptr<GroupObject> gobject;
};

// -------------------------------------------------------------------------------
struct World : ElemBase {
    ID id FAIL;
};

// -------------------------------------------------------------------------------
struct MVert : ElemBase {
    float co[3] FAIL;
    float no[3] FAIL;       // readed as short and divided through / 32767.f
    char flag;
    int mat_nr WARN;
    int bweight;

    MVert() : ElemBase()
        , flag(0)
        , mat_nr(0)
        , bweight(0)
    {}
};

// -------------------------------------------------------------------------------
struct MEdge : ElemBase {
      int v1, v2 FAIL;
      char crease, bweight;
      short flag;
};

// -------------------------------------------------------------------------------
struct MLoop : ElemBase {
    int v, e;
};

// -------------------------------------------------------------------------------
struct MLoopUV : ElemBase {
    float uv[2];
    int flag;
};

// -------------------------------------------------------------------------------
// Note that red and blue are not swapped, as with MCol
struct MLoopCol : ElemBase {
	unsigned char r, g, b, a;
};

// -------------------------------------------------------------------------------
struct MPoly : ElemBase {
    int loopstart;
    int totloop;
    short mat_nr;
    char flag;
};

// -------------------------------------------------------------------------------
struct MTexPoly : ElemBase {
    Image* tpage;
    char flag, transp;
    short mode, tile, pad;
};

// -------------------------------------------------------------------------------
struct MCol : ElemBase {
    char r,g,b,a FAIL;
};

// -------------------------------------------------------------------------------
struct MFace : ElemBase {
    int v1,v2,v3,v4 FAIL;
    int mat_nr FAIL;
    char flag;
};

// -------------------------------------------------------------------------------
struct TFace : ElemBase {
    float uv[4][2] FAIL;
    int col[4] FAIL;
    char flag;
    short mode;
    short tile;
    short unwrap;
};

// -------------------------------------------------------------------------------
struct MTFace : ElemBase {
	MTFace()
	: flag(0)
	, mode(0)
	, tile(0)
	, unwrap(0)
	{
	}

    float uv[4][2] FAIL;
    char flag;
    short mode;
    short tile;
    short unwrap;

    // std::shared_ptr<Image> tpage;
};

// -------------------------------------------------------------------------------
struct MDeformWeight : ElemBase  {
      int    def_nr FAIL;
      float  weight FAIL;
};

// -------------------------------------------------------------------------------
struct MDeformVert : ElemBase  {
    vector<MDeformWeight> dw WARN;
    int totweight;
};

// -------------------------------------------------------------------------------
#define MA_RAYMIRROR    0x40000
#define MA_TRANSPARENCY 0x10000
#define MA_RAYTRANSP    0x20000
#define MA_ZTRANSP      0x00040

struct Material : ElemBase {
    ID id FAIL;

    float r,g,b WARN;
    float specr,specg,specb WARN;
    short har;
    float ambr,ambg,ambb WARN;
    float mirr,mirg,mirb;
    float emit WARN;
    float ray_mirror;
    float alpha WARN;
    float ref;
    float translucency;
    int mode;
    float roughness;
    float darkness;
    float refrac;

    float amb;
    float ang;
    float spectra;
    float spec;
    float zoffs;
    float add;
    float fresnel_mir;
    float fresnel_mir_i;
    float fresnel_tra;
    float fresnel_tra_i;
    float filter;
    float tx_limit;
    float tx_falloff;
    float gloss_mir;
    float gloss_tra;
    float adapt_thresh_mir;
    float adapt_thresh_tra;
    float aniso_gloss_mir;
    float dist_mir;
    float hasize;
    float flaresize;
    float subsize;
    float flareboost;
    float strand_sta;
    float strand_end;
    float strand_ease;
    float strand_surfnor;
    float strand_min;
    float strand_widthfade;
    float sbias;
    float lbias;
    float shad_alpha;
    float param;
    float rms;
    float rampfac_col;
    float rampfac_spec;
    float friction;
    float fh;
    float reflect;
    float fhdist;
    float xyfrict;
    float sss_radius;
    float sss_col;
    float sss_error;
    float sss_scale;
    float sss_ior;
    float sss_colfac;
    float sss_texfac;
    float sss_front;
    float sss_back;

    short material_type;
    short flag;
    short ray_depth;
    short ray_depth_tra;
    short samp_gloss_mir;
    short samp_gloss_tra;
    short fadeto_mir;
    short shade_flag;
    short flarec;
    short starc;
    short linec;
    short ringc;
    short pr_lamp;
    short pr_texture;
    short ml_flag;
    short texco;
    short mapto;
    short ramp_show;
    short pad3;
    short dynamode;
    short pad2;
    short sss_flag;
    short sss_preset;
    short shadowonly_flag;
    short index;
    short vcol_alpha;
    short pad4;

    char seed1;
    char seed2;

    std::shared_ptr<Group> group;

    short diff_shader WARN;
    short spec_shader WARN;

    std::shared_ptr<MTex> mtex[18];
};

/*
CustomDataLayer 104

    int type 0 4
    int offset 4 4
    int flag 8 4
    int active 12 4
    int active_rnd 16 4
    int active_clone 20 4
    int active_mask 24 4
    int uid 28 4
    char name 32 64
    void *data 96 8
*/
struct CustomDataLayer : ElemBase {
    int type;
    int offset;
    int flag;
    int active;
    int active_rnd;
    int active_clone;
    int active_mask;
    int uid;
    char name[64];
    std::shared_ptr<ElemBase> data;     // must be converted to real type according type member

    CustomDataLayer()
        : ElemBase()
        , type(0)
        , offset(0)
        , flag(0)
        , active(0)
        , active_rnd(0)
        , active_clone(0)
        , active_mask(0)
        , uid(0)
        , data(nullptr)
    {
        memset(name, 0, sizeof name);
    }
};

/*
CustomData 208

    CustomDataLayer *layers 0 8
    int typemap 8 168
    int pad_i1 176 4
    int totlayer 180 4
    int maxlayer 184 4
    int totsize 188 4
    BLI_mempool *pool 192 8
    CustomDataExternal *external 200 8
*/
struct CustomData : ElemBase {
    vector<std::shared_ptr<struct CustomDataLayer> > layers;
    int typemap[42];    // CD_NUMTYPES
    int totlayer;
    int maxlayer;
    int totsize;
    /*
    std::shared_ptr<BLI_mempool> pool;
    std::shared_ptr<CustomDataExternal> external;
    */
};

// -------------------------------------------------------------------------------
struct Mesh : ElemBase {
    ID id FAIL;

    int totface FAIL;
    int totedge FAIL;
    int totvert FAIL;
    int totloop;
    int totpoly;

    short subdiv;
    short subdivr;
    short subsurftype;
    short smoothresh;

    vector<MFace> mface FAIL;
    vector<MTFace> mtface;
    vector<TFace> tface;
    vector<MVert> mvert FAIL;
    vector<MEdge> medge WARN;
    vector<MLoop> mloop;
    vector<MLoopUV> mloopuv;
    vector<MLoopCol> mloopcol;
    vector<MPoly> mpoly;
    vector<MTexPoly> mtpoly;
    vector<MDeformVert> dvert;
    vector<MCol> mcol;

    vector< std::shared_ptr<Material> > mat FAIL;

    struct CustomData vdata;
    struct CustomData edata;
    struct CustomData fdata;
    struct CustomData pdata;
    struct CustomData ldata;
};

// -------------------------------------------------------------------------------
struct Library : ElemBase {
    ID id FAIL;

    char name[240] WARN;
    char filename[240] FAIL;
    std::shared_ptr<Library> parent WARN;
};

// -------------------------------------------------------------------------------
struct Camera : ElemBase {
    enum Type {
          Type_PERSP    =   0
         ,Type_ORTHO    =   1
    };

    ID id FAIL;

    Type type,flag WARN;
    float lens WARN;
    float sensor_x WARN;
    float clipsta, clipend;
};


// -------------------------------------------------------------------------------
struct Lamp : ElemBase {

    enum FalloffType {
         FalloffType_Constant   = 0x0
        ,FalloffType_InvLinear  = 0x1
        ,FalloffType_InvSquare  = 0x2
        //,FalloffType_Curve    = 0x3
        //,FalloffType_Sliders  = 0x4
    };

    enum Type {
         Type_Local         = 0x0
        ,Type_Sun           = 0x1
        ,Type_Spot          = 0x2
        ,Type_Hemi          = 0x3
        ,Type_Area          = 0x4
        //,Type_YFPhoton    = 0x5
    };

      ID id FAIL;
      //AnimData *adt;

      Type type FAIL;
      short flags;

      //int mode;

      short colormodel, totex;
      float r,g,b,k WARN;
      //float shdwr, shdwg, shdwb;

      float energy, dist, spotsize, spotblend;
      //float haint;

      float att1, att2;
      //struct CurveMapping *curfalloff;
      FalloffType falloff_type;

      //float clipsta, clipend, shadspotsize;
      //float bias, soft, compressthresh;
      //short bufsize, samp, buffers, filtertype;
      //char bufflag, buftype;

      //short ray_samp, ray_sampy, ray_sampz;
      //short ray_samp_type;
      short area_shape;
      float area_size, area_sizey, area_sizez;
      //float adapt_thresh;
      //short ray_samp_method;

      //short texact, shadhalostep;

      //short sun_effect_type;
      //short skyblendtype;
      //float horizon_brightness;
      //float spread;
      float sun_brightness;
      //float sun_size;
      //float backscattered_light;
      //float sun_intensity;
      //float atm_turbidity;
      //float atm_inscattering_factor;
      //float atm_extinction_factor;
      //float atm_distance_factor;
      //float skyblendfac;
      //float sky_exposure;
      //short sky_colorspace;

      // int YF_numphotons, YF_numsearch;
      // short YF_phdepth, YF_useqmc, YF_bufsize, YF_pad;
      // float YF_causticblur, YF_ltradius;

      // float YF_glowint, YF_glowofs;
      // short YF_glowtype, YF_pad2;

      //struct Ipo *ipo;
      //struct MTex *mtex[18];
      // short pr_texture;

      //struct PreviewImage *preview;
};

// -------------------------------------------------------------------------------
struct ModifierData : ElemBase  {
    enum ModifierType {
      eModifierType_None = 0,
      eModifierType_Subsurf,
      eModifierType_Lattice,
      eModifierType_Curve,
      eModifierType_Build,
      eModifierType_Mirror,
      eModifierType_Decimate,
      eModifierType_Wave,
      eModifierType_Armature,
      eModifierType_Hook,
      eModifierType_Softbody,
      eModifierType_Boolean,
      eModifierType_Array,
      eModifierType_EdgeSplit,
      eModifierType_Displace,
      eModifierType_UVProject,
      eModifierType_Smooth,
      eModifierType_Cast,
      eModifierType_MeshDeform,
      eModifierType_ParticleSystem,
      eModifierType_ParticleInstance,
      eModifierType_Explode,
      eModifierType_Cloth,
      eModifierType_Collision,
      eModifierType_Bevel,
      eModifierType_Shrinkwrap,
      eModifierType_Fluidsim,
      eModifierType_Mask,
      eModifierType_SimpleDeform,
      eModifierType_Multires,
      eModifierType_Surface,
      eModifierType_Smoke,
      eModifierType_ShapeKey
    };

    std::shared_ptr<ElemBase> next WARN;
    std::shared_ptr<ElemBase> prev WARN;

    int type, mode;
    char name[32];
};

// -------------------------------------------------------------------------------
struct SubsurfModifierData : ElemBase  {

    enum Type {

        TYPE_CatmullClarke = 0x0,
        TYPE_Simple = 0x1
    };

    enum Flags {
        // some omitted
        FLAGS_SubsurfUV     =1<<3
    };

    ModifierData modifier FAIL;
    short subdivType WARN;
    short levels FAIL;
    short renderLevels ;
    short flags;
};

// -------------------------------------------------------------------------------
struct MirrorModifierData : ElemBase {

    enum Flags {
        Flags_CLIPPING      =1<<0,
        Flags_MIRROR_U      =1<<1,
        Flags_MIRROR_V      =1<<2,
        Flags_AXIS_X        =1<<3,
        Flags_AXIS_Y        =1<<4,
        Flags_AXIS_Z        =1<<5,
        Flags_VGROUP        =1<<6
    };

    ModifierData modifier FAIL;

    short axis, flag;
    float tolerance;
    std::shared_ptr<Object> mirror_ob;
};

// -------------------------------------------------------------------------------
struct Object : ElemBase  {
    ID id FAIL;

    enum Type {
         Type_EMPTY     =   0
        ,Type_MESH      =   1
        ,Type_CURVE     =   2
        ,Type_SURF      =   3
        ,Type_FONT      =   4
        ,Type_MBALL     =   5

        ,Type_LAMP      =   10
        ,Type_CAMERA    =   11

        ,Type_WAVE      =   21
        ,Type_LATTICE   =   22
    };

    Type type FAIL;
    float obmat[4][4] WARN;
    float parentinv[4][4] WARN;
    char parsubstr[32] WARN;

    Object* parent WARN;
    std::shared_ptr<Object> track WARN;

    std::shared_ptr<Object> proxy,proxy_from,proxy_group WARN;
    std::shared_ptr<Group> dup_group WARN;
    std::shared_ptr<ElemBase> data FAIL;

    ListBase modifiers;

    Object()
    : ElemBase()
    , type( Type_EMPTY )
    , parent( nullptr )
    , track()
    , proxy()
    , proxy_from()
    , data() {
        // empty
    }
};


// -------------------------------------------------------------------------------
struct Base : ElemBase {
    Base* prev WARN;
    std::shared_ptr<Base> next WARN;
    std::shared_ptr<Object> object WARN;

    Base() 
    : ElemBase()
    , prev( nullptr )
    , next()
    , object() {
        // empty
        // empty
    }
};

// -------------------------------------------------------------------------------
struct Scene : ElemBase {
    ID id FAIL;

    std::shared_ptr<Object> camera WARN;
    std::shared_ptr<World> world WARN;
    std::shared_ptr<Base> basact WARN;

    ListBase base;

    Scene()
    : ElemBase()
    , camera()
    , world()
    , basact() {
        // empty
    }
};

// -------------------------------------------------------------------------------
struct Image : ElemBase {
    ID id FAIL;

    char name[240] WARN;

    //struct anim *anim;

    short ok, flag;
    short source, type, pad, pad1;
    int lastframe;

    short tpageflag, totbind;
    short xrep, yrep;
    short twsta, twend;
    //unsigned int bindcode;
    //unsigned int *repbind;

    std::shared_ptr<PackedFile> packedfile;
    //struct PreviewImage * preview;

    float lastupdate;
    int lastused;
    short animspeed;

    short gen_x, gen_y, gen_type;
    
    Image()
    : ElemBase() {
        // empty
    }
};

// -------------------------------------------------------------------------------
struct Tex : ElemBase {

    // actually, the only texture type we support is Type_IMAGE
    enum Type {
         Type_CLOUDS        = 1
        ,Type_WOOD          = 2
        ,Type_MARBLE        = 3
        ,Type_MAGIC         = 4
        ,Type_BLEND         = 5
        ,Type_STUCCI        = 6
        ,Type_NOISE         = 7
        ,Type_IMAGE         = 8
        ,Type_PLUGIN        = 9
        ,Type_ENVMAP        = 10
        ,Type_MUSGRAVE      = 11
        ,Type_VORONOI       = 12
        ,Type_DISTNOISE     = 13
        ,Type_POINTDENSITY  = 14
        ,Type_VOXELDATA     = 15
    };

    enum ImageFlags {
         ImageFlags_INTERPOL         = 1
        ,ImageFlags_USEALPHA         = 2
        ,ImageFlags_MIPMAP           = 4
        ,ImageFlags_IMAROT           = 16
        ,ImageFlags_CALCALPHA        = 32
        ,ImageFlags_NORMALMAP        = 2048
        ,ImageFlags_GAUSS_MIP        = 4096
        ,ImageFlags_FILTER_MIN       = 8192
        ,ImageFlags_DERIVATIVEMAP   = 16384
    };

    ID id FAIL;
    // AnimData *adt;

    //float noisesize, turbul;
    //float bright, contrast, rfac, gfac, bfac;
    //float filtersize;

    //float mg_H, mg_lacunarity, mg_octaves, mg_offset, mg_gain;
    //float dist_amount, ns_outscale;

    //float vn_w1;
    //float vn_w2;
    //float vn_w3;
    //float vn_w4;
    //float vn_mexp;
    //short vn_distm, vn_coltype;

    //short noisedepth, noisetype;
    //short noisebasis, noisebasis2;

    //short flag;
    ImageFlags imaflag;
    Type type FAIL;
    //short stype;

    //float cropxmin, cropymin, cropxmax, cropymax;
    //int texfilter;
    //int afmax;
    //short xrepeat, yrepeat;
    //short extend;

    //short fie_ima;
    //int len;
    //int frames, offset, sfra;

    //float checkerdist, nabla;
    //float norfac;

    //ImageUser iuser;

    //bNodeTree *nodetree;
    //Ipo *ipo;
    std::shared_ptr<Image> ima WARN;
    //PluginTex *plugin;
    //ColorBand *coba;
    //EnvMap *env;
    //PreviewImage * preview;
    //PointDensity *pd;
    //VoxelData *vd;

    //char use_nodes;

    Tex()
    : ElemBase()
    , imaflag( ImageFlags_INTERPOL )
    , type( Type_CLOUDS )
    , ima() {
        // empty
    }
};

// -------------------------------------------------------------------------------
struct MTex : ElemBase {

    enum Projection {
         Proj_N = 0
        ,Proj_X = 1
        ,Proj_Y = 2
        ,Proj_Z = 3
    };

    enum Flag {
         Flag_RGBTOINT      = 0x1
        ,Flag_STENCIL       = 0x2
        ,Flag_NEGATIVE      = 0x4
        ,Flag_ALPHAMIX      = 0x8
        ,Flag_VIEWSPACE     = 0x10
    };

    enum BlendType {
         BlendType_BLEND            = 0
        ,BlendType_MUL              = 1
        ,BlendType_ADD              = 2
        ,BlendType_SUB              = 3
        ,BlendType_DIV              = 4
        ,BlendType_DARK             = 5
        ,BlendType_DIFF             = 6
        ,BlendType_LIGHT            = 7
        ,BlendType_SCREEN           = 8
        ,BlendType_OVERLAY          = 9
        ,BlendType_BLEND_HUE        = 10
        ,BlendType_BLEND_SAT        = 11
        ,BlendType_BLEND_VAL        = 12
        ,BlendType_BLEND_COLOR      = 13
    };

    enum MapType {
         MapType_COL         = 1
        ,MapType_NORM        = 2
        ,MapType_COLSPEC     = 4
        ,MapType_COLMIR      = 8
        ,MapType_REF         = 16
        ,MapType_SPEC        = 32
        ,MapType_EMIT        = 64
        ,MapType_ALPHA       = 128
        ,MapType_HAR         = 256
        ,MapType_RAYMIRR     = 512
        ,MapType_TRANSLU     = 1024
        ,MapType_AMB         = 2048
        ,MapType_DISPLACE    = 4096
        ,MapType_WARP        = 8192
    };

    // short texco, maptoneg;
    MapType mapto;

    BlendType blendtype;
    std::shared_ptr<Object> object;
    std::shared_ptr<Tex> tex;
    char uvname[32];

    Projection projx,projy,projz;
    char mapping;
    float ofs[3], size[3], rot;

    int texflag;
    short colormodel, pmapto, pmaptoneg;
    //short normapspace, which_output;
    //char brush_map_mode;
    float r,g,b,k WARN;
    //float def_var, rt;

    //float colfac, varfac;

    float norfac;
    //float dispfac, warpfac;
    float colspecfac, mirrfac, alphafac;
    float difffac, specfac, emitfac, hardfac;
    //float raymirrfac, translfac, ambfac;
    //float colemitfac, colreflfac, coltransfac;
    //float densfac, scatterfac, reflfac;

    //float timefac, lengthfac, clumpfac;
    //float kinkfac, roughfac, padensfac;
    //float lifefac, sizefac, ivelfac, pvelfac;
    //float shadowfac;
    //float zenupfac, zendownfac, blendfac;

    MTex()
    : ElemBase() {
        // empty
    }
};

}
}
#endif
