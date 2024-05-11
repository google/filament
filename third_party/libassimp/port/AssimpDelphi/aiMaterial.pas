unit aiMaterial;

interface

uses aiTypes, aiVector2D, aiVector3D;

{This following directive causes enums to be stored as double words (32bit), to be compatible with
 the assimp C Dll}
{$Z4}

type TaiTextureOp = (
	aiTextureOp_Multiply = $0,
	aiTextureOp_Add = $1,
	aiTextureOp_Subtract = $2,
	aiTextureOp_Divide = $3,
	aiTextureOp_SmoothAdd = $4,
	aiTextureOp_SignedAdd = $5
	//_aiTextureOp_Force32Bit = 0x9fffffff
);

type TaiTextureMapMode = (
    aiTextureMapMode_Wrap = $0,
    aiTextureMapMode_Clamp = $1,
    aiTextureMapMode_Decal = $3,
    aiTextureMapMode_Mirror = $2
	 //_aiTextureMapMode_Force32Bit = 0x9fffffff
);

type TaiTextureMapping = (
    aiTextureMapping_UV = $0,
    aiTextureMapping_SPHERE = $1,
    aiTextureMapping_CYLINDER = $2,
    aiTextureMapping_BOX = $3,
    aiTextureMapping_PLANE = $4,
    aiTextureMapping_OTHER = $5
	 //_aiTextureMapping_Force32Bit = 0x9fffffff
);

type TaiTextureType = (
    aiTextureType_NONE = $0,
    aiTextureType_DIFFUSE = $1,
    aiTextureType_SPECULAR = $2,
    aiTextureType_AMBIENT = $3,
    aiTextureType_EMISSIVE = $4,
    aiTextureType_HEIGHT = $5,
    aiTextureType_NORMALS = $6,
    aiTextureType_SHININESS = $7,
    aiTextureType_OPACITY = $8,
    aiTextureType_DISPLACEMENT = $9,
    aiTextureType_LIGHTMAP = $A,
    aiTextureType_REFLECTION = $B,
    aiTextureType_UNKNOWN = $C
	 //_aiTextureType_Force32Bit = 0x9fffffff
);

const AI_TEXTURE_TYPE_MAX = aiTextureType_UNKNOWN;

type TaiShadingMode = (
    aiShadingMode_Flat = $1,
    aiShadingMode_Gouraud =  $2,
    aiShadingMode_Phong = $3,
    aiShadingMode_Blinn	= $4,
    aiShadingMode_Toon = $5,
    aiShadingMode_OrenNayar = $6,
    aiShadingMode_Minnaert = $7,
    aiShadingMode_CookTorrance = $8,
    aiShadingMode_NoShading = $9,
    aiShadingMode_Fresnel = $A
	 //_aiShadingMode_Force32Bit = 0x9fffffff
);


type TaiTextureFlags = (
	aiTextureFlags_Invert = $1,
	aiTextureFlags_UseAlpha = $2,
	aiTextureFlags_IgnoreAlpha = $4
	//_aiTextureFlags_Force32Bit = 0x9fffffff
);

type TaiBlendMode = (
	aiBlendMode_Default = $0,
	aiBlendMode_Additive = $1
	//_aiBlendMode_Force32Bit = 0x9fffffff
);

type TaiUVTransform = packed record
   mTranslation: TaiVector2D;
   mScaling: TaiVector2D;
   mRotation: single;
end;

type TaiPropertyTypeInfo = (
   aiPTI_Float   = $1,
   aiPTI_String  = $3,
   aiPTI_Integer = $4,
   aiPTI_Buffer  = $5
	// _aiPTI_Force32Bit = 0x9fffffff
);

type TaiMaterialProperty = packed record
   mKey: aiString;
   mSemantic: Cardinal;
   mIndex: Cardinal;
   mDataLength: Cardinal;
   mType: TaiPropertyTypeInfo;
   mData: PChar;
end;
type PaiMaterialProperty = ^TaiMaterialProperty;

type TaiMaterial = packed record
   mProperties: pointer;
   mNumProperties: Cardinal;
   mNumAllocated: Cardinal;
end;
type PaiMaterial = ^TaiMaterial;
type PaiMaterialArray = array[0..0] of PaiMaterial;
type PPaiMaterialArray = ^PaiMaterialArray;

const AI_MATKEY_NAME = '?mat.name';
const AI_MATKEY_TWOSIDED = '$mat.twosided';
const AI_MATKEY_SHADING_MODEL = '$mat.shadingm';
const AI_MATKEY_ENABLE_WIREFRAME = '$mat.wireframe';
const AI_MATKEY_BLEND_FUNC = '$mat.blend';
const AI_MATKEY_OPACITY = '$mat.opacity';
const AI_MATKEY_BUMPSCALING = '$mat.bumpscaling';
const AI_MATKEY_SHININESS = '$mat.shininess';
const AI_MATKEY_REFLECTIVITY = '$mat.reflectivity';
const AI_MATKEY_SHININESS_STRENGTH = '$mat.shinpercent';
const AI_MATKEY_REFRACTI = '$mat.refracti';
const AI_MATKEY_COLOR_DIFFUSE = '$clr.diffuse';
const AI_MATKEY_COLOR_AMBIENT = '$clr.ambient';
const AI_MATKEY_COLOR_SPECULAR = '$clr.specular';
const AI_MATKEY_COLOR_EMISSIVE = '$clr.emissive';
const AI_MATKEY_COLOR_TRANSPARENT = '$clr.transparent';
const AI_MATKEY_COLOR_REFLECTIVE = '$clr.reflective';
const AI_MATKEY_GLOBAL_BACKGROUND_IMAGE = '?bg.global';

const _AI_MATKEY_TEXTURE_BASE = '$tex.file';
const _AI_MATKEY_UVWSRC_BASE = '$tex.uvwsrc';
const _AI_MATKEY_TEXOP_BASE = '$tex.op';
const _AI_MATKEY_MAPPING_BASE = '$tex.mapping';
const _AI_MATKEY_TEXBLEND_BASE = '$tex.blend';
const _AI_MATKEY_MAPPINGMODE_U_BASE = '$tex.mapmodeu';
const _AI_MATKEY_MAPPINGMODE_V_BASE = '$tex.mapmodev';
const _AI_MATKEY_TEXMAP_AXIS_BASE = '$tex.mapaxis';
const _AI_MATKEY_UVTRANSFORM_BASE = '$tex.uvtrafo';
const _AI_MATKEY_TEXFLAGS_BASE = '$tex.flags';



implementation

end.
