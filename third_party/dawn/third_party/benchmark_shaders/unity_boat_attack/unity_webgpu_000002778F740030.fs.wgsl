diagnostic(off, derivative_uniformity);

alias Arr = array<vec4<f32>, 2u>;

struct UnityPerDraw {
  /* @offset(0) */
  unity_ObjectToWorld : mat4x4<f32>,
  /* @offset(64) */
  unity_WorldToObject : mat4x4<f32>,
  /* @offset(128) */
  unity_LODFade : vec4<f32>,
  /* @offset(144) */
  unity_WorldTransformParams : vec4<f32>,
  /* @offset(160) */
  unity_RenderingLayer : vec4<f32>,
  /* @offset(176) */
  unity_LightData : vec4<f32>,
  /* @offset(192) */
  unity_LightIndices : Arr,
  /* @offset(224) */
  unity_ProbesOcclusion : vec4<f32>,
  /* @offset(240) */
  unity_SpecCube0_HDR : vec4<f32>,
  /* @offset(256) */
  unity_SpecCube1_HDR : vec4<f32>,
  /* @offset(272) */
  unity_SpecCube0_BoxMax : vec4<f32>,
  /* @offset(288) */
  unity_SpecCube0_BoxMin : vec4<f32>,
  /* @offset(304) */
  unity_SpecCube0_ProbePosition : vec4<f32>,
  /* @offset(320) */
  unity_SpecCube1_BoxMax : vec4<f32>,
  /* @offset(336) */
  unity_SpecCube1_BoxMin : vec4<f32>,
  /* @offset(352) */
  unity_SpecCube1_ProbePosition : vec4<f32>,
  /* @offset(368) */
  unity_LightmapST : vec4<f32>,
  /* @offset(384) */
  unity_DynamicLightmapST : vec4<f32>,
  /* @offset(400) */
  unity_SHAr : vec4<f32>,
  /* @offset(416) */
  unity_SHAg : vec4<f32>,
  /* @offset(432) */
  unity_SHAb : vec4<f32>,
  /* @offset(448) */
  unity_SHBr : vec4<f32>,
  /* @offset(464) */
  unity_SHBg : vec4<f32>,
  /* @offset(480) */
  unity_SHBb : vec4<f32>,
  /* @offset(496) */
  unity_SHC : vec4<f32>,
  /* @offset(512) */
  unity_RendererBounds_Min : vec4<f32>,
  /* @offset(528) */
  unity_RendererBounds_Max : vec4<f32>,
  /* @offset(544) */
  unity_MatrixPreviousM : mat4x4<f32>,
  /* @offset(608) */
  unity_MatrixPreviousMI : mat4x4<f32>,
  /* @offset(672) */
  unity_MotionVectorsParams : vec4<f32>,
  /* @offset(688) */
  unity_SpriteColor : vec4<f32>,
  /* @offset(704) */
  unity_SpriteProps : vec4<f32>,
}

struct PGlobals {
  /* @offset(0) */
  x_GlobalMipBias : vec2<f32>,
  /* @offset(8) */
  x_AlphaToMaskAvailable : f32,
  /* @offset(16) */
  x_MainLightPosition : vec4<f32>,
  /* @offset(32) */
  x_MainLightColor : vec4<f32>,
  /* @offset(48) */
  x_AdditionalLightsCount : vec4<f32>,
  /* @offset(64) */
  x_WorldSpaceCameraPos : vec3<f32>,
  /* @offset(80) */
  x_ProjectionParams : vec4<f32>,
  /* @offset(96) */
  x_ScreenParams : vec4<f32>,
  /* @offset(112) */
  unity_OrthoParams : vec4<f32>,
  /* @offset(128) */
  unity_FogParams : vec4<f32>,
  /* @offset(144) */
  unity_FogColor : vec4<f32>,
  /* @offset(160) */
  unity_MatrixV : mat4x4<f32>,
}

struct UnityPerMaterial {
  /* @offset(0) */
  Texture2D_B222E8F_TexelSize : vec4<f32>,
  /* @offset(16) */
  Color_C30C7CA3 : vec4<f32>,
  /* @offset(32) */
  Texture2D_D9BFD5F1_TexelSize : vec4<f32>,
}

alias Arr_1 = array<mat4x4<f32>, 5u>;

alias Arr_2 = array<vec4<f32>, 32u>;

alias Arr_3 = array<mat4x4<f32>, 32u>;

alias Arr_4 = array<vec4<f32>, 32u>;

alias Arr_5 = array<mat4x4<f32>, 32u>;

struct LightShadows {
  /* @offset(0) */
  x_MainLightWorldToShadow : Arr_1,
  /* @offset(320) */
  x_CascadeShadowSplitSpheres0 : vec4<f32>,
  /* @offset(336) */
  x_CascadeShadowSplitSpheres1 : vec4<f32>,
  /* @offset(352) */
  x_CascadeShadowSplitSpheres2 : vec4<f32>,
  /* @offset(368) */
  x_CascadeShadowSplitSpheres3 : vec4<f32>,
  /* @offset(384) */
  x_CascadeShadowSplitSphereRadii : vec4<f32>,
  /* @offset(400) */
  x_MainLightShadowOffset0 : vec4<f32>,
  /* @offset(416) */
  x_MainLightShadowOffset1 : vec4<f32>,
  /* @offset(432) */
  x_MainLightShadowParams : vec4<f32>,
  /* @offset(448) */
  x_MainLightShadowmapSize : vec4<f32>,
  /* @offset(464) */
  x_AdditionalShadowOffset0 : vec4<f32>,
  /* @offset(480) */
  x_AdditionalShadowOffset1 : vec4<f32>,
  /* @offset(496) */
  x_AdditionalShadowFadeParams : vec4<f32>,
  /* @offset(512) */
  x_AdditionalShadowmapSize : vec4<f32>,
  /* @offset(528) */
  x_AdditionalShadowParams : Arr_4,
  /* @offset(1040) */
  x_AdditionalLightsWorldToShadow : Arr_5,
}

alias Arr_6 = array<vec4<f32>, 32u>;

struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr_7 = array<strided_arr, 32u>;

alias Arr_8 = array<strided_arr, 32u>;

struct LightCookies {
  /* @offset(0) */
  x_MainLightWorldToLight : mat4x4<f32>,
  /* @offset(64) */
  x_AdditionalLightsCookieEnableBits : f32,
  /* @offset(68) */
  x_MainLightCookieTextureFormat : f32,
  /* @offset(72) */
  x_AdditionalLightsCookieAtlasTextureFormat : f32,
  /* @offset(80) */
  x_AdditionalLightsWorldToLights : Arr_5,
  /* @offset(2128) */
  x_AdditionalLightsCookieAtlasUVRects : Arr_4,
  /* @offset(2640) */
  x_AdditionalLightsLightTypes : Arr_8,
}

alias Arr_9 = array<vec4<f32>, 32u>;

alias Arr_10 = array<vec4<f32>, 32u>;

alias Arr_11 = array<vec4<f32>, 32u>;

alias Arr_12 = array<vec4<f32>, 32u>;

struct AdditionalLights {
  /* @offset(0) */
  x_AdditionalLightsPosition : Arr_4,
  /* @offset(512) */
  x_AdditionalLightsColor : Arr_4,
  /* @offset(1024) */
  x_AdditionalLightsAttenuation : Arr_4,
  /* @offset(1536) */
  x_AdditionalLightsSpotDir : Arr_4,
  /* @offset(2048) */
  x_AdditionalLightsOcclusionProbes : Arr_4,
  /* @offset(2560) */
  x_AdditionalLightsLayerMasks : Arr_8,
}

var<private> u_xlat0 : vec3<f32>;

var<private> vs_INTERP9 : vec3<f32>;

var<private> u_xlatb26 : vec2<bool>;

var<private> vs_INTERP4 : vec4<f32>;

@group(1) @binding(2) var<uniform> x_83 : UnityPerDraw;

var<private> u_xlat26 : vec3<f32>;

var<private> u_xlat1 : vec3<f32>;

var<private> u_xlat2 : vec4<f32>;

var<private> u_xlat3 : vec4<f32>;

var<private> u_xlatb0 : bool;

@group(1) @binding(0) var<uniform> x_149 : PGlobals;

var<private> u_xlat4 : vec3<f32>;

var<private> vs_INTERP8 : vec3<f32>;

var<private> u_xlat79 : f32;

var<private> u_xlat5 : vec4<f32>;

var<private> u_xlat6 : vec4<f32>;

@group(0) @binding(7) var Texture2D_B222E8F : texture_2d<f32>;

@group(0) @binding(14) var samplerTexture2D_B222E8F : sampler;

var<private> vs_INTERP5 : vec4<f32>;

var<private> u_xlat7 : vec3<f32>;

@group(1) @binding(5) var<uniform> x_267 : UnityPerMaterial;

var<private> u_xlat8 : vec4<f32>;

@group(0) @binding(8) var Texture2D_D9BFD5F1 : texture_2d<f32>;

@group(0) @binding(15) var samplerTexture2D_D9BFD5F1 : sampler;

var<private> u_xlat9 : vec4<f32>;

var<private> u_xlat34 : vec3<f32>;

var<private> vs_INTERP6 : vec4<f32>;

@group(1) @binding(3) var<uniform> x_372 : LightShadows;

var<private> u_xlat10 : vec4<f32>;

var<private> u_xlatb2 : vec4<bool>;

var<private> u_xlatu0 : u32;

var<private> u_xlati0 : i32;

var<private> u_xlatb79 : bool;

@group(0) @binding(3) var x_MainLightShadowmapTexture : texture_depth_2d;

@group(0) @binding(12) var sampler_LinearClampCompare : sampler_comparison;

var<private> u_xlatb80 : bool;

var<private> u_xlat55 : vec2<f32>;

var<private> u_xlat62 : vec2<f32>;

var<private> u_xlat11 : vec4<f32>;

var<private> u_xlat12 : vec4<f32>;

var<private> u_xlat13 : vec4<f32>;

var<private> u_xlat14 : vec4<f32>;

var<private> u_xlat15 : vec4<f32>;

var<private> u_xlat16 : vec4<f32>;

var<private> u_xlat80 : f32;

var<private> u_xlat29 : f32;

var<private> u_xlat35 : vec3<f32>;

var<private> u_xlat17 : vec4<f32>;

var<private> u_xlat18 : vec4<f32>;

var<private> u_xlat36 : vec2<f32>;

var<private> u_xlat68 : vec2<f32>;

var<private> u_xlat63 : vec2<f32>;

var<private> u_xlat19 : vec4<f32>;

var<private> u_xlat20 : vec4<f32>;

var<private> u_xlat21 : vec4<f32>;

var<private> u_xlat82 : f32;

var<private> u_xlatb3 : vec4<bool>;

var<private> u_xlatb29 : bool;

var<private> u_xlat27 : vec3<f32>;

var<private> u_xlatu5 : vec3<u32>;

var<private> u_xlatu55 : u32;

var<private> u_xlatu81 : u32;

var<private> u_xlati55 : i32;

var<private> u_xlat81 : f32;

var<private> u_xlatb55 : bool;

@group(0) @binding(2) var unity_LightmapInd : texture_2d<f32>;

@group(0) @binding(10) var samplerunity_Lightmap : sampler;

var<private> vs_INTERP0 : vec2<f32>;

@group(0) @binding(1) var unity_Lightmap : texture_2d<f32>;

var<private> u_xlat83 : f32;

var<private> u_xlat84 : f32;

var<private> u_xlat33 : f32;

var<private> u_xlatb59 : bool;

var<private> u_xlat59 : vec2<f32>;

var<private> u_xlat60 : vec2<f32>;

var<private> u_xlat85 : f32;

var<private> u_xlat66 : vec2<f32>;

var<private> u_xlat87 : f32;

var<private> u_xlat28 : vec3<f32>;

var<private> u_xlat54 : f32;

var<private> u_xlatb28 : vec2<bool>;

@group(1) @binding(4) var<uniform> x_3536 : LightCookies;

@group(0) @binding(5) var x_MainLightCookieTexture : texture_2d<f32>;

@group(0) @binding(13) var sampler_MainLightCookieTexture : sampler;

@group(0) @binding(0) var unity_SpecCube0 : texture_cube<f32>;

@group(0) @binding(9) var samplerunity_SpecCube0 : sampler;

var<private> u_xlatu84 : u32;

var<private> u_xlati85 : i32;

var<private> u_xlati84 : i32;

@group(1) @binding(1) var<uniform> x_4021 : AdditionalLights;

var<private> u_xlat86 : f32;

var<private> u_xlati87 : i32;

var<private> u_xlatb88 : bool;

var<private> u_xlatb11 : vec4<bool>;

var<private> u_xlat89 : f32;

var<private> u_xlat37 : vec3<f32>;

var<private> u_xlat88 : f32;

var<private> u_xlatb87 : bool;

@group(0) @binding(4) var x_AdditionalLightsShadowmapTexture : texture_depth_2d;

var<private> u_xlat64 : vec2<f32>;

var<private> u_xlat39 : vec3<f32>;

var<private> u_xlat22 : vec4<f32>;

var<private> u_xlat40 : vec2<f32>;

var<private> u_xlat72 : vec2<f32>;

var<private> u_xlat67 : vec2<f32>;

var<private> u_xlat23 : vec4<f32>;

var<private> u_xlat24 : vec4<f32>;

var<private> u_xlat25 : vec4<f32>;

var<private> u_xlati88 : i32;

var<private> u_xlati11 : i32;

var<private> u_xlati37 : i32;

var<private> u_xlatb37 : vec3<bool>;

@group(0) @binding(6) var x_AdditionalLightsCookieAtlasTexture : texture_2d<f32>;

@group(0) @binding(11) var sampler_LinearClamp : sampler;

var<private> u_xlat78 : f32;

var<private> SV_Target0 : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

var<private> u_xlatu82 : u32;

var<private> u_xlatb84 : bool;

fn int_bitfieldInsert_i1_i1_i1_i1_(base : ptr<function, i32>, insert : ptr<function, i32>, offset_1 : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  let x_19 : i32 = *(bits);
  let x_23 : i32 = *(offset_1);
  mask = (~((4294967295u << bitcast<u32>(x_19))) << bitcast<u32>(x_23));
  let x_26 : i32 = *(base);
  let x_28 : u32 = mask;
  let x_31 : i32 = *(insert);
  let x_33 : i32 = *(offset_1);
  let x_36 : u32 = mask;
  return bitcast<i32>(((bitcast<u32>(x_26) & ~(x_28)) | ((bitcast<u32>(x_31) << bitcast<u32>(x_33)) & x_36)));
}

fn main_1() {
  var x_189 : vec3<f32>;
  var txVec0 : vec3<f32>;
  var txVec1 : vec3<f32>;
  var txVec2 : vec3<f32>;
  var txVec3 : vec3<f32>;
  var txVec4 : vec3<f32>;
  var txVec5 : vec3<f32>;
  var txVec6 : vec3<f32>;
  var txVec7 : vec3<f32>;
  var txVec8 : vec3<f32>;
  var txVec9 : vec3<f32>;
  var txVec10 : vec3<f32>;
  var txVec11 : vec3<f32>;
  var txVec12 : vec3<f32>;
  var txVec13 : vec3<f32>;
  var txVec14 : vec3<f32>;
  var txVec15 : vec3<f32>;
  var txVec16 : vec3<f32>;
  var txVec17 : vec3<f32>;
  var txVec18 : vec3<f32>;
  var txVec19 : vec3<f32>;
  var txVec20 : vec3<f32>;
  var txVec21 : vec3<f32>;
  var txVec22 : vec3<f32>;
  var txVec23 : vec3<f32>;
  var txVec24 : vec3<f32>;
  var txVec25 : vec3<f32>;
  var txVec26 : vec3<f32>;
  var txVec27 : vec3<f32>;
  var txVec28 : vec3<f32>;
  var txVec29 : vec3<f32>;
  var x_1876 : f32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var x_2001 : f32;
  var x_2055 : f32;
  var txVec30 : vec3<f32>;
  var txVec31 : vec3<f32>;
  var txVec32 : vec3<f32>;
  var txVec33 : vec3<f32>;
  var txVec34 : vec3<f32>;
  var txVec35 : vec3<f32>;
  var txVec36 : vec3<f32>;
  var txVec37 : vec3<f32>;
  var txVec38 : vec3<f32>;
  var txVec39 : vec3<f32>;
  var txVec40 : vec3<f32>;
  var txVec41 : vec3<f32>;
  var txVec42 : vec3<f32>;
  var txVec43 : vec3<f32>;
  var txVec44 : vec3<f32>;
  var txVec45 : vec3<f32>;
  var txVec46 : vec3<f32>;
  var txVec47 : vec3<f32>;
  var txVec48 : vec3<f32>;
  var txVec49 : vec3<f32>;
  var txVec50 : vec3<f32>;
  var txVec51 : vec3<f32>;
  var txVec52 : vec3<f32>;
  var txVec53 : vec3<f32>;
  var txVec54 : vec3<f32>;
  var txVec55 : vec3<f32>;
  var txVec56 : vec3<f32>;
  var txVec57 : vec3<f32>;
  var txVec58 : vec3<f32>;
  var txVec59 : vec3<f32>;
  var x_3482 : f32;
  var x_3618 : f32;
  var x_3629 : vec3<f32>;
  var u_xlatu_loop_1 : u32;
  var indexable : array<vec4<u32>, 4u>;
  var x_4158 : f32;
  var x_4169 : f32;
  var txVec60 : vec3<f32>;
  var txVec61 : vec3<f32>;
  var txVec62 : vec3<f32>;
  var txVec63 : vec3<f32>;
  var txVec64 : vec3<f32>;
  var txVec65 : vec3<f32>;
  var txVec66 : vec3<f32>;
  var txVec67 : vec3<f32>;
  var txVec68 : vec3<f32>;
  var txVec69 : vec3<f32>;
  var txVec70 : vec3<f32>;
  var txVec71 : vec3<f32>;
  var txVec72 : vec3<f32>;
  var txVec73 : vec3<f32>;
  var txVec74 : vec3<f32>;
  var txVec75 : vec3<f32>;
  var txVec76 : vec3<f32>;
  var txVec77 : vec3<f32>;
  var txVec78 : vec3<f32>;
  var txVec79 : vec3<f32>;
  var txVec80 : vec3<f32>;
  var txVec81 : vec3<f32>;
  var txVec82 : vec3<f32>;
  var txVec83 : vec3<f32>;
  var txVec84 : vec3<f32>;
  var txVec85 : vec3<f32>;
  var txVec86 : vec3<f32>;
  var txVec87 : vec3<f32>;
  var txVec88 : vec3<f32>;
  var txVec89 : vec3<f32>;
  var x_5784 : f32;
  var x_5797 : f32;
  var x_5855 : f32;
  var x_5866 : vec3<f32>;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  let x_48 : vec3<f32> = vs_INTERP9;
  let x_49 : vec3<f32> = vs_INTERP9;
  u_xlat0.x = dot(x_48, x_49);
  let x_55 : f32 = u_xlat0.x;
  u_xlat0.x = sqrt(x_55);
  let x_60 : f32 = u_xlat0.x;
  u_xlat0.x = (1.0f / x_60);
  let x_74 : f32 = vs_INTERP4.w;
  u_xlatb26.x = (0.0f < x_74);
  let x_87 : f32 = x_83.unity_WorldTransformParams.w;
  u_xlatb26.y = (x_87 >= 0.0f);
  let x_93 : bool = u_xlatb26.x;
  u_xlat26.x = select(-1.0f, 1.0f, x_93);
  let x_98 : bool = u_xlatb26.y;
  u_xlat26.y = select(-1.0f, 1.0f, x_98);
  let x_102 : f32 = u_xlat26.y;
  let x_104 : f32 = u_xlat26.x;
  u_xlat26.x = (x_102 * x_104);
  let x_108 : vec4<f32> = vs_INTERP4;
  let x_110 : vec3<f32> = vs_INTERP9;
  u_xlat1 = (vec3<f32>(x_108.y, x_108.z, x_108.x) * vec3<f32>(x_110.z, x_110.x, x_110.y));
  let x_113 : vec3<f32> = vs_INTERP9;
  let x_115 : vec4<f32> = vs_INTERP4;
  let x_118 : vec3<f32> = u_xlat1;
  u_xlat1 = ((vec3<f32>(x_113.y, x_113.z, x_113.x) * vec3<f32>(x_115.z, x_115.x, x_115.y)) + -(x_118));
  let x_121 : vec3<f32> = u_xlat26;
  let x_123 : vec3<f32> = u_xlat1;
  u_xlat26 = (vec3<f32>(x_121.x, x_121.x, x_121.x) * x_123);
  let x_125 : vec3<f32> = u_xlat0;
  let x_127 : vec3<f32> = vs_INTERP9;
  u_xlat1 = (vec3<f32>(x_125.x, x_125.x, x_125.x) * x_127);
  let x_131 : vec3<f32> = u_xlat0;
  let x_133 : vec4<f32> = vs_INTERP4;
  let x_135 : vec3<f32> = (vec3<f32>(x_131.x, x_131.x, x_131.x) * vec3<f32>(x_133.x, x_133.y, x_133.z));
  let x_136 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_135.x, x_135.y, x_135.z, x_136.w);
  let x_139 : vec3<f32> = u_xlat26;
  let x_140 : vec3<f32> = u_xlat0;
  let x_142 : vec3<f32> = (x_139 * vec3<f32>(x_140.x, x_140.x, x_140.x));
  let x_143 : vec4<f32> = u_xlat3;
  u_xlat3 = vec4<f32>(x_142.x, x_142.y, x_142.z, x_143.w);
  let x_152 : f32 = x_149.unity_OrthoParams.w;
  u_xlatb0 = (x_152 == 0.0f);
  let x_156 : vec3<f32> = vs_INTERP8;
  let x_161 : vec3<f32> = x_149.x_WorldSpaceCameraPos;
  u_xlat4 = (-(x_156) + x_161);
  let x_164 : vec3<f32> = u_xlat4;
  let x_165 : vec3<f32> = u_xlat4;
  u_xlat79 = dot(x_164, x_165);
  let x_167 : f32 = u_xlat79;
  u_xlat79 = inverseSqrt(x_167);
  let x_169 : f32 = u_xlat79;
  let x_171 : vec3<f32> = u_xlat4;
  u_xlat4 = (vec3<f32>(x_169, x_169, x_169) * x_171);
  let x_177 : f32 = x_149.unity_MatrixV[0i].z;
  u_xlat5.x = x_177;
  let x_181 : f32 = x_149.unity_MatrixV[1i].z;
  u_xlat5.y = x_181;
  let x_185 : f32 = x_149.unity_MatrixV[2i].z;
  u_xlat5.z = x_185;
  let x_187 : bool = u_xlatb0;
  if (x_187) {
    let x_192 : vec3<f32> = u_xlat4;
    x_189 = x_192;
  } else {
    let x_194 : vec4<f32> = u_xlat5;
    x_189 = vec3<f32>(x_194.x, x_194.y, x_194.z);
  }
  let x_196 : vec3<f32> = x_189;
  u_xlat4 = x_196;
  let x_197 : vec3<f32> = u_xlat4;
  let x_201 : vec4<f32> = x_83.unity_WorldToObject[1i];
  let x_203 : vec3<f32> = (vec3<f32>(x_197.y, x_197.y, x_197.y) * vec3<f32>(x_201.x, x_201.y, x_201.z));
  let x_204 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_203.x, x_203.y, x_203.z, x_204.w);
  let x_207 : vec4<f32> = x_83.unity_WorldToObject[0i];
  let x_209 : vec3<f32> = u_xlat4;
  let x_212 : vec4<f32> = u_xlat5;
  let x_214 : vec3<f32> = ((vec3<f32>(x_207.x, x_207.y, x_207.z) * vec3<f32>(x_209.x, x_209.x, x_209.x)) + vec3<f32>(x_212.x, x_212.y, x_212.z));
  let x_215 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_214.x, x_214.y, x_214.z, x_215.w);
  let x_218 : vec4<f32> = x_83.unity_WorldToObject[2i];
  let x_220 : vec3<f32> = u_xlat4;
  let x_223 : vec4<f32> = u_xlat5;
  let x_225 : vec3<f32> = ((vec3<f32>(x_218.x, x_218.y, x_218.z) * vec3<f32>(x_220.z, x_220.z, x_220.z)) + vec3<f32>(x_223.x, x_223.y, x_223.z));
  let x_226 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_225.x, x_225.y, x_225.z, x_226.w);
  let x_228 : vec4<f32> = u_xlat5;
  let x_230 : vec4<f32> = u_xlat5;
  u_xlat0.x = dot(vec3<f32>(x_228.x, x_228.y, x_228.z), vec3<f32>(x_230.x, x_230.y, x_230.z));
  let x_235 : f32 = u_xlat0.x;
  u_xlat0.x = inverseSqrt(x_235);
  let x_238 : vec3<f32> = u_xlat0;
  let x_240 : vec4<f32> = u_xlat5;
  let x_242 : vec3<f32> = (vec3<f32>(x_238.x, x_238.x, x_238.x) * vec3<f32>(x_240.x, x_240.y, x_240.z));
  let x_243 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_242.x, x_242.y, x_242.z, x_243.w);
  let x_257 : vec4<f32> = vs_INTERP5;
  let x_260 : f32 = x_149.x_GlobalMipBias.x;
  let x_261 : vec4<f32> = textureSampleBias(Texture2D_B222E8F, samplerTexture2D_B222E8F, vec2<f32>(x_257.x, x_257.y), x_260);
  u_xlat6 = x_261;
  let x_263 : vec4<f32> = u_xlat6;
  let x_269 : vec4<f32> = x_267.Color_C30C7CA3;
  u_xlat7 = (vec3<f32>(x_263.x, x_263.y, x_263.z) * vec3<f32>(x_269.x, x_269.y, x_269.z));
  let x_278 : vec4<f32> = vs_INTERP5;
  let x_281 : f32 = x_149.x_GlobalMipBias.x;
  let x_282 : vec4<f32> = textureSampleBias(Texture2D_D9BFD5F1, samplerTexture2D_D9BFD5F1, vec2<f32>(x_278.x, x_278.y), x_281);
  u_xlat8 = vec4<f32>(x_282.w, x_282.x, x_282.y, x_282.z);
  let x_285 : vec4<f32> = u_xlat8;
  u_xlat9 = ((vec4<f32>(x_285.y, x_285.z, x_285.w, x_285.x) * vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f)) + vec4<f32>(-1.0f, -1.0f, -1.0f, -1.0f));
  let x_292 : vec4<f32> = u_xlat9;
  let x_293 : vec4<f32> = u_xlat9;
  u_xlat0.x = dot(x_292, x_293);
  let x_297 : f32 = u_xlat0.x;
  u_xlat0.x = inverseSqrt(x_297);
  let x_301 : vec3<f32> = u_xlat0;
  let x_303 : vec4<f32> = u_xlat9;
  u_xlat34 = (vec3<f32>(x_301.x, x_301.x, x_301.x) * vec3<f32>(x_303.x, x_303.y, x_303.z));
  let x_308 : f32 = vs_INTERP6.y;
  u_xlat0.x = (x_308 * 200.0f);
  let x_313 : f32 = u_xlat0.x;
  u_xlat0.x = min(x_313, 1.0f);
  let x_316 : vec3<f32> = u_xlat0;
  let x_318 : vec4<f32> = u_xlat6;
  let x_320 : vec3<f32> = (vec3<f32>(x_316.x, x_316.x, x_316.x) * vec3<f32>(x_318.x, x_318.y, x_318.z));
  let x_321 : vec4<f32> = u_xlat6;
  u_xlat6 = vec4<f32>(x_320.x, x_320.y, x_320.z, x_321.w);
  let x_323 : vec4<f32> = u_xlat3;
  let x_325 : vec3<f32> = u_xlat34;
  let x_327 : vec3<f32> = (vec3<f32>(x_323.x, x_323.y, x_323.z) * vec3<f32>(x_325.y, x_325.y, x_325.y));
  let x_328 : vec4<f32> = u_xlat3;
  u_xlat3 = vec4<f32>(x_327.x, x_327.y, x_327.z, x_328.w);
  let x_330 : vec3<f32> = u_xlat34;
  let x_332 : vec4<f32> = u_xlat2;
  let x_335 : vec4<f32> = u_xlat3;
  let x_337 : vec3<f32> = ((vec3<f32>(x_330.x, x_330.x, x_330.x) * vec3<f32>(x_332.x, x_332.y, x_332.z)) + vec3<f32>(x_335.x, x_335.y, x_335.z));
  let x_338 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_337.x, x_337.y, x_337.z, x_338.w);
  let x_340 : vec3<f32> = u_xlat34;
  let x_342 : vec3<f32> = u_xlat1;
  let x_344 : vec4<f32> = u_xlat2;
  u_xlat1 = ((vec3<f32>(x_340.z, x_340.z, x_340.z) * x_342) + vec3<f32>(x_344.x, x_344.y, x_344.z));
  let x_347 : vec3<f32> = u_xlat1;
  let x_348 : vec3<f32> = u_xlat1;
  u_xlat0.x = dot(x_347, x_348);
  let x_352 : f32 = u_xlat0.x;
  u_xlat0.x = max(x_352, 1.17549435e-38f);
  let x_357 : f32 = u_xlat0.x;
  u_xlat0.x = inverseSqrt(x_357);
  let x_360 : vec3<f32> = u_xlat0;
  let x_362 : vec3<f32> = u_xlat1;
  u_xlat1 = (vec3<f32>(x_360.x, x_360.x, x_360.x) * x_362);
  let x_364 : vec3<f32> = vs_INTERP8;
  let x_374 : vec4<f32> = x_372.x_CascadeShadowSplitSpheres0;
  let x_377 : vec3<f32> = (x_364 + -(vec3<f32>(x_374.x, x_374.y, x_374.z)));
  let x_378 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_377.x, x_377.y, x_377.z, x_378.w);
  let x_380 : vec3<f32> = vs_INTERP8;
  let x_382 : vec4<f32> = x_372.x_CascadeShadowSplitSpheres1;
  let x_385 : vec3<f32> = (x_380 + -(vec3<f32>(x_382.x, x_382.y, x_382.z)));
  let x_386 : vec4<f32> = u_xlat3;
  u_xlat3 = vec4<f32>(x_385.x, x_385.y, x_385.z, x_386.w);
  let x_388 : vec3<f32> = vs_INTERP8;
  let x_390 : vec4<f32> = x_372.x_CascadeShadowSplitSpheres2;
  let x_393 : vec3<f32> = (x_388 + -(vec3<f32>(x_390.x, x_390.y, x_390.z)));
  let x_394 : vec4<f32> = u_xlat9;
  u_xlat9 = vec4<f32>(x_393.x, x_393.y, x_393.z, x_394.w);
  let x_397 : vec3<f32> = vs_INTERP8;
  let x_400 : vec4<f32> = x_372.x_CascadeShadowSplitSpheres3;
  let x_403 : vec3<f32> = (x_397 + -(vec3<f32>(x_400.x, x_400.y, x_400.z)));
  let x_404 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_403.x, x_403.y, x_403.z, x_404.w);
  let x_406 : vec4<f32> = u_xlat2;
  let x_408 : vec4<f32> = u_xlat2;
  u_xlat2.x = dot(vec3<f32>(x_406.x, x_406.y, x_406.z), vec3<f32>(x_408.x, x_408.y, x_408.z));
  let x_412 : vec4<f32> = u_xlat3;
  let x_414 : vec4<f32> = u_xlat3;
  u_xlat2.y = dot(vec3<f32>(x_412.x, x_412.y, x_412.z), vec3<f32>(x_414.x, x_414.y, x_414.z));
  let x_418 : vec4<f32> = u_xlat9;
  let x_420 : vec4<f32> = u_xlat9;
  u_xlat2.z = dot(vec3<f32>(x_418.x, x_418.y, x_418.z), vec3<f32>(x_420.x, x_420.y, x_420.z));
  let x_424 : vec4<f32> = u_xlat10;
  let x_426 : vec4<f32> = u_xlat10;
  u_xlat2.w = dot(vec3<f32>(x_424.x, x_424.y, x_424.z), vec3<f32>(x_426.x, x_426.y, x_426.z));
  let x_433 : vec4<f32> = u_xlat2;
  let x_435 : vec4<f32> = x_372.x_CascadeShadowSplitSphereRadii;
  u_xlatb2 = (x_433 < x_435);
  let x_438 : bool = u_xlatb2.x;
  u_xlat3.x = select(0.0f, 1.0f, x_438);
  let x_442 : bool = u_xlatb2.y;
  u_xlat3.y = select(0.0f, 1.0f, x_442);
  let x_446 : bool = u_xlatb2.z;
  u_xlat3.z = select(0.0f, 1.0f, x_446);
  let x_450 : bool = u_xlatb2.w;
  u_xlat3.w = select(0.0f, 1.0f, x_450);
  let x_454 : bool = u_xlatb2.x;
  u_xlat2.x = select(-0.0f, -1.0f, x_454);
  let x_459 : bool = u_xlatb2.y;
  u_xlat2.y = select(-0.0f, -1.0f, x_459);
  let x_463 : bool = u_xlatb2.z;
  u_xlat2.z = select(-0.0f, -1.0f, x_463);
  let x_466 : vec4<f32> = u_xlat2;
  let x_468 : vec4<f32> = u_xlat3;
  let x_470 : vec3<f32> = (vec3<f32>(x_466.x, x_466.y, x_466.z) + vec3<f32>(x_468.y, x_468.z, x_468.w));
  let x_471 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_470.x, x_470.y, x_470.z, x_471.w);
  let x_473 : vec4<f32> = u_xlat2;
  let x_476 : vec3<f32> = max(vec3<f32>(x_473.x, x_473.y, x_473.z), vec3<f32>(0.0f, 0.0f, 0.0f));
  let x_477 : vec4<f32> = u_xlat3;
  u_xlat3 = vec4<f32>(x_477.x, x_476.x, x_476.y, x_476.z);
  let x_479 : vec4<f32> = u_xlat3;
  u_xlat0.x = dot(x_479, vec4<f32>(4.0f, 3.0f, 2.0f, 1.0f));
  let x_486 : f32 = u_xlat0.x;
  u_xlat0.x = (-(x_486) + 4.0f);
  let x_493 : f32 = u_xlat0.x;
  u_xlatu0 = u32(x_493);
  let x_497 : u32 = u_xlatu0;
  u_xlati0 = (bitcast<i32>(x_497) << bitcast<u32>(2i));
  let x_500 : vec3<f32> = vs_INTERP8;
  let x_502 : i32 = u_xlati0;
  let x_505 : i32 = u_xlati0;
  let x_509 : vec4<f32> = x_372.x_MainLightWorldToShadow[((x_502 + 1i) / 4i)][((x_505 + 1i) % 4i)];
  let x_511 : vec3<f32> = (vec3<f32>(x_500.y, x_500.y, x_500.y) * vec3<f32>(x_509.x, x_509.y, x_509.z));
  let x_512 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_511.x, x_511.y, x_511.z, x_512.w);
  let x_514 : i32 = u_xlati0;
  let x_516 : i32 = u_xlati0;
  let x_519 : vec4<f32> = x_372.x_MainLightWorldToShadow[(x_514 / 4i)][(x_516 % 4i)];
  let x_521 : vec3<f32> = vs_INTERP8;
  let x_524 : vec4<f32> = u_xlat2;
  let x_526 : vec3<f32> = ((vec3<f32>(x_519.x, x_519.y, x_519.z) * vec3<f32>(x_521.x, x_521.x, x_521.x)) + vec3<f32>(x_524.x, x_524.y, x_524.z));
  let x_527 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_526.x, x_526.y, x_526.z, x_527.w);
  let x_529 : i32 = u_xlati0;
  let x_532 : i32 = u_xlati0;
  let x_536 : vec4<f32> = x_372.x_MainLightWorldToShadow[((x_529 + 2i) / 4i)][((x_532 + 2i) % 4i)];
  let x_538 : vec3<f32> = vs_INTERP8;
  let x_541 : vec4<f32> = u_xlat2;
  let x_543 : vec3<f32> = ((vec3<f32>(x_536.x, x_536.y, x_536.z) * vec3<f32>(x_538.z, x_538.z, x_538.z)) + vec3<f32>(x_541.x, x_541.y, x_541.z));
  let x_544 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_543.x, x_543.y, x_543.z, x_544.w);
  let x_546 : vec4<f32> = u_xlat2;
  let x_548 : i32 = u_xlati0;
  let x_551 : i32 = u_xlati0;
  let x_555 : vec4<f32> = x_372.x_MainLightWorldToShadow[((x_548 + 3i) / 4i)][((x_551 + 3i) % 4i)];
  let x_557 : vec3<f32> = (vec3<f32>(x_546.x, x_546.y, x_546.z) + vec3<f32>(x_555.x, x_555.y, x_555.z));
  let x_558 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_557.x, x_557.y, x_557.z, x_558.w);
  let x_561 : f32 = x_372.x_MainLightShadowParams.y;
  u_xlatb0 = (0.0f < x_561);
  let x_563 : bool = u_xlatb0;
  if (x_563) {
    let x_568 : f32 = x_372.x_MainLightShadowParams.y;
    u_xlatb79 = (x_568 == 1.0f);
    let x_570 : bool = u_xlatb79;
    if (x_570) {
      let x_573 : vec4<f32> = u_xlat2;
      let x_577 : vec4<f32> = x_372.x_MainLightShadowOffset0;
      u_xlat3 = (vec4<f32>(x_573.x, x_573.y, x_573.x, x_573.y) + x_577);
      let x_580 : vec4<f32> = u_xlat3;
      let x_581 : vec2<f32> = vec2<f32>(x_580.x, x_580.y);
      let x_583 : f32 = u_xlat2.z;
      txVec0 = vec3<f32>(x_581.x, x_581.y, x_583);
      let x_595 : vec3<f32> = txVec0;
      let x_597 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_595.xy, x_595.z);
      u_xlat9.x = x_597;
      let x_600 : vec4<f32> = u_xlat3;
      let x_601 : vec2<f32> = vec2<f32>(x_600.z, x_600.w);
      let x_603 : f32 = u_xlat2.z;
      txVec1 = vec3<f32>(x_601.x, x_601.y, x_603);
      let x_610 : vec3<f32> = txVec1;
      let x_612 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_610.xy, x_610.z);
      u_xlat9.y = x_612;
      let x_614 : vec4<f32> = u_xlat2;
      let x_618 : vec4<f32> = x_372.x_MainLightShadowOffset1;
      u_xlat3 = (vec4<f32>(x_614.x, x_614.y, x_614.x, x_614.y) + x_618);
      let x_621 : vec4<f32> = u_xlat3;
      let x_622 : vec2<f32> = vec2<f32>(x_621.x, x_621.y);
      let x_624 : f32 = u_xlat2.z;
      txVec2 = vec3<f32>(x_622.x, x_622.y, x_624);
      let x_631 : vec3<f32> = txVec2;
      let x_633 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_631.xy, x_631.z);
      u_xlat9.z = x_633;
      let x_636 : vec4<f32> = u_xlat3;
      let x_637 : vec2<f32> = vec2<f32>(x_636.z, x_636.w);
      let x_639 : f32 = u_xlat2.z;
      txVec3 = vec3<f32>(x_637.x, x_637.y, x_639);
      let x_646 : vec3<f32> = txVec3;
      let x_648 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_646.xy, x_646.z);
      u_xlat9.w = x_648;
      let x_650 : vec4<f32> = u_xlat9;
      u_xlat79 = dot(x_650, vec4<f32>(0.25f, 0.25f, 0.25f, 0.25f));
    } else {
      let x_657 : f32 = x_372.x_MainLightShadowParams.y;
      u_xlatb80 = (x_657 == 2.0f);
      let x_659 : bool = u_xlatb80;
      if (x_659) {
        let x_662 : vec4<f32> = u_xlat2;
        let x_666 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_671 : vec2<f32> = ((vec2<f32>(x_662.x, x_662.y) * vec2<f32>(x_666.z, x_666.w)) + vec2<f32>(0.5f, 0.5f));
        let x_672 : vec4<f32> = u_xlat3;
        u_xlat3 = vec4<f32>(x_671.x, x_671.y, x_672.z, x_672.w);
        let x_674 : vec4<f32> = u_xlat3;
        let x_676 : vec2<f32> = floor(vec2<f32>(x_674.x, x_674.y));
        let x_677 : vec4<f32> = u_xlat3;
        u_xlat3 = vec4<f32>(x_676.x, x_676.y, x_677.z, x_677.w);
        let x_681 : vec4<f32> = u_xlat2;
        let x_684 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_687 : vec4<f32> = u_xlat3;
        u_xlat55 = ((vec2<f32>(x_681.x, x_681.y) * vec2<f32>(x_684.z, x_684.w)) + -(vec2<f32>(x_687.x, x_687.y)));
        let x_691 : vec2<f32> = u_xlat55;
        u_xlat9 = (vec4<f32>(x_691.x, x_691.x, x_691.y, x_691.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        let x_695 : vec4<f32> = u_xlat9;
        let x_697 : vec4<f32> = u_xlat9;
        u_xlat10 = (vec4<f32>(x_695.x, x_695.x, x_695.z, x_695.z) * vec4<f32>(x_697.x, x_697.x, x_697.z, x_697.z));
        let x_700 : vec4<f32> = u_xlat10;
        let x_704 : vec2<f32> = (vec2<f32>(x_700.y, x_700.w) * vec2<f32>(0.07999999821186065674f, 0.07999999821186065674f));
        let x_705 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_704.x, x_705.y, x_704.y, x_705.w);
        let x_707 : vec4<f32> = u_xlat10;
        let x_710 : vec2<f32> = u_xlat55;
        let x_712 : vec2<f32> = ((vec2<f32>(x_707.x, x_707.z) * vec2<f32>(0.5f, 0.5f)) + -(x_710));
        let x_713 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_712.x, x_712.y, x_713.z, x_713.w);
        let x_716 : vec2<f32> = u_xlat55;
        u_xlat62 = (-(x_716) + vec2<f32>(1.0f, 1.0f));
        let x_721 : vec2<f32> = u_xlat55;
        let x_723 : vec2<f32> = min(x_721, vec2<f32>(0.0f, 0.0f));
        let x_724 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_723.x, x_723.y, x_724.z, x_724.w);
        let x_726 : vec4<f32> = u_xlat11;
        let x_729 : vec4<f32> = u_xlat11;
        let x_732 : vec2<f32> = u_xlat62;
        let x_733 : vec2<f32> = ((-(vec2<f32>(x_726.x, x_726.y)) * vec2<f32>(x_729.x, x_729.y)) + x_732);
        let x_734 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_733.x, x_733.y, x_734.z, x_734.w);
        let x_736 : vec2<f32> = u_xlat55;
        u_xlat55 = max(x_736, vec2<f32>(0.0f, 0.0f));
        let x_738 : vec2<f32> = u_xlat55;
        let x_740 : vec2<f32> = u_xlat55;
        let x_742 : vec4<f32> = u_xlat9;
        u_xlat55 = ((-(x_738) * x_740) + vec2<f32>(x_742.y, x_742.w));
        let x_745 : vec4<f32> = u_xlat11;
        let x_747 : vec2<f32> = (vec2<f32>(x_745.x, x_745.y) + vec2<f32>(1.0f, 1.0f));
        let x_748 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_747.x, x_747.y, x_748.z, x_748.w);
        let x_750 : vec2<f32> = u_xlat55;
        u_xlat55 = (x_750 + vec2<f32>(1.0f, 1.0f));
        let x_753 : vec4<f32> = u_xlat10;
        let x_757 : vec2<f32> = (vec2<f32>(x_753.x, x_753.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_758 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_757.x, x_757.y, x_758.z, x_758.w);
        let x_760 : vec2<f32> = u_xlat62;
        let x_761 : vec2<f32> = (x_760 * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_762 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_761.x, x_761.y, x_762.z, x_762.w);
        let x_764 : vec4<f32> = u_xlat11;
        let x_766 : vec2<f32> = (vec2<f32>(x_764.x, x_764.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_767 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_766.x, x_766.y, x_767.z, x_767.w);
        let x_770 : vec2<f32> = u_xlat55;
        let x_771 : vec2<f32> = (x_770 * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_772 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_771.x, x_771.y, x_772.z, x_772.w);
        let x_774 : vec4<f32> = u_xlat9;
        u_xlat55 = (vec2<f32>(x_774.y, x_774.w) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_778 : f32 = u_xlat11.x;
        u_xlat12.z = x_778;
        let x_781 : f32 = u_xlat55.x;
        u_xlat12.w = x_781;
        let x_784 : f32 = u_xlat13.x;
        u_xlat10.z = x_784;
        let x_787 : f32 = u_xlat9.x;
        u_xlat10.w = x_787;
        let x_790 : vec4<f32> = u_xlat10;
        let x_792 : vec4<f32> = u_xlat12;
        u_xlat14 = (vec4<f32>(x_790.z, x_790.w, x_790.x, x_790.z) + vec4<f32>(x_792.z, x_792.w, x_792.x, x_792.z));
        let x_796 : f32 = u_xlat12.y;
        u_xlat11.z = x_796;
        let x_799 : f32 = u_xlat55.y;
        u_xlat11.w = x_799;
        let x_802 : f32 = u_xlat10.y;
        u_xlat13.z = x_802;
        let x_805 : f32 = u_xlat9.z;
        u_xlat13.w = x_805;
        let x_807 : vec4<f32> = u_xlat11;
        let x_809 : vec4<f32> = u_xlat13;
        let x_811 : vec3<f32> = (vec3<f32>(x_807.z, x_807.y, x_807.w) + vec3<f32>(x_809.z, x_809.y, x_809.w));
        let x_812 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_811.x, x_811.y, x_811.z, x_812.w);
        let x_814 : vec4<f32> = u_xlat10;
        let x_816 : vec4<f32> = u_xlat14;
        let x_818 : vec3<f32> = (vec3<f32>(x_814.x, x_814.z, x_814.w) / vec3<f32>(x_816.z, x_816.w, x_816.y));
        let x_819 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_818.x, x_818.y, x_818.z, x_819.w);
        let x_821 : vec4<f32> = u_xlat10;
        let x_827 : vec3<f32> = (vec3<f32>(x_821.x, x_821.y, x_821.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
        let x_828 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_827.x, x_827.y, x_827.z, x_828.w);
        let x_830 : vec4<f32> = u_xlat13;
        let x_832 : vec4<f32> = u_xlat9;
        let x_834 : vec3<f32> = (vec3<f32>(x_830.z, x_830.y, x_830.w) / vec3<f32>(x_832.x, x_832.y, x_832.z));
        let x_835 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_834.x, x_834.y, x_834.z, x_835.w);
        let x_837 : vec4<f32> = u_xlat11;
        let x_839 : vec3<f32> = (vec3<f32>(x_837.x, x_837.y, x_837.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
        let x_840 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_839.x, x_839.y, x_839.z, x_840.w);
        let x_842 : vec4<f32> = u_xlat10;
        let x_845 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_847 : vec3<f32> = (vec3<f32>(x_842.y, x_842.x, x_842.z) * vec3<f32>(x_845.x, x_845.x, x_845.x));
        let x_848 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_847.x, x_847.y, x_847.z, x_848.w);
        let x_850 : vec4<f32> = u_xlat11;
        let x_853 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_855 : vec3<f32> = (vec3<f32>(x_850.x, x_850.y, x_850.z) * vec3<f32>(x_853.y, x_853.y, x_853.y));
        let x_856 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_855.x, x_855.y, x_855.z, x_856.w);
        let x_859 : f32 = u_xlat11.x;
        u_xlat10.w = x_859;
        let x_861 : vec4<f32> = u_xlat3;
        let x_864 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_867 : vec4<f32> = u_xlat10;
        u_xlat12 = ((vec4<f32>(x_861.x, x_861.y, x_861.x, x_861.y) * vec4<f32>(x_864.x, x_864.y, x_864.x, x_864.y)) + vec4<f32>(x_867.y, x_867.w, x_867.x, x_867.w));
        let x_870 : vec4<f32> = u_xlat3;
        let x_873 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_876 : vec4<f32> = u_xlat10;
        u_xlat55 = ((vec2<f32>(x_870.x, x_870.y) * vec2<f32>(x_873.x, x_873.y)) + vec2<f32>(x_876.z, x_876.w));
        let x_880 : f32 = u_xlat10.y;
        u_xlat11.w = x_880;
        let x_882 : vec4<f32> = u_xlat11;
        let x_883 : vec2<f32> = vec2<f32>(x_882.y, x_882.z);
        let x_884 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_884.x, x_883.x, x_884.z, x_883.y);
        let x_886 : vec4<f32> = u_xlat3;
        let x_889 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_892 : vec4<f32> = u_xlat10;
        u_xlat13 = ((vec4<f32>(x_886.x, x_886.y, x_886.x, x_886.y) * vec4<f32>(x_889.x, x_889.y, x_889.x, x_889.y)) + vec4<f32>(x_892.x, x_892.y, x_892.z, x_892.y));
        let x_895 : vec4<f32> = u_xlat3;
        let x_898 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_901 : vec4<f32> = u_xlat11;
        u_xlat11 = ((vec4<f32>(x_895.x, x_895.y, x_895.x, x_895.y) * vec4<f32>(x_898.x, x_898.y, x_898.x, x_898.y)) + vec4<f32>(x_901.w, x_901.y, x_901.w, x_901.z));
        let x_904 : vec4<f32> = u_xlat3;
        let x_907 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_910 : vec4<f32> = u_xlat10;
        u_xlat10 = ((vec4<f32>(x_904.x, x_904.y, x_904.x, x_904.y) * vec4<f32>(x_907.x, x_907.y, x_907.x, x_907.y)) + vec4<f32>(x_910.x, x_910.w, x_910.z, x_910.w));
        let x_914 : vec4<f32> = u_xlat9;
        let x_916 : vec4<f32> = u_xlat14;
        u_xlat15 = (vec4<f32>(x_914.x, x_914.x, x_914.x, x_914.y) * vec4<f32>(x_916.z, x_916.w, x_916.y, x_916.z));
        let x_920 : vec4<f32> = u_xlat9;
        let x_922 : vec4<f32> = u_xlat14;
        u_xlat16 = (vec4<f32>(x_920.y, x_920.y, x_920.z, x_920.z) * x_922);
        let x_926 : f32 = u_xlat9.z;
        let x_928 : f32 = u_xlat14.y;
        u_xlat80 = (x_926 * x_928);
        let x_931 : vec4<f32> = u_xlat12;
        let x_932 : vec2<f32> = vec2<f32>(x_931.x, x_931.y);
        let x_934 : f32 = u_xlat2.z;
        txVec4 = vec3<f32>(x_932.x, x_932.y, x_934);
        let x_941 : vec3<f32> = txVec4;
        let x_943 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_941.xy, x_941.z);
        u_xlat3.x = x_943;
        let x_946 : vec4<f32> = u_xlat12;
        let x_947 : vec2<f32> = vec2<f32>(x_946.z, x_946.w);
        let x_949 : f32 = u_xlat2.z;
        txVec5 = vec3<f32>(x_947.x, x_947.y, x_949);
        let x_957 : vec3<f32> = txVec5;
        let x_959 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_957.xy, x_957.z);
        u_xlat29 = x_959;
        let x_960 : f32 = u_xlat29;
        let x_962 : f32 = u_xlat15.y;
        u_xlat29 = (x_960 * x_962);
        let x_965 : f32 = u_xlat15.x;
        let x_967 : f32 = u_xlat3.x;
        let x_969 : f32 = u_xlat29;
        u_xlat3.x = ((x_965 * x_967) + x_969);
        let x_973 : vec2<f32> = u_xlat55;
        let x_975 : f32 = u_xlat2.z;
        txVec6 = vec3<f32>(x_973.x, x_973.y, x_975);
        let x_982 : vec3<f32> = txVec6;
        let x_984 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_982.xy, x_982.z);
        u_xlat29 = x_984;
        let x_986 : f32 = u_xlat15.z;
        let x_987 : f32 = u_xlat29;
        let x_990 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_986 * x_987) + x_990);
        let x_994 : vec4<f32> = u_xlat11;
        let x_995 : vec2<f32> = vec2<f32>(x_994.x, x_994.y);
        let x_997 : f32 = u_xlat2.z;
        txVec7 = vec3<f32>(x_995.x, x_995.y, x_997);
        let x_1004 : vec3<f32> = txVec7;
        let x_1006 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1004.xy, x_1004.z);
        u_xlat29 = x_1006;
        let x_1008 : f32 = u_xlat15.w;
        let x_1009 : f32 = u_xlat29;
        let x_1012 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_1008 * x_1009) + x_1012);
        let x_1016 : vec4<f32> = u_xlat13;
        let x_1017 : vec2<f32> = vec2<f32>(x_1016.x, x_1016.y);
        let x_1019 : f32 = u_xlat2.z;
        txVec8 = vec3<f32>(x_1017.x, x_1017.y, x_1019);
        let x_1026 : vec3<f32> = txVec8;
        let x_1028 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1026.xy, x_1026.z);
        u_xlat29 = x_1028;
        let x_1030 : f32 = u_xlat16.x;
        let x_1031 : f32 = u_xlat29;
        let x_1034 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_1030 * x_1031) + x_1034);
        let x_1038 : vec4<f32> = u_xlat13;
        let x_1039 : vec2<f32> = vec2<f32>(x_1038.z, x_1038.w);
        let x_1041 : f32 = u_xlat2.z;
        txVec9 = vec3<f32>(x_1039.x, x_1039.y, x_1041);
        let x_1048 : vec3<f32> = txVec9;
        let x_1050 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1048.xy, x_1048.z);
        u_xlat29 = x_1050;
        let x_1052 : f32 = u_xlat16.y;
        let x_1053 : f32 = u_xlat29;
        let x_1056 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_1052 * x_1053) + x_1056);
        let x_1060 : vec4<f32> = u_xlat11;
        let x_1061 : vec2<f32> = vec2<f32>(x_1060.z, x_1060.w);
        let x_1063 : f32 = u_xlat2.z;
        txVec10 = vec3<f32>(x_1061.x, x_1061.y, x_1063);
        let x_1070 : vec3<f32> = txVec10;
        let x_1072 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1070.xy, x_1070.z);
        u_xlat29 = x_1072;
        let x_1074 : f32 = u_xlat16.z;
        let x_1075 : f32 = u_xlat29;
        let x_1078 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_1074 * x_1075) + x_1078);
        let x_1082 : vec4<f32> = u_xlat10;
        let x_1083 : vec2<f32> = vec2<f32>(x_1082.x, x_1082.y);
        let x_1085 : f32 = u_xlat2.z;
        txVec11 = vec3<f32>(x_1083.x, x_1083.y, x_1085);
        let x_1092 : vec3<f32> = txVec11;
        let x_1094 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1092.xy, x_1092.z);
        u_xlat29 = x_1094;
        let x_1096 : f32 = u_xlat16.w;
        let x_1097 : f32 = u_xlat29;
        let x_1100 : f32 = u_xlat3.x;
        u_xlat3.x = ((x_1096 * x_1097) + x_1100);
        let x_1104 : vec4<f32> = u_xlat10;
        let x_1105 : vec2<f32> = vec2<f32>(x_1104.z, x_1104.w);
        let x_1107 : f32 = u_xlat2.z;
        txVec12 = vec3<f32>(x_1105.x, x_1105.y, x_1107);
        let x_1114 : vec3<f32> = txVec12;
        let x_1116 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1114.xy, x_1114.z);
        u_xlat29 = x_1116;
        let x_1117 : f32 = u_xlat80;
        let x_1118 : f32 = u_xlat29;
        let x_1121 : f32 = u_xlat3.x;
        u_xlat79 = ((x_1117 * x_1118) + x_1121);
      } else {
        let x_1124 : vec4<f32> = u_xlat2;
        let x_1127 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1130 : vec2<f32> = ((vec2<f32>(x_1124.x, x_1124.y) * vec2<f32>(x_1127.z, x_1127.w)) + vec2<f32>(0.5f, 0.5f));
        let x_1131 : vec4<f32> = u_xlat3;
        u_xlat3 = vec4<f32>(x_1130.x, x_1130.y, x_1131.z, x_1131.w);
        let x_1133 : vec4<f32> = u_xlat3;
        let x_1135 : vec2<f32> = floor(vec2<f32>(x_1133.x, x_1133.y));
        let x_1136 : vec4<f32> = u_xlat3;
        u_xlat3 = vec4<f32>(x_1135.x, x_1135.y, x_1136.z, x_1136.w);
        let x_1138 : vec4<f32> = u_xlat2;
        let x_1141 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1144 : vec4<f32> = u_xlat3;
        u_xlat55 = ((vec2<f32>(x_1138.x, x_1138.y) * vec2<f32>(x_1141.z, x_1141.w)) + -(vec2<f32>(x_1144.x, x_1144.y)));
        let x_1148 : vec2<f32> = u_xlat55;
        u_xlat9 = (vec4<f32>(x_1148.x, x_1148.x, x_1148.y, x_1148.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        let x_1151 : vec4<f32> = u_xlat9;
        let x_1153 : vec4<f32> = u_xlat9;
        u_xlat10 = (vec4<f32>(x_1151.x, x_1151.x, x_1151.z, x_1151.z) * vec4<f32>(x_1153.x, x_1153.x, x_1153.z, x_1153.z));
        let x_1156 : vec4<f32> = u_xlat10;
        let x_1160 : vec2<f32> = (vec2<f32>(x_1156.y, x_1156.w) * vec2<f32>(0.04081600159406661987f, 0.04081600159406661987f));
        let x_1161 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_1161.x, x_1160.x, x_1161.z, x_1160.y);
        let x_1163 : vec4<f32> = u_xlat10;
        let x_1166 : vec2<f32> = u_xlat55;
        let x_1168 : vec2<f32> = ((vec2<f32>(x_1163.x, x_1163.z) * vec2<f32>(0.5f, 0.5f)) + -(x_1166));
        let x_1169 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_1168.x, x_1169.y, x_1168.y, x_1169.w);
        let x_1171 : vec2<f32> = u_xlat55;
        let x_1173 : vec2<f32> = (-(x_1171) + vec2<f32>(1.0f, 1.0f));
        let x_1174 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_1173.x, x_1173.y, x_1174.z, x_1174.w);
        let x_1176 : vec2<f32> = u_xlat55;
        u_xlat62 = min(x_1176, vec2<f32>(0.0f, 0.0f));
        let x_1178 : vec2<f32> = u_xlat62;
        let x_1180 : vec2<f32> = u_xlat62;
        let x_1182 : vec4<f32> = u_xlat10;
        let x_1184 : vec2<f32> = ((-(x_1178) * x_1180) + vec2<f32>(x_1182.x, x_1182.y));
        let x_1185 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_1184.x, x_1184.y, x_1185.z, x_1185.w);
        let x_1187 : vec2<f32> = u_xlat55;
        u_xlat62 = max(x_1187, vec2<f32>(0.0f, 0.0f));
        let x_1190 : vec2<f32> = u_xlat62;
        let x_1192 : vec2<f32> = u_xlat62;
        let x_1194 : vec4<f32> = u_xlat9;
        let x_1196 : vec2<f32> = ((-(x_1190) * x_1192) + vec2<f32>(x_1194.y, x_1194.w));
        let x_1197 : vec3<f32> = u_xlat35;
        u_xlat35 = vec3<f32>(x_1196.x, x_1197.y, x_1196.y);
        let x_1199 : vec4<f32> = u_xlat10;
        let x_1202 : vec2<f32> = (vec2<f32>(x_1199.x, x_1199.y) + vec2<f32>(2.0f, 2.0f));
        let x_1203 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_1202.x, x_1202.y, x_1203.z, x_1203.w);
        let x_1205 : vec3<f32> = u_xlat35;
        let x_1207 : vec2<f32> = (vec2<f32>(x_1205.x, x_1205.z) + vec2<f32>(2.0f, 2.0f));
        let x_1208 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_1208.x, x_1207.x, x_1208.z, x_1207.y);
        let x_1211 : f32 = u_xlat9.y;
        u_xlat12.z = (x_1211 * 0.08163200318813323975f);
        let x_1215 : vec4<f32> = u_xlat9;
        let x_1218 : vec3<f32> = (vec3<f32>(x_1215.z, x_1215.x, x_1215.w) * vec3<f32>(0.08163200318813323975f, 0.08163200318813323975f, 0.08163200318813323975f));
        let x_1219 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_1218.x, x_1218.y, x_1218.z, x_1219.w);
        let x_1221 : vec4<f32> = u_xlat10;
        let x_1224 : vec2<f32> = (vec2<f32>(x_1221.x, x_1221.y) * vec2<f32>(0.08163200318813323975f, 0.08163200318813323975f));
        let x_1225 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_1224.x, x_1224.y, x_1225.z, x_1225.w);
        let x_1228 : f32 = u_xlat13.y;
        u_xlat12.x = x_1228;
        let x_1230 : vec2<f32> = u_xlat55;
        let x_1237 : vec2<f32> = ((vec2<f32>(x_1230.x, x_1230.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let x_1238 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_1238.x, x_1237.x, x_1238.z, x_1237.y);
        let x_1240 : vec2<f32> = u_xlat55;
        let x_1244 : vec2<f32> = ((vec2<f32>(x_1240.x, x_1240.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let x_1245 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_1244.x, x_1245.y, x_1244.y, x_1245.w);
        let x_1248 : f32 = u_xlat9.x;
        u_xlat10.y = x_1248;
        let x_1251 : f32 = u_xlat11.y;
        u_xlat10.w = x_1251;
        let x_1253 : vec4<f32> = u_xlat10;
        let x_1254 : vec4<f32> = u_xlat12;
        u_xlat12 = (x_1253 + x_1254);
        let x_1256 : vec2<f32> = u_xlat55;
        let x_1259 : vec2<f32> = ((vec2<f32>(x_1256.y, x_1256.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let x_1260 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_1260.x, x_1259.x, x_1260.z, x_1259.y);
        let x_1262 : vec2<f32> = u_xlat55;
        let x_1265 : vec2<f32> = ((vec2<f32>(x_1262.y, x_1262.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let x_1266 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_1265.x, x_1266.y, x_1265.y, x_1266.w);
        let x_1269 : f32 = u_xlat9.y;
        u_xlat11.y = x_1269;
        let x_1271 : vec4<f32> = u_xlat11;
        let x_1272 : vec4<f32> = u_xlat13;
        u_xlat9 = (x_1271 + x_1272);
        let x_1274 : vec4<f32> = u_xlat10;
        let x_1275 : vec4<f32> = u_xlat12;
        u_xlat10 = (x_1274 / x_1275);
        let x_1277 : vec4<f32> = u_xlat10;
        u_xlat10 = (x_1277 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        let x_1283 : vec4<f32> = u_xlat11;
        let x_1284 : vec4<f32> = u_xlat9;
        u_xlat11 = (x_1283 / x_1284);
        let x_1286 : vec4<f32> = u_xlat11;
        u_xlat11 = (x_1286 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        let x_1288 : vec4<f32> = u_xlat10;
        let x_1291 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat10 = (vec4<f32>(x_1288.w, x_1288.x, x_1288.y, x_1288.z) * vec4<f32>(x_1291.x, x_1291.x, x_1291.x, x_1291.x));
        let x_1294 : vec4<f32> = u_xlat11;
        let x_1297 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat11 = (vec4<f32>(x_1294.x, x_1294.w, x_1294.y, x_1294.z) * vec4<f32>(x_1297.y, x_1297.y, x_1297.y, x_1297.y));
        let x_1300 : vec4<f32> = u_xlat10;
        let x_1301 : vec3<f32> = vec3<f32>(x_1300.y, x_1300.z, x_1300.w);
        let x_1302 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_1301.x, x_1302.y, x_1301.y, x_1301.z);
        let x_1305 : f32 = u_xlat11.x;
        u_xlat13.y = x_1305;
        let x_1307 : vec4<f32> = u_xlat3;
        let x_1310 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1313 : vec4<f32> = u_xlat13;
        u_xlat14 = ((vec4<f32>(x_1307.x, x_1307.y, x_1307.x, x_1307.y) * vec4<f32>(x_1310.x, x_1310.y, x_1310.x, x_1310.y)) + vec4<f32>(x_1313.x, x_1313.y, x_1313.z, x_1313.y));
        let x_1316 : vec4<f32> = u_xlat3;
        let x_1319 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1322 : vec4<f32> = u_xlat13;
        u_xlat55 = ((vec2<f32>(x_1316.x, x_1316.y) * vec2<f32>(x_1319.x, x_1319.y)) + vec2<f32>(x_1322.w, x_1322.y));
        let x_1326 : f32 = u_xlat13.y;
        u_xlat10.y = x_1326;
        let x_1329 : f32 = u_xlat11.z;
        u_xlat13.y = x_1329;
        let x_1331 : vec4<f32> = u_xlat3;
        let x_1334 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1337 : vec4<f32> = u_xlat13;
        u_xlat15 = ((vec4<f32>(x_1331.x, x_1331.y, x_1331.x, x_1331.y) * vec4<f32>(x_1334.x, x_1334.y, x_1334.x, x_1334.y)) + vec4<f32>(x_1337.x, x_1337.y, x_1337.z, x_1337.y));
        let x_1340 : vec4<f32> = u_xlat3;
        let x_1343 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1346 : vec4<f32> = u_xlat13;
        let x_1348 : vec2<f32> = ((vec2<f32>(x_1340.x, x_1340.y) * vec2<f32>(x_1343.x, x_1343.y)) + vec2<f32>(x_1346.w, x_1346.y));
        let x_1349 : vec4<f32> = u_xlat16;
        u_xlat16 = vec4<f32>(x_1348.x, x_1348.y, x_1349.z, x_1349.w);
        let x_1352 : f32 = u_xlat13.y;
        u_xlat10.z = x_1352;
        let x_1355 : vec4<f32> = u_xlat3;
        let x_1358 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1361 : vec4<f32> = u_xlat10;
        u_xlat17 = ((vec4<f32>(x_1355.x, x_1355.y, x_1355.x, x_1355.y) * vec4<f32>(x_1358.x, x_1358.y, x_1358.x, x_1358.y)) + vec4<f32>(x_1361.x, x_1361.y, x_1361.x, x_1361.z));
        let x_1365 : f32 = u_xlat11.w;
        u_xlat13.y = x_1365;
        let x_1368 : vec4<f32> = u_xlat3;
        let x_1371 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1374 : vec4<f32> = u_xlat13;
        u_xlat18 = ((vec4<f32>(x_1368.x, x_1368.y, x_1368.x, x_1368.y) * vec4<f32>(x_1371.x, x_1371.y, x_1371.x, x_1371.y)) + vec4<f32>(x_1374.x, x_1374.y, x_1374.z, x_1374.y));
        let x_1378 : vec4<f32> = u_xlat3;
        let x_1381 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1384 : vec4<f32> = u_xlat13;
        u_xlat36 = ((vec2<f32>(x_1378.x, x_1378.y) * vec2<f32>(x_1381.x, x_1381.y)) + vec2<f32>(x_1384.w, x_1384.y));
        let x_1388 : f32 = u_xlat13.y;
        u_xlat10.w = x_1388;
        let x_1391 : vec4<f32> = u_xlat3;
        let x_1394 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1397 : vec4<f32> = u_xlat10;
        u_xlat68 = ((vec2<f32>(x_1391.x, x_1391.y) * vec2<f32>(x_1394.x, x_1394.y)) + vec2<f32>(x_1397.x, x_1397.w));
        let x_1400 : vec4<f32> = u_xlat13;
        let x_1401 : vec3<f32> = vec3<f32>(x_1400.x, x_1400.z, x_1400.w);
        let x_1402 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_1401.x, x_1402.y, x_1401.y, x_1401.z);
        let x_1404 : vec4<f32> = u_xlat3;
        let x_1407 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1410 : vec4<f32> = u_xlat11;
        u_xlat13 = ((vec4<f32>(x_1404.x, x_1404.y, x_1404.x, x_1404.y) * vec4<f32>(x_1407.x, x_1407.y, x_1407.x, x_1407.y)) + vec4<f32>(x_1410.x, x_1410.y, x_1410.z, x_1410.y));
        let x_1414 : vec4<f32> = u_xlat3;
        let x_1417 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1420 : vec4<f32> = u_xlat11;
        u_xlat63 = ((vec2<f32>(x_1414.x, x_1414.y) * vec2<f32>(x_1417.x, x_1417.y)) + vec2<f32>(x_1420.w, x_1420.y));
        let x_1424 : f32 = u_xlat10.x;
        u_xlat11.x = x_1424;
        let x_1426 : vec4<f32> = u_xlat3;
        let x_1429 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_1432 : vec4<f32> = u_xlat11;
        let x_1434 : vec2<f32> = ((vec2<f32>(x_1426.x, x_1426.y) * vec2<f32>(x_1429.x, x_1429.y)) + vec2<f32>(x_1432.x, x_1432.y));
        let x_1435 : vec4<f32> = u_xlat3;
        u_xlat3 = vec4<f32>(x_1434.x, x_1434.y, x_1435.z, x_1435.w);
        let x_1438 : vec4<f32> = u_xlat9;
        let x_1440 : vec4<f32> = u_xlat12;
        u_xlat19 = (vec4<f32>(x_1438.x, x_1438.x, x_1438.x, x_1438.x) * x_1440);
        let x_1443 : vec4<f32> = u_xlat9;
        let x_1445 : vec4<f32> = u_xlat12;
        u_xlat20 = (vec4<f32>(x_1443.y, x_1443.y, x_1443.y, x_1443.y) * x_1445);
        let x_1448 : vec4<f32> = u_xlat9;
        let x_1450 : vec4<f32> = u_xlat12;
        u_xlat21 = (vec4<f32>(x_1448.z, x_1448.z, x_1448.z, x_1448.z) * x_1450);
        let x_1452 : vec4<f32> = u_xlat9;
        let x_1454 : vec4<f32> = u_xlat12;
        u_xlat9 = (vec4<f32>(x_1452.w, x_1452.w, x_1452.w, x_1452.w) * x_1454);
        let x_1457 : vec4<f32> = u_xlat14;
        let x_1458 : vec2<f32> = vec2<f32>(x_1457.x, x_1457.y);
        let x_1460 : f32 = u_xlat2.z;
        txVec13 = vec3<f32>(x_1458.x, x_1458.y, x_1460);
        let x_1467 : vec3<f32> = txVec13;
        let x_1469 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1467.xy, x_1467.z);
        u_xlat80 = x_1469;
        let x_1471 : vec4<f32> = u_xlat14;
        let x_1472 : vec2<f32> = vec2<f32>(x_1471.z, x_1471.w);
        let x_1474 : f32 = u_xlat2.z;
        txVec14 = vec3<f32>(x_1472.x, x_1472.y, x_1474);
        let x_1482 : vec3<f32> = txVec14;
        let x_1484 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1482.xy, x_1482.z);
        u_xlat82 = x_1484;
        let x_1485 : f32 = u_xlat82;
        let x_1487 : f32 = u_xlat19.y;
        u_xlat82 = (x_1485 * x_1487);
        let x_1490 : f32 = u_xlat19.x;
        let x_1491 : f32 = u_xlat80;
        let x_1493 : f32 = u_xlat82;
        u_xlat80 = ((x_1490 * x_1491) + x_1493);
        let x_1496 : vec2<f32> = u_xlat55;
        let x_1498 : f32 = u_xlat2.z;
        txVec15 = vec3<f32>(x_1496.x, x_1496.y, x_1498);
        let x_1505 : vec3<f32> = txVec15;
        let x_1507 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1505.xy, x_1505.z);
        u_xlat55.x = x_1507;
        let x_1510 : f32 = u_xlat19.z;
        let x_1512 : f32 = u_xlat55.x;
        let x_1514 : f32 = u_xlat80;
        u_xlat80 = ((x_1510 * x_1512) + x_1514);
        let x_1517 : vec4<f32> = u_xlat17;
        let x_1518 : vec2<f32> = vec2<f32>(x_1517.x, x_1517.y);
        let x_1520 : f32 = u_xlat2.z;
        txVec16 = vec3<f32>(x_1518.x, x_1518.y, x_1520);
        let x_1527 : vec3<f32> = txVec16;
        let x_1529 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1527.xy, x_1527.z);
        u_xlat55.x = x_1529;
        let x_1532 : f32 = u_xlat19.w;
        let x_1534 : f32 = u_xlat55.x;
        let x_1536 : f32 = u_xlat80;
        u_xlat80 = ((x_1532 * x_1534) + x_1536);
        let x_1539 : vec4<f32> = u_xlat15;
        let x_1540 : vec2<f32> = vec2<f32>(x_1539.x, x_1539.y);
        let x_1542 : f32 = u_xlat2.z;
        txVec17 = vec3<f32>(x_1540.x, x_1540.y, x_1542);
        let x_1549 : vec3<f32> = txVec17;
        let x_1551 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1549.xy, x_1549.z);
        u_xlat55.x = x_1551;
        let x_1554 : f32 = u_xlat20.x;
        let x_1556 : f32 = u_xlat55.x;
        let x_1558 : f32 = u_xlat80;
        u_xlat80 = ((x_1554 * x_1556) + x_1558);
        let x_1561 : vec4<f32> = u_xlat15;
        let x_1562 : vec2<f32> = vec2<f32>(x_1561.z, x_1561.w);
        let x_1564 : f32 = u_xlat2.z;
        txVec18 = vec3<f32>(x_1562.x, x_1562.y, x_1564);
        let x_1571 : vec3<f32> = txVec18;
        let x_1573 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1571.xy, x_1571.z);
        u_xlat55.x = x_1573;
        let x_1576 : f32 = u_xlat20.y;
        let x_1578 : f32 = u_xlat55.x;
        let x_1580 : f32 = u_xlat80;
        u_xlat80 = ((x_1576 * x_1578) + x_1580);
        let x_1583 : vec4<f32> = u_xlat16;
        let x_1584 : vec2<f32> = vec2<f32>(x_1583.x, x_1583.y);
        let x_1586 : f32 = u_xlat2.z;
        txVec19 = vec3<f32>(x_1584.x, x_1584.y, x_1586);
        let x_1593 : vec3<f32> = txVec19;
        let x_1595 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1593.xy, x_1593.z);
        u_xlat55.x = x_1595;
        let x_1598 : f32 = u_xlat20.z;
        let x_1600 : f32 = u_xlat55.x;
        let x_1602 : f32 = u_xlat80;
        u_xlat80 = ((x_1598 * x_1600) + x_1602);
        let x_1605 : vec4<f32> = u_xlat17;
        let x_1606 : vec2<f32> = vec2<f32>(x_1605.z, x_1605.w);
        let x_1608 : f32 = u_xlat2.z;
        txVec20 = vec3<f32>(x_1606.x, x_1606.y, x_1608);
        let x_1615 : vec3<f32> = txVec20;
        let x_1617 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1615.xy, x_1615.z);
        u_xlat55.x = x_1617;
        let x_1620 : f32 = u_xlat20.w;
        let x_1622 : f32 = u_xlat55.x;
        let x_1624 : f32 = u_xlat80;
        u_xlat80 = ((x_1620 * x_1622) + x_1624);
        let x_1627 : vec4<f32> = u_xlat18;
        let x_1628 : vec2<f32> = vec2<f32>(x_1627.x, x_1627.y);
        let x_1630 : f32 = u_xlat2.z;
        txVec21 = vec3<f32>(x_1628.x, x_1628.y, x_1630);
        let x_1637 : vec3<f32> = txVec21;
        let x_1639 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1637.xy, x_1637.z);
        u_xlat55.x = x_1639;
        let x_1642 : f32 = u_xlat21.x;
        let x_1644 : f32 = u_xlat55.x;
        let x_1646 : f32 = u_xlat80;
        u_xlat80 = ((x_1642 * x_1644) + x_1646);
        let x_1649 : vec4<f32> = u_xlat18;
        let x_1650 : vec2<f32> = vec2<f32>(x_1649.z, x_1649.w);
        let x_1652 : f32 = u_xlat2.z;
        txVec22 = vec3<f32>(x_1650.x, x_1650.y, x_1652);
        let x_1659 : vec3<f32> = txVec22;
        let x_1661 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1659.xy, x_1659.z);
        u_xlat55.x = x_1661;
        let x_1664 : f32 = u_xlat21.y;
        let x_1666 : f32 = u_xlat55.x;
        let x_1668 : f32 = u_xlat80;
        u_xlat80 = ((x_1664 * x_1666) + x_1668);
        let x_1671 : vec2<f32> = u_xlat36;
        let x_1673 : f32 = u_xlat2.z;
        txVec23 = vec3<f32>(x_1671.x, x_1671.y, x_1673);
        let x_1680 : vec3<f32> = txVec23;
        let x_1682 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1680.xy, x_1680.z);
        u_xlat55.x = x_1682;
        let x_1685 : f32 = u_xlat21.z;
        let x_1687 : f32 = u_xlat55.x;
        let x_1689 : f32 = u_xlat80;
        u_xlat80 = ((x_1685 * x_1687) + x_1689);
        let x_1692 : vec2<f32> = u_xlat68;
        let x_1694 : f32 = u_xlat2.z;
        txVec24 = vec3<f32>(x_1692.x, x_1692.y, x_1694);
        let x_1701 : vec3<f32> = txVec24;
        let x_1703 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1701.xy, x_1701.z);
        u_xlat55.x = x_1703;
        let x_1706 : f32 = u_xlat21.w;
        let x_1708 : f32 = u_xlat55.x;
        let x_1710 : f32 = u_xlat80;
        u_xlat80 = ((x_1706 * x_1708) + x_1710);
        let x_1713 : vec4<f32> = u_xlat13;
        let x_1714 : vec2<f32> = vec2<f32>(x_1713.x, x_1713.y);
        let x_1716 : f32 = u_xlat2.z;
        txVec25 = vec3<f32>(x_1714.x, x_1714.y, x_1716);
        let x_1723 : vec3<f32> = txVec25;
        let x_1725 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1723.xy, x_1723.z);
        u_xlat55.x = x_1725;
        let x_1728 : f32 = u_xlat9.x;
        let x_1730 : f32 = u_xlat55.x;
        let x_1732 : f32 = u_xlat80;
        u_xlat80 = ((x_1728 * x_1730) + x_1732);
        let x_1735 : vec4<f32> = u_xlat13;
        let x_1736 : vec2<f32> = vec2<f32>(x_1735.z, x_1735.w);
        let x_1738 : f32 = u_xlat2.z;
        txVec26 = vec3<f32>(x_1736.x, x_1736.y, x_1738);
        let x_1745 : vec3<f32> = txVec26;
        let x_1747 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1745.xy, x_1745.z);
        u_xlat55.x = x_1747;
        let x_1750 : f32 = u_xlat9.y;
        let x_1752 : f32 = u_xlat55.x;
        let x_1754 : f32 = u_xlat80;
        u_xlat80 = ((x_1750 * x_1752) + x_1754);
        let x_1757 : vec2<f32> = u_xlat63;
        let x_1759 : f32 = u_xlat2.z;
        txVec27 = vec3<f32>(x_1757.x, x_1757.y, x_1759);
        let x_1766 : vec3<f32> = txVec27;
        let x_1768 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1766.xy, x_1766.z);
        u_xlat55.x = x_1768;
        let x_1771 : f32 = u_xlat9.z;
        let x_1773 : f32 = u_xlat55.x;
        let x_1775 : f32 = u_xlat80;
        u_xlat80 = ((x_1771 * x_1773) + x_1775);
        let x_1778 : vec4<f32> = u_xlat3;
        let x_1779 : vec2<f32> = vec2<f32>(x_1778.x, x_1778.y);
        let x_1781 : f32 = u_xlat2.z;
        txVec28 = vec3<f32>(x_1779.x, x_1779.y, x_1781);
        let x_1788 : vec3<f32> = txVec28;
        let x_1790 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1788.xy, x_1788.z);
        u_xlat3.x = x_1790;
        let x_1793 : f32 = u_xlat9.w;
        let x_1795 : f32 = u_xlat3.x;
        let x_1797 : f32 = u_xlat80;
        u_xlat79 = ((x_1793 * x_1795) + x_1797);
      }
    }
  } else {
    let x_1801 : vec4<f32> = u_xlat2;
    let x_1802 : vec2<f32> = vec2<f32>(x_1801.x, x_1801.y);
    let x_1804 : f32 = u_xlat2.z;
    txVec29 = vec3<f32>(x_1802.x, x_1802.y, x_1804);
    let x_1811 : vec3<f32> = txVec29;
    let x_1813 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_1811.xy, x_1811.z);
    u_xlat79 = x_1813;
  }
  let x_1815 : f32 = x_372.x_MainLightShadowParams.x;
  u_xlat80 = (-(x_1815) + 1.0f);
  let x_1818 : f32 = u_xlat79;
  let x_1820 : f32 = x_372.x_MainLightShadowParams.x;
  let x_1822 : f32 = u_xlat80;
  u_xlat79 = ((x_1818 * x_1820) + x_1822);
  let x_1826 : f32 = u_xlat2.z;
  u_xlatb3.x = (0.0f >= x_1826);
  let x_1831 : f32 = u_xlat2.z;
  u_xlatb29 = (x_1831 >= 1.0f);
  let x_1833 : bool = u_xlatb29;
  let x_1835 : bool = u_xlatb3.x;
  u_xlatb3.x = (x_1833 | x_1835);
  let x_1839 : bool = u_xlatb3.x;
  let x_1840 : f32 = u_xlat79;
  u_xlat79 = select(x_1840, 1.0f, x_1839);
  let x_1842 : vec3<f32> = u_xlat1;
  let x_1844 : vec4<f32> = x_149.x_MainLightPosition;
  u_xlat1.x = dot(x_1842, -(vec3<f32>(x_1844.x, x_1844.y, x_1844.z)));
  let x_1850 : f32 = u_xlat1.x;
  u_xlat1.x = clamp(x_1850, 0.0f, 1.0f);
  let x_1854 : f32 = u_xlat79;
  let x_1857 : vec4<f32> = x_149.x_MainLightColor;
  u_xlat27 = (vec3<f32>(x_1854, x_1854, x_1854) * vec3<f32>(x_1857.x, x_1857.y, x_1857.z));
  let x_1860 : vec3<f32> = u_xlat27;
  let x_1861 : vec3<f32> = u_xlat1;
  u_xlat1 = (x_1860 * vec3<f32>(x_1861.x, x_1861.x, x_1861.x));
  let x_1864 : vec3<f32> = u_xlat1;
  let x_1865 : vec4<f32> = u_xlat6;
  u_xlat1 = (x_1864 * vec3<f32>(x_1865.x, x_1865.y, x_1865.z));
  let x_1869 : f32 = x_83.unity_LODFade.x;
  u_xlatb79 = (x_1869 < 0.0f);
  let x_1872 : f32 = x_83.unity_LODFade.x;
  u_xlat29 = (x_1872 + 1.0f);
  let x_1874 : bool = u_xlatb79;
  if (x_1874) {
    let x_1879 : f32 = u_xlat29;
    x_1876 = x_1879;
  } else {
    let x_1882 : f32 = x_83.unity_LODFade.x;
    x_1876 = x_1882;
  }
  let x_1883 : f32 = x_1876;
  u_xlat79 = x_1883;
  let x_1884 : f32 = u_xlat79;
  u_xlatb29 = (0.5f >= x_1884);
  let x_1886 : vec4<f32> = u_xlat5;
  let x_1890 : vec4<f32> = x_149.x_ScreenParams;
  let x_1892 : vec3<f32> = (abs(vec3<f32>(x_1886.x, x_1886.y, x_1886.z)) * vec3<f32>(x_1890.x, x_1890.y, x_1890.x));
  let x_1893 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_1892.x, x_1892.y, x_1892.z, x_1893.w);
  let x_1898 : vec4<f32> = u_xlat5;
  u_xlatu5 = vec3<u32>(vec3<f32>(x_1898.x, x_1898.y, x_1898.z));
  let x_1903 : u32 = u_xlatu5.z;
  u_xlatu55 = (x_1903 * 1025u);
  let x_1907 : u32 = u_xlatu55;
  u_xlatu81 = (x_1907 >> 6u);
  let x_1911 : u32 = u_xlatu81;
  let x_1912 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1911 ^ x_1912));
  let x_1915 : i32 = u_xlati55;
  u_xlatu55 = (bitcast<u32>(x_1915) * 9u);
  let x_1919 : u32 = u_xlatu55;
  u_xlatu81 = (x_1919 >> 11u);
  let x_1922 : u32 = u_xlatu81;
  let x_1923 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1922 ^ x_1923));
  let x_1926 : i32 = u_xlati55;
  u_xlati55 = (x_1926 * 32769i);
  let x_1929 : i32 = u_xlati55;
  let x_1932 : u32 = u_xlatu5.y;
  u_xlati55 = bitcast<i32>((bitcast<u32>(x_1929) ^ x_1932));
  let x_1935 : i32 = u_xlati55;
  u_xlatu55 = (bitcast<u32>(x_1935) * 1025u);
  let x_1938 : u32 = u_xlatu55;
  u_xlatu81 = (x_1938 >> 6u);
  let x_1940 : u32 = u_xlatu81;
  let x_1941 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1940 ^ x_1941));
  let x_1944 : i32 = u_xlati55;
  u_xlatu55 = (bitcast<u32>(x_1944) * 9u);
  let x_1947 : u32 = u_xlatu55;
  u_xlatu81 = (x_1947 >> 11u);
  let x_1949 : u32 = u_xlatu81;
  let x_1950 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1949 ^ x_1950));
  let x_1953 : i32 = u_xlati55;
  u_xlati55 = (x_1953 * 32769i);
  let x_1955 : i32 = u_xlati55;
  let x_1958 : u32 = u_xlatu5.x;
  u_xlati55 = bitcast<i32>((bitcast<u32>(x_1955) ^ x_1958));
  let x_1961 : i32 = u_xlati55;
  u_xlatu55 = (bitcast<u32>(x_1961) * 1025u);
  let x_1964 : u32 = u_xlatu55;
  u_xlatu81 = (x_1964 >> 6u);
  let x_1966 : u32 = u_xlatu81;
  let x_1967 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1966 ^ x_1967));
  let x_1970 : i32 = u_xlati55;
  u_xlatu55 = (bitcast<u32>(x_1970) * 9u);
  let x_1973 : u32 = u_xlatu55;
  u_xlatu81 = (x_1973 >> 11u);
  let x_1975 : u32 = u_xlatu81;
  let x_1976 : u32 = u_xlatu55;
  u_xlati55 = bitcast<i32>((x_1975 ^ x_1976));
  let x_1979 : i32 = u_xlati55;
  u_xlati55 = (x_1979 * 32769i);
  param = 1065353216i;
  let x_1985 : i32 = u_xlati55;
  param_1 = x_1985;
  param_2 = 0i;
  param_3 = 23i;
  let x_1988 : i32 = int_bitfieldInsert_i1_i1_i1_i1_(&(param), &(param_1), &(param_2), &(param_3));
  u_xlat55.x = bitcast<f32>(x_1988);
  let x_1992 : f32 = u_xlat55.x;
  u_xlat55.x = (x_1992 + -1.0f);
  let x_1997 : f32 = u_xlat55.x;
  u_xlat81 = (-(x_1997) + 1.0f);
  let x_2000 : bool = u_xlatb29;
  if (x_2000) {
    let x_2005 : f32 = u_xlat55.x;
    x_2001 = x_2005;
  } else {
    let x_2007 : f32 = u_xlat81;
    x_2001 = x_2007;
  }
  let x_2008 : f32 = x_2001;
  u_xlat29 = x_2008;
  let x_2009 : f32 = u_xlat79;
  let x_2011 : f32 = u_xlat29;
  u_xlat79 = ((x_2009 * 2.0f) + -(x_2011));
  let x_2014 : f32 = u_xlat79;
  let x_2016 : f32 = u_xlat6.w;
  u_xlat29 = (x_2014 * x_2016);
  let x_2019 : f32 = u_xlat29;
  u_xlatb55 = (x_2019 >= 0.40000000596046447754f);
  let x_2022 : bool = u_xlatb55;
  let x_2023 : f32 = u_xlat29;
  u_xlat55.x = select(0.0f, x_2023, x_2022);
  let x_2027 : f32 = u_xlat6.w;
  let x_2028 : f32 = u_xlat79;
  u_xlat79 = ((x_2027 * x_2028) + -0.40000000596046447754f);
  let x_2032 : f32 = u_xlat29;
  u_xlat81 = dpdxCoarse(x_2032);
  let x_2034 : f32 = u_xlat29;
  u_xlat29 = dpdyCoarse(x_2034);
  let x_2036 : f32 = u_xlat29;
  let x_2038 : f32 = u_xlat81;
  u_xlat29 = (abs(x_2036) + abs(x_2038));
  let x_2041 : f32 = u_xlat29;
  u_xlat29 = max(x_2041, 0.00009999999747378752f);
  let x_2044 : f32 = u_xlat79;
  let x_2045 : f32 = u_xlat29;
  u_xlat79 = (x_2044 / x_2045);
  let x_2047 : f32 = u_xlat79;
  u_xlat79 = (x_2047 + 0.5f);
  let x_2049 : f32 = u_xlat79;
  u_xlat79 = clamp(x_2049, 0.0f, 1.0f);
  let x_2052 : f32 = x_149.x_AlphaToMaskAvailable;
  u_xlatb29 = !((x_2052 == 0.0f));
  let x_2054 : bool = u_xlatb29;
  if (x_2054) {
    let x_2058 : f32 = u_xlat79;
    x_2055 = x_2058;
  } else {
    let x_2061 : f32 = u_xlat55.x;
    x_2055 = x_2061;
  }
  let x_2062 : f32 = x_2055;
  u_xlat79 = x_2062;
  let x_2063 : f32 = u_xlat79;
  u_xlat55.x = (x_2063 + -0.00009999999747378752f);
  let x_2068 : f32 = u_xlat55.x;
  u_xlatb55 = (x_2068 < 0.0f);
  let x_2070 : bool = u_xlatb55;
  if (((select(0i, 1i, x_2070) * -1i) != 0i)) {
    discard;
  }
  let x_2078 : vec3<f32> = u_xlat26;
  let x_2079 : vec3<f32> = u_xlat34;
  u_xlat26 = (x_2078 * vec3<f32>(x_2079.y, x_2079.y, x_2079.y));
  let x_2082 : vec3<f32> = u_xlat34;
  let x_2084 : vec4<f32> = vs_INTERP4;
  let x_2087 : vec3<f32> = u_xlat26;
  u_xlat26 = ((vec3<f32>(x_2082.x, x_2082.x, x_2082.x) * vec3<f32>(x_2084.x, x_2084.y, x_2084.z)) + x_2087);
  let x_2089 : vec3<f32> = u_xlat34;
  let x_2091 : vec3<f32> = vs_INTERP9;
  let x_2093 : vec3<f32> = u_xlat26;
  u_xlat26 = ((vec3<f32>(x_2089.z, x_2089.z, x_2089.z) * x_2091) + x_2093);
  let x_2095 : vec3<f32> = u_xlat26;
  let x_2096 : vec3<f32> = u_xlat26;
  u_xlat55.x = dot(x_2095, x_2096);
  let x_2100 : f32 = u_xlat55.x;
  u_xlat55.x = inverseSqrt(x_2100);
  let x_2103 : vec3<f32> = u_xlat26;
  let x_2104 : vec2<f32> = u_xlat55;
  u_xlat26 = (x_2103 * vec3<f32>(x_2104.x, x_2104.x, x_2104.x));
  let x_2108 : f32 = vs_INTERP8.y;
  let x_2110 : f32 = x_149.unity_MatrixV[1i].z;
  u_xlat55.x = (x_2108 * x_2110);
  let x_2114 : f32 = x_149.unity_MatrixV[0i].z;
  let x_2116 : f32 = vs_INTERP8.x;
  let x_2119 : f32 = u_xlat55.x;
  u_xlat55.x = ((x_2114 * x_2116) + x_2119);
  let x_2123 : f32 = x_149.unity_MatrixV[2i].z;
  let x_2125 : f32 = vs_INTERP8.z;
  let x_2128 : f32 = u_xlat55.x;
  u_xlat55.x = ((x_2123 * x_2125) + x_2128);
  let x_2132 : f32 = u_xlat55.x;
  let x_2134 : f32 = x_149.unity_MatrixV[3i].z;
  u_xlat55.x = (x_2132 + x_2134);
  let x_2138 : f32 = u_xlat55.x;
  let x_2141 : f32 = x_149.x_ProjectionParams.y;
  u_xlat55.x = (-(x_2138) + -(x_2141));
  let x_2146 : f32 = u_xlat55.x;
  u_xlat55.x = max(x_2146, 0.0f);
  let x_2150 : f32 = u_xlat55.x;
  let x_2152 : f32 = x_149.unity_FogParams.x;
  u_xlat55.x = (x_2150 * x_2152);
  let x_2162 : vec2<f32> = vs_INTERP0;
  let x_2164 : f32 = x_149.x_GlobalMipBias.x;
  let x_2165 : vec4<f32> = textureSampleBias(unity_LightmapInd, samplerunity_Lightmap, x_2162, x_2164);
  u_xlat5 = x_2165;
  let x_2170 : vec2<f32> = vs_INTERP0;
  let x_2172 : f32 = x_149.x_GlobalMipBias.x;
  let x_2173 : vec4<f32> = textureSampleBias(unity_Lightmap, samplerunity_Lightmap, x_2170, x_2172);
  let x_2174 : vec3<f32> = vec3<f32>(x_2173.x, x_2173.y, x_2173.z);
  let x_2175 : vec4<f32> = u_xlat6;
  u_xlat6 = vec4<f32>(x_2174.x, x_2174.y, x_2174.z, x_2175.w);
  let x_2177 : vec4<f32> = u_xlat5;
  let x_2180 : vec3<f32> = (vec3<f32>(x_2177.x, x_2177.y, x_2177.z) + vec3<f32>(-0.5f, -0.5f, -0.5f));
  let x_2181 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_2180.x, x_2180.y, x_2180.z, x_2181.w);
  let x_2183 : vec3<f32> = u_xlat26;
  let x_2184 : vec4<f32> = u_xlat5;
  u_xlat81 = dot(x_2183, vec3<f32>(x_2184.x, x_2184.y, x_2184.z));
  let x_2187 : f32 = u_xlat81;
  u_xlat81 = (x_2187 + 0.5f);
  let x_2189 : f32 = u_xlat81;
  let x_2191 : vec4<f32> = u_xlat6;
  let x_2193 : vec3<f32> = (vec3<f32>(x_2189, x_2189, x_2189) * vec3<f32>(x_2191.x, x_2191.y, x_2191.z));
  let x_2194 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_2193.x, x_2193.y, x_2193.z, x_2194.w);
  let x_2197 : f32 = u_xlat5.w;
  u_xlat81 = max(x_2197, 0.00009999999747378752f);
  let x_2199 : vec4<f32> = u_xlat5;
  let x_2201 : f32 = u_xlat81;
  let x_2203 : vec3<f32> = (vec3<f32>(x_2199.x, x_2199.y, x_2199.z) / vec3<f32>(x_2201, x_2201, x_2201));
  let x_2204 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_2203.x, x_2203.y, x_2203.z, x_2204.w);
  let x_2207 : f32 = u_xlat8.x;
  u_xlat8.x = x_2207;
  let x_2210 : f32 = u_xlat8.x;
  u_xlat8.x = clamp(x_2210, 0.0f, 1.0f);
  let x_2213 : f32 = u_xlat79;
  u_xlat79 = x_2213;
  let x_2214 : f32 = u_xlat79;
  u_xlat79 = clamp(x_2214, 0.0f, 1.0f);
  let x_2216 : vec3<f32> = u_xlat7;
  let x_2219 : vec3<f32> = (x_2216 * vec3<f32>(0.95999997854232788086f, 0.95999997854232788086f, 0.95999997854232788086f));
  let x_2220 : vec4<f32> = u_xlat6;
  u_xlat6 = vec4<f32>(x_2219.x, x_2219.y, x_2219.z, x_2220.w);
  let x_2223 : f32 = u_xlat8.x;
  u_xlat81 = (-(x_2223) + 1.0f);
  let x_2226 : f32 = u_xlat81;
  let x_2227 : f32 = u_xlat81;
  u_xlat82 = (x_2226 * x_2227);
  let x_2229 : f32 = u_xlat82;
  u_xlat82 = max(x_2229, 0.0078125f);
  let x_2233 : f32 = u_xlat82;
  let x_2234 : f32 = u_xlat82;
  u_xlat83 = (x_2233 * x_2234);
  let x_2238 : f32 = u_xlat8.x;
  u_xlat84 = (x_2238 + 0.04000002145767211914f);
  let x_2241 : f32 = u_xlat84;
  u_xlat84 = min(x_2241, 1.0f);
  let x_2243 : f32 = u_xlat82;
  u_xlat7.x = ((x_2243 * 4.0f) + 2.0f);
  let x_2249 : f32 = vs_INTERP6.w;
  u_xlat33 = min(x_2249, 1.0f);
  let x_2251 : bool = u_xlatb0;
  if (x_2251) {
    let x_2255 : f32 = x_372.x_MainLightShadowParams.y;
    u_xlatb0 = (x_2255 == 1.0f);
    let x_2257 : bool = u_xlatb0;
    if (x_2257) {
      let x_2260 : vec4<f32> = u_xlat2;
      let x_2263 : vec4<f32> = x_372.x_MainLightShadowOffset0;
      u_xlat8 = (vec4<f32>(x_2260.x, x_2260.y, x_2260.x, x_2260.y) + x_2263);
      let x_2266 : vec4<f32> = u_xlat8;
      let x_2267 : vec2<f32> = vec2<f32>(x_2266.x, x_2266.y);
      let x_2269 : f32 = u_xlat2.z;
      txVec30 = vec3<f32>(x_2267.x, x_2267.y, x_2269);
      let x_2276 : vec3<f32> = txVec30;
      let x_2278 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2276.xy, x_2276.z);
      u_xlat9.x = x_2278;
      let x_2281 : vec4<f32> = u_xlat8;
      let x_2282 : vec2<f32> = vec2<f32>(x_2281.z, x_2281.w);
      let x_2284 : f32 = u_xlat2.z;
      txVec31 = vec3<f32>(x_2282.x, x_2282.y, x_2284);
      let x_2291 : vec3<f32> = txVec31;
      let x_2293 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2291.xy, x_2291.z);
      u_xlat9.y = x_2293;
      let x_2295 : vec4<f32> = u_xlat2;
      let x_2298 : vec4<f32> = x_372.x_MainLightShadowOffset1;
      u_xlat8 = (vec4<f32>(x_2295.x, x_2295.y, x_2295.x, x_2295.y) + x_2298);
      let x_2301 : vec4<f32> = u_xlat8;
      let x_2302 : vec2<f32> = vec2<f32>(x_2301.x, x_2301.y);
      let x_2304 : f32 = u_xlat2.z;
      txVec32 = vec3<f32>(x_2302.x, x_2302.y, x_2304);
      let x_2311 : vec3<f32> = txVec32;
      let x_2313 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2311.xy, x_2311.z);
      u_xlat9.z = x_2313;
      let x_2316 : vec4<f32> = u_xlat8;
      let x_2317 : vec2<f32> = vec2<f32>(x_2316.z, x_2316.w);
      let x_2319 : f32 = u_xlat2.z;
      txVec33 = vec3<f32>(x_2317.x, x_2317.y, x_2319);
      let x_2326 : vec3<f32> = txVec33;
      let x_2328 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2326.xy, x_2326.z);
      u_xlat9.w = x_2328;
      let x_2330 : vec4<f32> = u_xlat9;
      u_xlat0.x = dot(x_2330, vec4<f32>(0.25f, 0.25f, 0.25f, 0.25f));
    } else {
      let x_2336 : f32 = x_372.x_MainLightShadowParams.y;
      u_xlatb59 = (x_2336 == 2.0f);
      let x_2338 : bool = u_xlatb59;
      if (x_2338) {
        let x_2342 : vec4<f32> = u_xlat2;
        let x_2345 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat59 = ((vec2<f32>(x_2342.x, x_2342.y) * vec2<f32>(x_2345.z, x_2345.w)) + vec2<f32>(0.5f, 0.5f));
        let x_2349 : vec2<f32> = u_xlat59;
        u_xlat59 = floor(x_2349);
        let x_2351 : vec4<f32> = u_xlat2;
        let x_2354 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2357 : vec2<f32> = u_xlat59;
        let x_2359 : vec2<f32> = ((vec2<f32>(x_2351.x, x_2351.y) * vec2<f32>(x_2354.z, x_2354.w)) + -(x_2357));
        let x_2360 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2359.x, x_2359.y, x_2360.z, x_2360.w);
        let x_2362 : vec4<f32> = u_xlat8;
        u_xlat9 = (vec4<f32>(x_2362.x, x_2362.x, x_2362.y, x_2362.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        let x_2365 : vec4<f32> = u_xlat9;
        let x_2367 : vec4<f32> = u_xlat9;
        u_xlat10 = (vec4<f32>(x_2365.x, x_2365.x, x_2365.z, x_2365.z) * vec4<f32>(x_2367.x, x_2367.x, x_2367.z, x_2367.z));
        let x_2371 : vec4<f32> = u_xlat10;
        u_xlat60 = (vec2<f32>(x_2371.y, x_2371.w) * vec2<f32>(0.07999999821186065674f, 0.07999999821186065674f));
        let x_2374 : vec4<f32> = u_xlat10;
        let x_2377 : vec4<f32> = u_xlat8;
        let x_2380 : vec2<f32> = ((vec2<f32>(x_2374.x, x_2374.z) * vec2<f32>(0.5f, 0.5f)) + -(vec2<f32>(x_2377.x, x_2377.y)));
        let x_2381 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_2380.x, x_2381.y, x_2380.y, x_2381.w);
        let x_2383 : vec4<f32> = u_xlat8;
        let x_2386 : vec2<f32> = (-(vec2<f32>(x_2383.x, x_2383.y)) + vec2<f32>(1.0f, 1.0f));
        let x_2387 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2386.x, x_2386.y, x_2387.z, x_2387.w);
        let x_2389 : vec4<f32> = u_xlat8;
        u_xlat62 = min(vec2<f32>(x_2389.x, x_2389.y), vec2<f32>(0.0f, 0.0f));
        let x_2392 : vec2<f32> = u_xlat62;
        let x_2394 : vec2<f32> = u_xlat62;
        let x_2396 : vec4<f32> = u_xlat10;
        u_xlat62 = ((-(x_2392) * x_2394) + vec2<f32>(x_2396.x, x_2396.y));
        let x_2399 : vec4<f32> = u_xlat8;
        let x_2401 : vec2<f32> = max(vec2<f32>(x_2399.x, x_2399.y), vec2<f32>(0.0f, 0.0f));
        let x_2402 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2401.x, x_2401.y, x_2402.z, x_2402.w);
        let x_2404 : vec4<f32> = u_xlat8;
        let x_2407 : vec4<f32> = u_xlat8;
        let x_2410 : vec4<f32> = u_xlat9;
        let x_2412 : vec2<f32> = ((-(vec2<f32>(x_2404.x, x_2404.y)) * vec2<f32>(x_2407.x, x_2407.y)) + vec2<f32>(x_2410.y, x_2410.w));
        let x_2413 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2412.x, x_2412.y, x_2413.z, x_2413.w);
        let x_2415 : vec2<f32> = u_xlat62;
        u_xlat62 = (x_2415 + vec2<f32>(1.0f, 1.0f));
        let x_2417 : vec4<f32> = u_xlat8;
        let x_2419 : vec2<f32> = (vec2<f32>(x_2417.x, x_2417.y) + vec2<f32>(1.0f, 1.0f));
        let x_2420 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2419.x, x_2419.y, x_2420.z, x_2420.w);
        let x_2422 : vec4<f32> = u_xlat9;
        let x_2424 : vec2<f32> = (vec2<f32>(x_2422.x, x_2422.z) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_2425 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2424.x, x_2424.y, x_2425.z, x_2425.w);
        let x_2427 : vec4<f32> = u_xlat10;
        let x_2429 : vec2<f32> = (vec2<f32>(x_2427.x, x_2427.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_2430 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_2429.x, x_2429.y, x_2430.z, x_2430.w);
        let x_2432 : vec2<f32> = u_xlat62;
        let x_2433 : vec2<f32> = (x_2432 * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_2434 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2433.x, x_2433.y, x_2434.z, x_2434.w);
        let x_2436 : vec4<f32> = u_xlat8;
        let x_2438 : vec2<f32> = (vec2<f32>(x_2436.x, x_2436.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_2439 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_2438.x, x_2438.y, x_2439.z, x_2439.w);
        let x_2441 : vec4<f32> = u_xlat9;
        let x_2443 : vec2<f32> = (vec2<f32>(x_2441.y, x_2441.w) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
        let x_2444 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2443.x, x_2443.y, x_2444.z, x_2444.w);
        let x_2447 : f32 = u_xlat10.x;
        u_xlat11.z = x_2447;
        let x_2450 : f32 = u_xlat8.x;
        u_xlat11.w = x_2450;
        let x_2453 : f32 = u_xlat13.x;
        u_xlat12.z = x_2453;
        let x_2456 : f32 = u_xlat60.x;
        u_xlat12.w = x_2456;
        let x_2458 : vec4<f32> = u_xlat11;
        let x_2460 : vec4<f32> = u_xlat12;
        u_xlat9 = (vec4<f32>(x_2458.z, x_2458.w, x_2458.x, x_2458.z) + vec4<f32>(x_2460.z, x_2460.w, x_2460.x, x_2460.z));
        let x_2464 : f32 = u_xlat11.y;
        u_xlat10.z = x_2464;
        let x_2467 : f32 = u_xlat8.y;
        u_xlat10.w = x_2467;
        let x_2470 : f32 = u_xlat12.y;
        u_xlat13.z = x_2470;
        let x_2473 : f32 = u_xlat60.y;
        u_xlat13.w = x_2473;
        let x_2475 : vec4<f32> = u_xlat10;
        let x_2477 : vec4<f32> = u_xlat13;
        let x_2479 : vec3<f32> = (vec3<f32>(x_2475.z, x_2475.y, x_2475.w) + vec3<f32>(x_2477.z, x_2477.y, x_2477.w));
        let x_2480 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2479.x, x_2479.y, x_2479.z, x_2480.w);
        let x_2482 : vec4<f32> = u_xlat12;
        let x_2484 : vec4<f32> = u_xlat9;
        let x_2486 : vec3<f32> = (vec3<f32>(x_2482.x, x_2482.z, x_2482.w) / vec3<f32>(x_2484.z, x_2484.w, x_2484.y));
        let x_2487 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2486.x, x_2486.y, x_2486.z, x_2487.w);
        let x_2489 : vec4<f32> = u_xlat10;
        let x_2491 : vec3<f32> = (vec3<f32>(x_2489.x, x_2489.y, x_2489.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
        let x_2492 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2491.x, x_2491.y, x_2491.z, x_2492.w);
        let x_2494 : vec4<f32> = u_xlat13;
        let x_2496 : vec4<f32> = u_xlat8;
        let x_2498 : vec3<f32> = (vec3<f32>(x_2494.z, x_2494.y, x_2494.w) / vec3<f32>(x_2496.x, x_2496.y, x_2496.z));
        let x_2499 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2498.x, x_2498.y, x_2498.z, x_2499.w);
        let x_2501 : vec4<f32> = u_xlat11;
        let x_2503 : vec3<f32> = (vec3<f32>(x_2501.x, x_2501.y, x_2501.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
        let x_2504 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2503.x, x_2503.y, x_2503.z, x_2504.w);
        let x_2506 : vec4<f32> = u_xlat10;
        let x_2509 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2511 : vec3<f32> = (vec3<f32>(x_2506.y, x_2506.x, x_2506.z) * vec3<f32>(x_2509.x, x_2509.x, x_2509.x));
        let x_2512 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2511.x, x_2511.y, x_2511.z, x_2512.w);
        let x_2514 : vec4<f32> = u_xlat11;
        let x_2517 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2519 : vec3<f32> = (vec3<f32>(x_2514.x, x_2514.y, x_2514.z) * vec3<f32>(x_2517.y, x_2517.y, x_2517.y));
        let x_2520 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2519.x, x_2519.y, x_2519.z, x_2520.w);
        let x_2523 : f32 = u_xlat11.x;
        u_xlat10.w = x_2523;
        let x_2525 : vec2<f32> = u_xlat59;
        let x_2528 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2531 : vec4<f32> = u_xlat10;
        u_xlat12 = ((vec4<f32>(x_2525.x, x_2525.y, x_2525.x, x_2525.y) * vec4<f32>(x_2528.x, x_2528.y, x_2528.x, x_2528.y)) + vec4<f32>(x_2531.y, x_2531.w, x_2531.x, x_2531.w));
        let x_2534 : vec2<f32> = u_xlat59;
        let x_2536 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2539 : vec4<f32> = u_xlat10;
        let x_2541 : vec2<f32> = ((x_2534 * vec2<f32>(x_2536.x, x_2536.y)) + vec2<f32>(x_2539.z, x_2539.w));
        let x_2542 : vec4<f32> = u_xlat13;
        u_xlat13 = vec4<f32>(x_2541.x, x_2541.y, x_2542.z, x_2542.w);
        let x_2545 : f32 = u_xlat10.y;
        u_xlat11.w = x_2545;
        let x_2547 : vec4<f32> = u_xlat11;
        let x_2548 : vec2<f32> = vec2<f32>(x_2547.y, x_2547.z);
        let x_2549 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2549.x, x_2548.x, x_2549.z, x_2548.y);
        let x_2551 : vec2<f32> = u_xlat59;
        let x_2554 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2557 : vec4<f32> = u_xlat10;
        u_xlat14 = ((vec4<f32>(x_2551.x, x_2551.y, x_2551.x, x_2551.y) * vec4<f32>(x_2554.x, x_2554.y, x_2554.x, x_2554.y)) + vec4<f32>(x_2557.x, x_2557.y, x_2557.z, x_2557.y));
        let x_2560 : vec2<f32> = u_xlat59;
        let x_2563 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2566 : vec4<f32> = u_xlat11;
        u_xlat11 = ((vec4<f32>(x_2560.x, x_2560.y, x_2560.x, x_2560.y) * vec4<f32>(x_2563.x, x_2563.y, x_2563.x, x_2563.y)) + vec4<f32>(x_2566.w, x_2566.y, x_2566.w, x_2566.z));
        let x_2569 : vec2<f32> = u_xlat59;
        let x_2572 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2575 : vec4<f32> = u_xlat10;
        u_xlat10 = ((vec4<f32>(x_2569.x, x_2569.y, x_2569.x, x_2569.y) * vec4<f32>(x_2572.x, x_2572.y, x_2572.x, x_2572.y)) + vec4<f32>(x_2575.x, x_2575.w, x_2575.z, x_2575.w));
        let x_2578 : vec4<f32> = u_xlat8;
        let x_2580 : vec4<f32> = u_xlat9;
        u_xlat15 = (vec4<f32>(x_2578.x, x_2578.x, x_2578.x, x_2578.y) * vec4<f32>(x_2580.z, x_2580.w, x_2580.y, x_2580.z));
        let x_2583 : vec4<f32> = u_xlat8;
        let x_2585 : vec4<f32> = u_xlat9;
        u_xlat16 = (vec4<f32>(x_2583.y, x_2583.y, x_2583.z, x_2583.z) * x_2585);
        let x_2588 : f32 = u_xlat8.z;
        let x_2590 : f32 = u_xlat9.y;
        u_xlat59.x = (x_2588 * x_2590);
        let x_2594 : vec4<f32> = u_xlat12;
        let x_2595 : vec2<f32> = vec2<f32>(x_2594.x, x_2594.y);
        let x_2597 : f32 = u_xlat2.z;
        txVec34 = vec3<f32>(x_2595.x, x_2595.y, x_2597);
        let x_2605 : vec3<f32> = txVec34;
        let x_2607 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2605.xy, x_2605.z);
        u_xlat85 = x_2607;
        let x_2609 : vec4<f32> = u_xlat12;
        let x_2610 : vec2<f32> = vec2<f32>(x_2609.z, x_2609.w);
        let x_2612 : f32 = u_xlat2.z;
        txVec35 = vec3<f32>(x_2610.x, x_2610.y, x_2612);
        let x_2619 : vec3<f32> = txVec35;
        let x_2621 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2619.xy, x_2619.z);
        u_xlat8.x = x_2621;
        let x_2624 : f32 = u_xlat8.x;
        let x_2626 : f32 = u_xlat15.y;
        u_xlat8.x = (x_2624 * x_2626);
        let x_2630 : f32 = u_xlat15.x;
        let x_2631 : f32 = u_xlat85;
        let x_2634 : f32 = u_xlat8.x;
        u_xlat85 = ((x_2630 * x_2631) + x_2634);
        let x_2637 : vec4<f32> = u_xlat13;
        let x_2638 : vec2<f32> = vec2<f32>(x_2637.x, x_2637.y);
        let x_2640 : f32 = u_xlat2.z;
        txVec36 = vec3<f32>(x_2638.x, x_2638.y, x_2640);
        let x_2647 : vec3<f32> = txVec36;
        let x_2649 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2647.xy, x_2647.z);
        u_xlat8.x = x_2649;
        let x_2652 : f32 = u_xlat15.z;
        let x_2654 : f32 = u_xlat8.x;
        let x_2656 : f32 = u_xlat85;
        u_xlat85 = ((x_2652 * x_2654) + x_2656);
        let x_2659 : vec4<f32> = u_xlat11;
        let x_2660 : vec2<f32> = vec2<f32>(x_2659.x, x_2659.y);
        let x_2662 : f32 = u_xlat2.z;
        txVec37 = vec3<f32>(x_2660.x, x_2660.y, x_2662);
        let x_2669 : vec3<f32> = txVec37;
        let x_2671 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2669.xy, x_2669.z);
        u_xlat8.x = x_2671;
        let x_2674 : f32 = u_xlat15.w;
        let x_2676 : f32 = u_xlat8.x;
        let x_2678 : f32 = u_xlat85;
        u_xlat85 = ((x_2674 * x_2676) + x_2678);
        let x_2681 : vec4<f32> = u_xlat14;
        let x_2682 : vec2<f32> = vec2<f32>(x_2681.x, x_2681.y);
        let x_2684 : f32 = u_xlat2.z;
        txVec38 = vec3<f32>(x_2682.x, x_2682.y, x_2684);
        let x_2691 : vec3<f32> = txVec38;
        let x_2693 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2691.xy, x_2691.z);
        u_xlat8.x = x_2693;
        let x_2696 : f32 = u_xlat16.x;
        let x_2698 : f32 = u_xlat8.x;
        let x_2700 : f32 = u_xlat85;
        u_xlat85 = ((x_2696 * x_2698) + x_2700);
        let x_2703 : vec4<f32> = u_xlat14;
        let x_2704 : vec2<f32> = vec2<f32>(x_2703.z, x_2703.w);
        let x_2706 : f32 = u_xlat2.z;
        txVec39 = vec3<f32>(x_2704.x, x_2704.y, x_2706);
        let x_2713 : vec3<f32> = txVec39;
        let x_2715 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2713.xy, x_2713.z);
        u_xlat8.x = x_2715;
        let x_2718 : f32 = u_xlat16.y;
        let x_2720 : f32 = u_xlat8.x;
        let x_2722 : f32 = u_xlat85;
        u_xlat85 = ((x_2718 * x_2720) + x_2722);
        let x_2725 : vec4<f32> = u_xlat11;
        let x_2726 : vec2<f32> = vec2<f32>(x_2725.z, x_2725.w);
        let x_2728 : f32 = u_xlat2.z;
        txVec40 = vec3<f32>(x_2726.x, x_2726.y, x_2728);
        let x_2735 : vec3<f32> = txVec40;
        let x_2737 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2735.xy, x_2735.z);
        u_xlat8.x = x_2737;
        let x_2740 : f32 = u_xlat16.z;
        let x_2742 : f32 = u_xlat8.x;
        let x_2744 : f32 = u_xlat85;
        u_xlat85 = ((x_2740 * x_2742) + x_2744);
        let x_2747 : vec4<f32> = u_xlat10;
        let x_2748 : vec2<f32> = vec2<f32>(x_2747.x, x_2747.y);
        let x_2750 : f32 = u_xlat2.z;
        txVec41 = vec3<f32>(x_2748.x, x_2748.y, x_2750);
        let x_2757 : vec3<f32> = txVec41;
        let x_2759 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2757.xy, x_2757.z);
        u_xlat8.x = x_2759;
        let x_2762 : f32 = u_xlat16.w;
        let x_2764 : f32 = u_xlat8.x;
        let x_2766 : f32 = u_xlat85;
        u_xlat85 = ((x_2762 * x_2764) + x_2766);
        let x_2769 : vec4<f32> = u_xlat10;
        let x_2770 : vec2<f32> = vec2<f32>(x_2769.z, x_2769.w);
        let x_2772 : f32 = u_xlat2.z;
        txVec42 = vec3<f32>(x_2770.x, x_2770.y, x_2772);
        let x_2779 : vec3<f32> = txVec42;
        let x_2781 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_2779.xy, x_2779.z);
        u_xlat8.x = x_2781;
        let x_2784 : f32 = u_xlat59.x;
        let x_2786 : f32 = u_xlat8.x;
        let x_2788 : f32 = u_xlat85;
        u_xlat0.x = ((x_2784 * x_2786) + x_2788);
      } else {
        let x_2792 : vec4<f32> = u_xlat2;
        let x_2795 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat59 = ((vec2<f32>(x_2792.x, x_2792.y) * vec2<f32>(x_2795.z, x_2795.w)) + vec2<f32>(0.5f, 0.5f));
        let x_2799 : vec2<f32> = u_xlat59;
        u_xlat59 = floor(x_2799);
        let x_2801 : vec4<f32> = u_xlat2;
        let x_2804 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2807 : vec2<f32> = u_xlat59;
        let x_2809 : vec2<f32> = ((vec2<f32>(x_2801.x, x_2801.y) * vec2<f32>(x_2804.z, x_2804.w)) + -(x_2807));
        let x_2810 : vec4<f32> = u_xlat8;
        u_xlat8 = vec4<f32>(x_2809.x, x_2809.y, x_2810.z, x_2810.w);
        let x_2812 : vec4<f32> = u_xlat8;
        u_xlat9 = (vec4<f32>(x_2812.x, x_2812.x, x_2812.y, x_2812.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        let x_2815 : vec4<f32> = u_xlat9;
        let x_2817 : vec4<f32> = u_xlat9;
        u_xlat10 = (vec4<f32>(x_2815.x, x_2815.x, x_2815.z, x_2815.z) * vec4<f32>(x_2817.x, x_2817.x, x_2817.z, x_2817.z));
        let x_2820 : vec4<f32> = u_xlat10;
        let x_2822 : vec2<f32> = (vec2<f32>(x_2820.y, x_2820.w) * vec2<f32>(0.04081600159406661987f, 0.04081600159406661987f));
        let x_2823 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2823.x, x_2822.x, x_2823.z, x_2822.y);
        let x_2825 : vec4<f32> = u_xlat10;
        let x_2828 : vec4<f32> = u_xlat8;
        u_xlat60 = ((vec2<f32>(x_2825.x, x_2825.z) * vec2<f32>(0.5f, 0.5f)) + -(vec2<f32>(x_2828.x, x_2828.y)));
        let x_2832 : vec4<f32> = u_xlat8;
        let x_2835 : vec2<f32> = (-(vec2<f32>(x_2832.x, x_2832.y)) + vec2<f32>(1.0f, 1.0f));
        let x_2836 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_2835.x, x_2836.y, x_2835.y, x_2836.w);
        let x_2838 : vec4<f32> = u_xlat8;
        let x_2840 : vec2<f32> = min(vec2<f32>(x_2838.x, x_2838.y), vec2<f32>(0.0f, 0.0f));
        let x_2841 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2840.x, x_2840.y, x_2841.z, x_2841.w);
        let x_2843 : vec4<f32> = u_xlat10;
        let x_2846 : vec4<f32> = u_xlat10;
        let x_2849 : vec4<f32> = u_xlat9;
        let x_2851 : vec2<f32> = ((-(vec2<f32>(x_2843.x, x_2843.y)) * vec2<f32>(x_2846.x, x_2846.y)) + vec2<f32>(x_2849.x, x_2849.z));
        let x_2852 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_2851.x, x_2852.y, x_2851.y, x_2852.w);
        let x_2854 : vec4<f32> = u_xlat8;
        let x_2856 : vec2<f32> = max(vec2<f32>(x_2854.x, x_2854.y), vec2<f32>(0.0f, 0.0f));
        let x_2857 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2856.x, x_2856.y, x_2857.z, x_2857.w);
        let x_2859 : vec4<f32> = u_xlat10;
        let x_2862 : vec4<f32> = u_xlat10;
        let x_2865 : vec4<f32> = u_xlat9;
        let x_2867 : vec2<f32> = ((-(vec2<f32>(x_2859.x, x_2859.y)) * vec2<f32>(x_2862.x, x_2862.y)) + vec2<f32>(x_2865.y, x_2865.w));
        let x_2868 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_2868.x, x_2867.x, x_2868.z, x_2867.y);
        let x_2870 : vec4<f32> = u_xlat9;
        u_xlat9 = (x_2870 + vec4<f32>(2.0f, 2.0f, 2.0f, 2.0f));
        let x_2873 : f32 = u_xlat9.y;
        u_xlat10.z = (x_2873 * 0.08163200318813323975f);
        let x_2876 : vec2<f32> = u_xlat60;
        let x_2878 : vec2<f32> = (vec2<f32>(x_2876.y, x_2876.x) * vec2<f32>(0.08163200318813323975f, 0.08163200318813323975f));
        let x_2879 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_2878.x, x_2878.y, x_2879.z, x_2879.w);
        let x_2881 : vec4<f32> = u_xlat9;
        u_xlat60 = (vec2<f32>(x_2881.x, x_2881.z) * vec2<f32>(0.08163200318813323975f, 0.08163200318813323975f));
        let x_2885 : f32 = u_xlat9.w;
        u_xlat12.z = (x_2885 * 0.08163200318813323975f);
        let x_2889 : f32 = u_xlat12.y;
        u_xlat10.x = x_2889;
        let x_2891 : vec4<f32> = u_xlat8;
        let x_2894 : vec2<f32> = ((vec2<f32>(x_2891.x, x_2891.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let x_2895 : vec4<f32> = u_xlat10;
        u_xlat10 = vec4<f32>(x_2895.x, x_2894.x, x_2895.z, x_2894.y);
        let x_2897 : vec4<f32> = u_xlat8;
        let x_2900 : vec2<f32> = ((vec2<f32>(x_2897.x, x_2897.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let x_2901 : vec4<f32> = u_xlat9;
        u_xlat9 = vec4<f32>(x_2900.x, x_2901.y, x_2900.y, x_2901.w);
        let x_2904 : f32 = u_xlat60.x;
        u_xlat9.y = x_2904;
        let x_2907 : f32 = u_xlat11.y;
        u_xlat9.w = x_2907;
        let x_2909 : vec4<f32> = u_xlat9;
        let x_2910 : vec4<f32> = u_xlat10;
        u_xlat10 = (x_2909 + x_2910);
        let x_2912 : vec4<f32> = u_xlat8;
        let x_2915 : vec2<f32> = ((vec2<f32>(x_2912.y, x_2912.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let x_2916 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_2916.x, x_2915.x, x_2916.z, x_2915.y);
        let x_2918 : vec4<f32> = u_xlat8;
        let x_2921 : vec2<f32> = ((vec2<f32>(x_2918.y, x_2918.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let x_2922 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_2921.x, x_2922.y, x_2921.y, x_2922.w);
        let x_2925 : f32 = u_xlat60.y;
        u_xlat11.y = x_2925;
        let x_2927 : vec4<f32> = u_xlat11;
        let x_2928 : vec4<f32> = u_xlat12;
        u_xlat8 = (x_2927 + x_2928);
        let x_2930 : vec4<f32> = u_xlat9;
        let x_2931 : vec4<f32> = u_xlat10;
        u_xlat9 = (x_2930 / x_2931);
        let x_2933 : vec4<f32> = u_xlat9;
        u_xlat9 = (x_2933 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        let x_2935 : vec4<f32> = u_xlat11;
        let x_2936 : vec4<f32> = u_xlat8;
        u_xlat11 = (x_2935 / x_2936);
        let x_2938 : vec4<f32> = u_xlat11;
        u_xlat11 = (x_2938 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        let x_2940 : vec4<f32> = u_xlat9;
        let x_2943 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat9 = (vec4<f32>(x_2940.w, x_2940.x, x_2940.y, x_2940.z) * vec4<f32>(x_2943.x, x_2943.x, x_2943.x, x_2943.x));
        let x_2946 : vec4<f32> = u_xlat11;
        let x_2949 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        u_xlat11 = (vec4<f32>(x_2946.x, x_2946.w, x_2946.y, x_2946.z) * vec4<f32>(x_2949.y, x_2949.y, x_2949.y, x_2949.y));
        let x_2952 : vec4<f32> = u_xlat9;
        let x_2953 : vec3<f32> = vec3<f32>(x_2952.y, x_2952.z, x_2952.w);
        let x_2954 : vec4<f32> = u_xlat12;
        u_xlat12 = vec4<f32>(x_2953.x, x_2954.y, x_2953.y, x_2953.z);
        let x_2957 : f32 = u_xlat11.x;
        u_xlat12.y = x_2957;
        let x_2959 : vec2<f32> = u_xlat59;
        let x_2962 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2965 : vec4<f32> = u_xlat12;
        u_xlat13 = ((vec4<f32>(x_2959.x, x_2959.y, x_2959.x, x_2959.y) * vec4<f32>(x_2962.x, x_2962.y, x_2962.x, x_2962.y)) + vec4<f32>(x_2965.x, x_2965.y, x_2965.z, x_2965.y));
        let x_2968 : vec2<f32> = u_xlat59;
        let x_2970 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2973 : vec4<f32> = u_xlat12;
        let x_2975 : vec2<f32> = ((x_2968 * vec2<f32>(x_2970.x, x_2970.y)) + vec2<f32>(x_2973.w, x_2973.y));
        let x_2976 : vec4<f32> = u_xlat14;
        u_xlat14 = vec4<f32>(x_2975.x, x_2975.y, x_2976.z, x_2976.w);
        let x_2979 : f32 = u_xlat12.y;
        u_xlat9.y = x_2979;
        let x_2982 : f32 = u_xlat11.z;
        u_xlat12.y = x_2982;
        let x_2984 : vec2<f32> = u_xlat59;
        let x_2987 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2990 : vec4<f32> = u_xlat12;
        u_xlat15 = ((vec4<f32>(x_2984.x, x_2984.y, x_2984.x, x_2984.y) * vec4<f32>(x_2987.x, x_2987.y, x_2987.x, x_2987.y)) + vec4<f32>(x_2990.x, x_2990.y, x_2990.z, x_2990.y));
        let x_2994 : vec2<f32> = u_xlat59;
        let x_2996 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_2999 : vec4<f32> = u_xlat12;
        u_xlat66 = ((x_2994 * vec2<f32>(x_2996.x, x_2996.y)) + vec2<f32>(x_2999.w, x_2999.y));
        let x_3003 : f32 = u_xlat12.y;
        u_xlat9.z = x_3003;
        let x_3005 : vec2<f32> = u_xlat59;
        let x_3008 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3011 : vec4<f32> = u_xlat9;
        u_xlat16 = ((vec4<f32>(x_3005.x, x_3005.y, x_3005.x, x_3005.y) * vec4<f32>(x_3008.x, x_3008.y, x_3008.x, x_3008.y)) + vec4<f32>(x_3011.x, x_3011.y, x_3011.x, x_3011.z));
        let x_3015 : f32 = u_xlat11.w;
        u_xlat12.y = x_3015;
        let x_3017 : vec2<f32> = u_xlat59;
        let x_3020 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3023 : vec4<f32> = u_xlat12;
        u_xlat17 = ((vec4<f32>(x_3017.x, x_3017.y, x_3017.x, x_3017.y) * vec4<f32>(x_3020.x, x_3020.y, x_3020.x, x_3020.y)) + vec4<f32>(x_3023.x, x_3023.y, x_3023.z, x_3023.y));
        let x_3026 : vec2<f32> = u_xlat59;
        let x_3028 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3031 : vec4<f32> = u_xlat12;
        let x_3033 : vec2<f32> = ((x_3026 * vec2<f32>(x_3028.x, x_3028.y)) + vec2<f32>(x_3031.w, x_3031.y));
        let x_3034 : vec3<f32> = u_xlat35;
        u_xlat35 = vec3<f32>(x_3033.x, x_3033.y, x_3034.z);
        let x_3037 : f32 = u_xlat12.y;
        u_xlat9.w = x_3037;
        let x_3039 : vec2<f32> = u_xlat59;
        let x_3041 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3044 : vec4<f32> = u_xlat9;
        let x_3046 : vec2<f32> = ((x_3039 * vec2<f32>(x_3041.x, x_3041.y)) + vec2<f32>(x_3044.x, x_3044.w));
        let x_3047 : vec4<f32> = u_xlat18;
        u_xlat18 = vec4<f32>(x_3046.x, x_3046.y, x_3047.z, x_3047.w);
        let x_3049 : vec4<f32> = u_xlat12;
        let x_3050 : vec3<f32> = vec3<f32>(x_3049.x, x_3049.z, x_3049.w);
        let x_3051 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_3050.x, x_3051.y, x_3050.y, x_3050.z);
        let x_3053 : vec2<f32> = u_xlat59;
        let x_3056 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3059 : vec4<f32> = u_xlat11;
        u_xlat12 = ((vec4<f32>(x_3053.x, x_3053.y, x_3053.x, x_3053.y) * vec4<f32>(x_3056.x, x_3056.y, x_3056.x, x_3056.y)) + vec4<f32>(x_3059.x, x_3059.y, x_3059.z, x_3059.y));
        let x_3062 : vec2<f32> = u_xlat59;
        let x_3064 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3067 : vec4<f32> = u_xlat11;
        u_xlat63 = ((x_3062 * vec2<f32>(x_3064.x, x_3064.y)) + vec2<f32>(x_3067.w, x_3067.y));
        let x_3071 : f32 = u_xlat9.x;
        u_xlat11.x = x_3071;
        let x_3073 : vec2<f32> = u_xlat59;
        let x_3075 : vec4<f32> = x_372.x_MainLightShadowmapSize;
        let x_3078 : vec4<f32> = u_xlat11;
        u_xlat59 = ((x_3073 * vec2<f32>(x_3075.x, x_3075.y)) + vec2<f32>(x_3078.x, x_3078.y));
        let x_3081 : vec4<f32> = u_xlat8;
        let x_3083 : vec4<f32> = u_xlat10;
        u_xlat19 = (vec4<f32>(x_3081.x, x_3081.x, x_3081.x, x_3081.x) * x_3083);
        let x_3085 : vec4<f32> = u_xlat8;
        let x_3087 : vec4<f32> = u_xlat10;
        u_xlat20 = (vec4<f32>(x_3085.y, x_3085.y, x_3085.y, x_3085.y) * x_3087);
        let x_3089 : vec4<f32> = u_xlat8;
        let x_3091 : vec4<f32> = u_xlat10;
        u_xlat21 = (vec4<f32>(x_3089.z, x_3089.z, x_3089.z, x_3089.z) * x_3091);
        let x_3093 : vec4<f32> = u_xlat8;
        let x_3095 : vec4<f32> = u_xlat10;
        u_xlat8 = (vec4<f32>(x_3093.w, x_3093.w, x_3093.w, x_3093.w) * x_3095);
        let x_3098 : vec4<f32> = u_xlat13;
        let x_3099 : vec2<f32> = vec2<f32>(x_3098.x, x_3098.y);
        let x_3101 : f32 = u_xlat2.z;
        txVec43 = vec3<f32>(x_3099.x, x_3099.y, x_3101);
        let x_3108 : vec3<f32> = txVec43;
        let x_3110 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3108.xy, x_3108.z);
        u_xlat9.x = x_3110;
        let x_3113 : vec4<f32> = u_xlat13;
        let x_3114 : vec2<f32> = vec2<f32>(x_3113.z, x_3113.w);
        let x_3116 : f32 = u_xlat2.z;
        txVec44 = vec3<f32>(x_3114.x, x_3114.y, x_3116);
        let x_3124 : vec3<f32> = txVec44;
        let x_3126 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3124.xy, x_3124.z);
        u_xlat87 = x_3126;
        let x_3127 : f32 = u_xlat87;
        let x_3129 : f32 = u_xlat19.y;
        u_xlat87 = (x_3127 * x_3129);
        let x_3132 : f32 = u_xlat19.x;
        let x_3134 : f32 = u_xlat9.x;
        let x_3136 : f32 = u_xlat87;
        u_xlat9.x = ((x_3132 * x_3134) + x_3136);
        let x_3140 : vec4<f32> = u_xlat14;
        let x_3141 : vec2<f32> = vec2<f32>(x_3140.x, x_3140.y);
        let x_3143 : f32 = u_xlat2.z;
        txVec45 = vec3<f32>(x_3141.x, x_3141.y, x_3143);
        let x_3150 : vec3<f32> = txVec45;
        let x_3152 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3150.xy, x_3150.z);
        u_xlat87 = x_3152;
        let x_3154 : f32 = u_xlat19.z;
        let x_3155 : f32 = u_xlat87;
        let x_3158 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3154 * x_3155) + x_3158);
        let x_3162 : vec4<f32> = u_xlat16;
        let x_3163 : vec2<f32> = vec2<f32>(x_3162.x, x_3162.y);
        let x_3165 : f32 = u_xlat2.z;
        txVec46 = vec3<f32>(x_3163.x, x_3163.y, x_3165);
        let x_3172 : vec3<f32> = txVec46;
        let x_3174 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3172.xy, x_3172.z);
        u_xlat87 = x_3174;
        let x_3176 : f32 = u_xlat19.w;
        let x_3177 : f32 = u_xlat87;
        let x_3180 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3176 * x_3177) + x_3180);
        let x_3184 : vec4<f32> = u_xlat15;
        let x_3185 : vec2<f32> = vec2<f32>(x_3184.x, x_3184.y);
        let x_3187 : f32 = u_xlat2.z;
        txVec47 = vec3<f32>(x_3185.x, x_3185.y, x_3187);
        let x_3194 : vec3<f32> = txVec47;
        let x_3196 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3194.xy, x_3194.z);
        u_xlat87 = x_3196;
        let x_3198 : f32 = u_xlat20.x;
        let x_3199 : f32 = u_xlat87;
        let x_3202 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3198 * x_3199) + x_3202);
        let x_3206 : vec4<f32> = u_xlat15;
        let x_3207 : vec2<f32> = vec2<f32>(x_3206.z, x_3206.w);
        let x_3209 : f32 = u_xlat2.z;
        txVec48 = vec3<f32>(x_3207.x, x_3207.y, x_3209);
        let x_3216 : vec3<f32> = txVec48;
        let x_3218 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3216.xy, x_3216.z);
        u_xlat87 = x_3218;
        let x_3220 : f32 = u_xlat20.y;
        let x_3221 : f32 = u_xlat87;
        let x_3224 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3220 * x_3221) + x_3224);
        let x_3228 : vec2<f32> = u_xlat66;
        let x_3230 : f32 = u_xlat2.z;
        txVec49 = vec3<f32>(x_3228.x, x_3228.y, x_3230);
        let x_3237 : vec3<f32> = txVec49;
        let x_3239 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3237.xy, x_3237.z);
        u_xlat87 = x_3239;
        let x_3241 : f32 = u_xlat20.z;
        let x_3242 : f32 = u_xlat87;
        let x_3245 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3241 * x_3242) + x_3245);
        let x_3249 : vec4<f32> = u_xlat16;
        let x_3250 : vec2<f32> = vec2<f32>(x_3249.z, x_3249.w);
        let x_3252 : f32 = u_xlat2.z;
        txVec50 = vec3<f32>(x_3250.x, x_3250.y, x_3252);
        let x_3259 : vec3<f32> = txVec50;
        let x_3261 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3259.xy, x_3259.z);
        u_xlat87 = x_3261;
        let x_3263 : f32 = u_xlat20.w;
        let x_3264 : f32 = u_xlat87;
        let x_3267 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3263 * x_3264) + x_3267);
        let x_3271 : vec4<f32> = u_xlat17;
        let x_3272 : vec2<f32> = vec2<f32>(x_3271.x, x_3271.y);
        let x_3274 : f32 = u_xlat2.z;
        txVec51 = vec3<f32>(x_3272.x, x_3272.y, x_3274);
        let x_3281 : vec3<f32> = txVec51;
        let x_3283 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3281.xy, x_3281.z);
        u_xlat87 = x_3283;
        let x_3285 : f32 = u_xlat21.x;
        let x_3286 : f32 = u_xlat87;
        let x_3289 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3285 * x_3286) + x_3289);
        let x_3293 : vec4<f32> = u_xlat17;
        let x_3294 : vec2<f32> = vec2<f32>(x_3293.z, x_3293.w);
        let x_3296 : f32 = u_xlat2.z;
        txVec52 = vec3<f32>(x_3294.x, x_3294.y, x_3296);
        let x_3303 : vec3<f32> = txVec52;
        let x_3305 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3303.xy, x_3303.z);
        u_xlat87 = x_3305;
        let x_3307 : f32 = u_xlat21.y;
        let x_3308 : f32 = u_xlat87;
        let x_3311 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3307 * x_3308) + x_3311);
        let x_3315 : vec3<f32> = u_xlat35;
        let x_3316 : vec2<f32> = vec2<f32>(x_3315.x, x_3315.y);
        let x_3318 : f32 = u_xlat2.z;
        txVec53 = vec3<f32>(x_3316.x, x_3316.y, x_3318);
        let x_3325 : vec3<f32> = txVec53;
        let x_3327 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3325.xy, x_3325.z);
        u_xlat35.x = x_3327;
        let x_3330 : f32 = u_xlat21.z;
        let x_3332 : f32 = u_xlat35.x;
        let x_3335 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3330 * x_3332) + x_3335);
        let x_3339 : vec4<f32> = u_xlat18;
        let x_3340 : vec2<f32> = vec2<f32>(x_3339.x, x_3339.y);
        let x_3342 : f32 = u_xlat2.z;
        txVec54 = vec3<f32>(x_3340.x, x_3340.y, x_3342);
        let x_3349 : vec3<f32> = txVec54;
        let x_3351 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3349.xy, x_3349.z);
        u_xlat35.x = x_3351;
        let x_3354 : f32 = u_xlat21.w;
        let x_3356 : f32 = u_xlat35.x;
        let x_3359 : f32 = u_xlat9.x;
        u_xlat9.x = ((x_3354 * x_3356) + x_3359);
        let x_3363 : vec4<f32> = u_xlat12;
        let x_3364 : vec2<f32> = vec2<f32>(x_3363.x, x_3363.y);
        let x_3366 : f32 = u_xlat2.z;
        txVec55 = vec3<f32>(x_3364.x, x_3364.y, x_3366);
        let x_3373 : vec3<f32> = txVec55;
        let x_3375 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3373.xy, x_3373.z);
        u_xlat35.x = x_3375;
        let x_3378 : f32 = u_xlat8.x;
        let x_3380 : f32 = u_xlat35.x;
        let x_3383 : f32 = u_xlat9.x;
        u_xlat8.x = ((x_3378 * x_3380) + x_3383);
        let x_3387 : vec4<f32> = u_xlat12;
        let x_3388 : vec2<f32> = vec2<f32>(x_3387.z, x_3387.w);
        let x_3390 : f32 = u_xlat2.z;
        txVec56 = vec3<f32>(x_3388.x, x_3388.y, x_3390);
        let x_3397 : vec3<f32> = txVec56;
        let x_3399 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3397.xy, x_3397.z);
        u_xlat9.x = x_3399;
        let x_3402 : f32 = u_xlat8.y;
        let x_3404 : f32 = u_xlat9.x;
        let x_3407 : f32 = u_xlat8.x;
        u_xlat8.x = ((x_3402 * x_3404) + x_3407);
        let x_3411 : vec2<f32> = u_xlat63;
        let x_3413 : f32 = u_xlat2.z;
        txVec57 = vec3<f32>(x_3411.x, x_3411.y, x_3413);
        let x_3420 : vec3<f32> = txVec57;
        let x_3422 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3420.xy, x_3420.z);
        u_xlat34.x = x_3422;
        let x_3425 : f32 = u_xlat8.z;
        let x_3427 : f32 = u_xlat34.x;
        let x_3430 : f32 = u_xlat8.x;
        u_xlat8.x = ((x_3425 * x_3427) + x_3430);
        let x_3434 : vec2<f32> = u_xlat59;
        let x_3436 : f32 = u_xlat2.z;
        txVec58 = vec3<f32>(x_3434.x, x_3434.y, x_3436);
        let x_3443 : vec3<f32> = txVec58;
        let x_3445 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3443.xy, x_3443.z);
        u_xlat59.x = x_3445;
        let x_3448 : f32 = u_xlat8.w;
        let x_3450 : f32 = u_xlat59.x;
        let x_3453 : f32 = u_xlat8.x;
        u_xlat0.x = ((x_3448 * x_3450) + x_3453);
      }
    }
  } else {
    let x_3458 : vec4<f32> = u_xlat2;
    let x_3459 : vec2<f32> = vec2<f32>(x_3458.x, x_3458.y);
    let x_3461 : f32 = u_xlat2.z;
    txVec59 = vec3<f32>(x_3459.x, x_3459.y, x_3461);
    let x_3468 : vec3<f32> = txVec59;
    let x_3470 : f32 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, x_3468.xy, x_3468.z);
    u_xlat0.x = x_3470;
  }
  let x_3473 : f32 = u_xlat0.x;
  let x_3475 : f32 = x_372.x_MainLightShadowParams.x;
  let x_3477 : f32 = u_xlat80;
  u_xlat0.x = ((x_3473 * x_3475) + x_3477);
  let x_3481 : bool = u_xlatb3.x;
  if (x_3481) {
    x_3482 = 1.0f;
  } else {
    let x_3487 : f32 = u_xlat0.x;
    x_3482 = x_3487;
  }
  let x_3488 : f32 = x_3482;
  u_xlat0.x = x_3488;
  let x_3490 : vec3<f32> = vs_INTERP8;
  let x_3492 : vec3<f32> = x_149.x_WorldSpaceCameraPos;
  let x_3494 : vec3<f32> = (x_3490 + -(x_3492));
  let x_3495 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_3494.x, x_3494.y, x_3494.z, x_3495.w);
  let x_3497 : vec4<f32> = u_xlat2;
  let x_3499 : vec4<f32> = u_xlat2;
  u_xlat2.x = dot(vec3<f32>(x_3497.x, x_3497.y, x_3497.z), vec3<f32>(x_3499.x, x_3499.y, x_3499.z));
  let x_3505 : f32 = u_xlat2.x;
  let x_3507 : f32 = x_372.x_MainLightShadowParams.z;
  let x_3510 : f32 = x_372.x_MainLightShadowParams.w;
  u_xlat28.x = ((x_3505 * x_3507) + x_3510);
  let x_3514 : f32 = u_xlat28.x;
  u_xlat28.x = clamp(x_3514, 0.0f, 1.0f);
  let x_3519 : f32 = u_xlat0.x;
  u_xlat54 = (-(x_3519) + 1.0f);
  let x_3523 : f32 = u_xlat28.x;
  let x_3524 : f32 = u_xlat54;
  let x_3527 : f32 = u_xlat0.x;
  u_xlat0.x = ((x_3523 * x_3524) + x_3527);
  let x_3538 : f32 = x_3536.x_MainLightCookieTextureFormat;
  u_xlatb28.x = !((x_3538 == -1.0f));
  let x_3542 : bool = u_xlatb28.x;
  if (x_3542) {
    let x_3545 : vec3<f32> = vs_INTERP8;
    let x_3548 : vec4<f32> = x_3536.x_MainLightWorldToLight[1i];
    let x_3550 : vec2<f32> = (vec2<f32>(x_3545.y, x_3545.y) * vec2<f32>(x_3548.x, x_3548.y));
    let x_3551 : vec3<f32> = u_xlat28;
    u_xlat28 = vec3<f32>(x_3550.x, x_3550.y, x_3551.z);
    let x_3554 : vec4<f32> = x_3536.x_MainLightWorldToLight[0i];
    let x_3556 : vec3<f32> = vs_INTERP8;
    let x_3559 : vec3<f32> = u_xlat28;
    let x_3561 : vec2<f32> = ((vec2<f32>(x_3554.x, x_3554.y) * vec2<f32>(x_3556.x, x_3556.x)) + vec2<f32>(x_3559.x, x_3559.y));
    let x_3562 : vec3<f32> = u_xlat28;
    u_xlat28 = vec3<f32>(x_3561.x, x_3561.y, x_3562.z);
    let x_3565 : vec4<f32> = x_3536.x_MainLightWorldToLight[2i];
    let x_3567 : vec3<f32> = vs_INTERP8;
    let x_3570 : vec3<f32> = u_xlat28;
    let x_3572 : vec2<f32> = ((vec2<f32>(x_3565.x, x_3565.y) * vec2<f32>(x_3567.z, x_3567.z)) + vec2<f32>(x_3570.x, x_3570.y));
    let x_3573 : vec3<f32> = u_xlat28;
    u_xlat28 = vec3<f32>(x_3572.x, x_3572.y, x_3573.z);
    let x_3575 : vec3<f32> = u_xlat28;
    let x_3578 : vec4<f32> = x_3536.x_MainLightWorldToLight[3i];
    let x_3580 : vec2<f32> = (vec2<f32>(x_3575.x, x_3575.y) + vec2<f32>(x_3578.x, x_3578.y));
    let x_3581 : vec3<f32> = u_xlat28;
    u_xlat28 = vec3<f32>(x_3580.x, x_3580.y, x_3581.z);
    let x_3583 : vec3<f32> = u_xlat28;
    let x_3586 : vec2<f32> = ((vec2<f32>(x_3583.x, x_3583.y) * vec2<f32>(0.5f, 0.5f)) + vec2<f32>(0.5f, 0.5f));
    let x_3587 : vec3<f32> = u_xlat28;
    u_xlat28 = vec3<f32>(x_3586.x, x_3586.y, x_3587.z);
    let x_3594 : vec3<f32> = u_xlat28;
    let x_3597 : f32 = x_149.x_GlobalMipBias.x;
    let x_3598 : vec4<f32> = textureSampleBias(x_MainLightCookieTexture, sampler_MainLightCookieTexture, vec2<f32>(x_3594.x, x_3594.y), x_3597);
    u_xlat8 = x_3598;
    let x_3600 : f32 = x_3536.x_MainLightCookieTextureFormat;
    let x_3602 : f32 = x_3536.x_MainLightCookieTextureFormat;
    let x_3604 : f32 = x_3536.x_MainLightCookieTextureFormat;
    let x_3606 : f32 = x_3536.x_MainLightCookieTextureFormat;
    let x_3607 : vec4<f32> = vec4<f32>(x_3600, x_3602, x_3604, x_3606);
    let x_3614 : vec4<bool> = (vec4<f32>(x_3607.x, x_3607.y, x_3607.z, x_3607.w) == vec4<f32>(0.0f, 1.0f, 0.0f, 0.0f));
    u_xlatb28 = vec2<bool>(x_3614.x, x_3614.y);
    let x_3617 : bool = u_xlatb28.y;
    if (x_3617) {
      let x_3622 : f32 = u_xlat8.w;
      x_3618 = x_3622;
    } else {
      let x_3625 : f32 = u_xlat8.x;
      x_3618 = x_3625;
    }
    let x_3626 : f32 = x_3618;
    u_xlat54 = x_3626;
    let x_3628 : bool = u_xlatb28.x;
    if (x_3628) {
      let x_3632 : vec4<f32> = u_xlat8;
      x_3629 = vec3<f32>(x_3632.x, x_3632.y, x_3632.z);
    } else {
      let x_3635 : f32 = u_xlat54;
      x_3629 = vec3<f32>(x_3635, x_3635, x_3635);
    }
    let x_3637 : vec3<f32> = x_3629;
    u_xlat28 = x_3637;
  } else {
    u_xlat28.x = 1.0f;
    u_xlat28.y = 1.0f;
    u_xlat28.z = 1.0f;
  }
  let x_3642 : vec3<f32> = u_xlat28;
  let x_3644 : vec4<f32> = x_149.x_MainLightColor;
  u_xlat28 = (x_3642 * vec3<f32>(x_3644.x, x_3644.y, x_3644.z));
  let x_3647 : vec3<f32> = u_xlat4;
  let x_3649 : vec3<f32> = u_xlat26;
  u_xlat3.x = dot(-(x_3647), x_3649);
  let x_3653 : f32 = u_xlat3.x;
  let x_3655 : f32 = u_xlat3.x;
  u_xlat3.x = (x_3653 + x_3655);
  let x_3658 : vec3<f32> = u_xlat26;
  let x_3659 : vec4<f32> = u_xlat3;
  let x_3663 : vec3<f32> = u_xlat4;
  let x_3665 : vec3<f32> = ((x_3658 * -(vec3<f32>(x_3659.x, x_3659.x, x_3659.x))) + -(x_3663));
  let x_3666 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3665.x, x_3665.y, x_3665.z, x_3666.w);
  let x_3668 : vec3<f32> = u_xlat26;
  let x_3669 : vec3<f32> = u_xlat4;
  u_xlat3.x = dot(x_3668, x_3669);
  let x_3673 : f32 = u_xlat3.x;
  u_xlat3.x = clamp(x_3673, 0.0f, 1.0f);
  let x_3677 : f32 = u_xlat3.x;
  u_xlat3.x = (-(x_3677) + 1.0f);
  let x_3682 : f32 = u_xlat3.x;
  let x_3684 : f32 = u_xlat3.x;
  u_xlat3.x = (x_3682 * x_3684);
  let x_3688 : f32 = u_xlat3.x;
  let x_3690 : f32 = u_xlat3.x;
  u_xlat3.x = (x_3688 * x_3690);
  let x_3693 : f32 = u_xlat81;
  u_xlat59.x = ((-(x_3693) * 0.69999998807907104492f) + 1.70000004768371582031f);
  let x_3700 : f32 = u_xlat81;
  let x_3702 : f32 = u_xlat59.x;
  u_xlat81 = (x_3700 * x_3702);
  let x_3704 : f32 = u_xlat81;
  u_xlat81 = (x_3704 * 6.0f);
  let x_3715 : vec4<f32> = u_xlat8;
  let x_3717 : f32 = u_xlat81;
  let x_3718 : vec4<f32> = textureSampleLevel(unity_SpecCube0, samplerunity_SpecCube0, vec3<f32>(x_3715.x, x_3715.y, x_3715.z), x_3717);
  u_xlat8 = x_3718;
  let x_3720 : f32 = u_xlat8.w;
  u_xlat81 = (x_3720 + -1.0f);
  let x_3723 : f32 = x_83.unity_SpecCube0_HDR.w;
  let x_3724 : f32 = u_xlat81;
  u_xlat81 = ((x_3723 * x_3724) + 1.0f);
  let x_3727 : f32 = u_xlat81;
  u_xlat81 = max(x_3727, 0.0f);
  let x_3729 : f32 = u_xlat81;
  u_xlat81 = log2(x_3729);
  let x_3731 : f32 = u_xlat81;
  let x_3733 : f32 = x_83.unity_SpecCube0_HDR.y;
  u_xlat81 = (x_3731 * x_3733);
  let x_3735 : f32 = u_xlat81;
  u_xlat81 = exp2(x_3735);
  let x_3737 : f32 = u_xlat81;
  let x_3739 : f32 = x_83.unity_SpecCube0_HDR.x;
  u_xlat81 = (x_3737 * x_3739);
  let x_3741 : vec4<f32> = u_xlat8;
  let x_3743 : f32 = u_xlat81;
  let x_3745 : vec3<f32> = (vec3<f32>(x_3741.x, x_3741.y, x_3741.z) * vec3<f32>(x_3743, x_3743, x_3743));
  let x_3746 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3745.x, x_3745.y, x_3745.z, x_3746.w);
  let x_3748 : f32 = u_xlat82;
  let x_3750 : f32 = u_xlat82;
  u_xlat59 = ((vec2<f32>(x_3748, x_3748) * vec2<f32>(x_3750, x_3750)) + vec2<f32>(-1.0f, 1.0f));
  let x_3756 : f32 = u_xlat59.y;
  u_xlat81 = (1.0f / x_3756);
  let x_3758 : f32 = u_xlat84;
  u_xlat82 = (x_3758 + -0.03999999910593032837f);
  let x_3762 : f32 = u_xlat3.x;
  let x_3763 : f32 = u_xlat82;
  u_xlat3.x = ((x_3762 * x_3763) + 0.03999999910593032837f);
  let x_3769 : f32 = u_xlat3.x;
  let x_3770 : f32 = u_xlat81;
  u_xlat3.x = (x_3769 * x_3770);
  let x_3773 : vec4<f32> = u_xlat3;
  let x_3775 : vec4<f32> = u_xlat8;
  let x_3777 : vec3<f32> = (vec3<f32>(x_3773.x, x_3773.x, x_3773.x) * vec3<f32>(x_3775.x, x_3775.y, x_3775.z));
  let x_3778 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3777.x, x_3777.y, x_3777.z, x_3778.w);
  let x_3780 : vec4<f32> = u_xlat5;
  let x_3782 : vec4<f32> = u_xlat6;
  let x_3785 : vec4<f32> = u_xlat8;
  let x_3787 : vec3<f32> = ((vec3<f32>(x_3780.x, x_3780.y, x_3780.z) * vec3<f32>(x_3782.x, x_3782.y, x_3782.z)) + vec3<f32>(x_3785.x, x_3785.y, x_3785.z));
  let x_3788 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_3787.x, x_3787.y, x_3787.z, x_3788.w);
  let x_3791 : f32 = u_xlat0.x;
  let x_3793 : f32 = x_83.unity_LightData.z;
  u_xlat0.x = (x_3791 * x_3793);
  let x_3796 : vec3<f32> = u_xlat26;
  let x_3798 : vec4<f32> = x_149.x_MainLightPosition;
  u_xlat3.x = dot(x_3796, vec3<f32>(x_3798.x, x_3798.y, x_3798.z));
  let x_3803 : f32 = u_xlat3.x;
  u_xlat3.x = clamp(x_3803, 0.0f, 1.0f);
  let x_3807 : f32 = u_xlat0.x;
  let x_3809 : f32 = u_xlat3.x;
  u_xlat0.x = (x_3807 * x_3809);
  let x_3812 : vec3<f32> = u_xlat0;
  let x_3814 : vec3<f32> = u_xlat28;
  u_xlat28 = (vec3<f32>(x_3812.x, x_3812.x, x_3812.x) * x_3814);
  let x_3816 : vec3<f32> = u_xlat4;
  let x_3818 : vec4<f32> = x_149.x_MainLightPosition;
  let x_3820 : vec3<f32> = (x_3816 + vec3<f32>(x_3818.x, x_3818.y, x_3818.z));
  let x_3821 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3820.x, x_3820.y, x_3820.z, x_3821.w);
  let x_3823 : vec4<f32> = u_xlat8;
  let x_3825 : vec4<f32> = u_xlat8;
  u_xlat0.x = dot(vec3<f32>(x_3823.x, x_3823.y, x_3823.z), vec3<f32>(x_3825.x, x_3825.y, x_3825.z));
  let x_3830 : f32 = u_xlat0.x;
  u_xlat0.x = max(x_3830, 1.17549435e-38f);
  let x_3834 : f32 = u_xlat0.x;
  u_xlat0.x = inverseSqrt(x_3834);
  let x_3837 : vec3<f32> = u_xlat0;
  let x_3839 : vec4<f32> = u_xlat8;
  let x_3841 : vec3<f32> = (vec3<f32>(x_3837.x, x_3837.x, x_3837.x) * vec3<f32>(x_3839.x, x_3839.y, x_3839.z));
  let x_3842 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3841.x, x_3841.y, x_3841.z, x_3842.w);
  let x_3844 : vec3<f32> = u_xlat26;
  let x_3845 : vec4<f32> = u_xlat8;
  u_xlat0.x = dot(x_3844, vec3<f32>(x_3845.x, x_3845.y, x_3845.z));
  let x_3850 : f32 = u_xlat0.x;
  u_xlat0.x = clamp(x_3850, 0.0f, 1.0f);
  let x_3854 : vec4<f32> = x_149.x_MainLightPosition;
  let x_3856 : vec4<f32> = u_xlat8;
  u_xlat3.x = dot(vec3<f32>(x_3854.x, x_3854.y, x_3854.z), vec3<f32>(x_3856.x, x_3856.y, x_3856.z));
  let x_3861 : f32 = u_xlat3.x;
  u_xlat3.x = clamp(x_3861, 0.0f, 1.0f);
  let x_3865 : f32 = u_xlat0.x;
  let x_3867 : f32 = u_xlat0.x;
  u_xlat0.x = (x_3865 * x_3867);
  let x_3871 : f32 = u_xlat0.x;
  let x_3873 : f32 = u_xlat59.x;
  u_xlat0.x = ((x_3871 * x_3873) + 1.00001001358032226562f);
  let x_3879 : f32 = u_xlat3.x;
  let x_3881 : f32 = u_xlat3.x;
  u_xlat3.x = (x_3879 * x_3881);
  let x_3885 : f32 = u_xlat0.x;
  let x_3887 : f32 = u_xlat0.x;
  u_xlat0.x = (x_3885 * x_3887);
  let x_3891 : f32 = u_xlat3.x;
  u_xlat3.x = max(x_3891, 0.10000000149011611938f);
  let x_3896 : f32 = u_xlat0.x;
  let x_3898 : f32 = u_xlat3.x;
  u_xlat0.x = (x_3896 * x_3898);
  let x_3902 : f32 = u_xlat7.x;
  let x_3904 : f32 = u_xlat0.x;
  u_xlat0.x = (x_3902 * x_3904);
  let x_3907 : f32 = u_xlat83;
  let x_3909 : f32 = u_xlat0.x;
  u_xlat0.x = (x_3907 / x_3909);
  let x_3912 : vec3<f32> = u_xlat0;
  let x_3916 : vec4<f32> = u_xlat6;
  let x_3918 : vec3<f32> = ((vec3<f32>(x_3912.x, x_3912.x, x_3912.x) * vec3<f32>(0.03999999910593032837f, 0.03999999910593032837f, 0.03999999910593032837f)) + vec3<f32>(x_3916.x, x_3916.y, x_3916.z));
  let x_3919 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_3918.x, x_3918.y, x_3918.z, x_3919.w);
  let x_3921 : vec3<f32> = u_xlat28;
  let x_3922 : vec4<f32> = u_xlat8;
  u_xlat28 = (x_3921 * vec3<f32>(x_3922.x, x_3922.y, x_3922.z));
  let x_3926 : f32 = x_149.x_AdditionalLightsCount.x;
  let x_3928 : f32 = x_83.unity_LightData.y;
  u_xlat0.x = min(x_3926, x_3928);
  let x_3932 : f32 = u_xlat0.x;
  u_xlatu0 = bitcast<u32>(i32(x_3932));
  let x_3936 : f32 = u_xlat2.x;
  let x_3939 : f32 = x_372.x_AdditionalShadowFadeParams.x;
  let x_3942 : f32 = x_372.x_AdditionalShadowFadeParams.y;
  u_xlat2.x = ((x_3936 * x_3939) + x_3942);
  let x_3946 : f32 = u_xlat2.x;
  u_xlat2.x = clamp(x_3946, 0.0f, 1.0f);
  let x_3950 : f32 = x_3536.x_AdditionalLightsCookieAtlasTextureFormat;
  let x_3952 : f32 = x_3536.x_AdditionalLightsCookieAtlasTextureFormat;
  let x_3954 : f32 = x_3536.x_AdditionalLightsCookieAtlasTextureFormat;
  let x_3956 : f32 = x_3536.x_AdditionalLightsCookieAtlasTextureFormat;
  let x_3957 : vec4<f32> = vec4<f32>(x_3950, x_3952, x_3954, x_3956);
  let x_3964 : vec4<bool> = (vec4<f32>(x_3957.x, x_3957.y, x_3957.z, x_3957.w) == vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f));
  let x_3965 : vec2<bool> = vec2<bool>(x_3964.x, x_3964.w);
  let x_3966 : vec4<bool> = u_xlatb3;
  u_xlatb3 = vec4<bool>(x_3965.x, x_3966.y, x_3966.z, x_3965.y);
  u_xlat8.x = 0.0f;
  u_xlat8.y = 0.0f;
  u_xlat8.z = 0.0f;
  u_xlatu_loop_1 = 0u;
  loop {
    let x_3977 : u32 = u_xlatu_loop_1;
    let x_3978 : u32 = u_xlatu0;
    if ((x_3977 < x_3978)) {
    } else {
      break;
    }
    let x_3981 : u32 = u_xlatu_loop_1;
    u_xlatu84 = (x_3981 >> 2u);
    let x_3984 : u32 = u_xlatu_loop_1;
    u_xlati85 = bitcast<i32>((x_3984 & 3u));
    let x_3987 : u32 = u_xlatu84;
    let x_3990 : vec4<f32> = x_83.unity_LightIndices[bitcast<i32>(x_3987)];
    let x_4000 : i32 = u_xlati85;
    indexable = array<vec4<u32>, 4u>(vec4<u32>(1065353216u, 0u, 0u, 0u), vec4<u32>(0u, 1065353216u, 0u, 0u), vec4<u32>(0u, 0u, 1065353216u, 0u), vec4<u32>(0u, 0u, 0u, 1065353216u));
    let x_4005 : vec4<u32> = indexable[x_4000];
    u_xlat84 = dot(x_3990, bitcast<vec4<f32>>(x_4005));
    let x_4009 : f32 = u_xlat84;
    u_xlati84 = i32(x_4009);
    let x_4011 : vec3<f32> = vs_INTERP8;
    let x_4022 : i32 = u_xlati84;
    let x_4024 : vec4<f32> = x_4021.x_AdditionalLightsPosition[x_4022];
    let x_4027 : i32 = u_xlati84;
    let x_4029 : vec4<f32> = x_4021.x_AdditionalLightsPosition[x_4027];
    let x_4031 : vec3<f32> = ((-(x_4011) * vec3<f32>(x_4024.w, x_4024.w, x_4024.w)) + vec3<f32>(x_4029.x, x_4029.y, x_4029.z));
    let x_4032 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_4031.x, x_4031.y, x_4031.z, x_4032.w);
    let x_4034 : vec4<f32> = u_xlat9;
    let x_4036 : vec4<f32> = u_xlat9;
    u_xlat85 = dot(vec3<f32>(x_4034.x, x_4034.y, x_4034.z), vec3<f32>(x_4036.x, x_4036.y, x_4036.z));
    let x_4039 : f32 = u_xlat85;
    u_xlat85 = max(x_4039, 0.00006103515625f);
    let x_4043 : f32 = u_xlat85;
    u_xlat86 = inverseSqrt(x_4043);
    let x_4045 : f32 = u_xlat86;
    let x_4047 : vec4<f32> = u_xlat9;
    let x_4049 : vec3<f32> = (vec3<f32>(x_4045, x_4045, x_4045) * vec3<f32>(x_4047.x, x_4047.y, x_4047.z));
    let x_4050 : vec4<f32> = u_xlat10;
    u_xlat10 = vec4<f32>(x_4049.x, x_4049.y, x_4049.z, x_4050.w);
    let x_4052 : f32 = u_xlat85;
    u_xlat87 = (1.0f / x_4052);
    let x_4054 : f32 = u_xlat85;
    let x_4055 : i32 = u_xlati84;
    let x_4057 : f32 = x_4021.x_AdditionalLightsAttenuation[x_4055].x;
    u_xlat85 = (x_4054 * x_4057);
    let x_4059 : f32 = u_xlat85;
    let x_4061 : f32 = u_xlat85;
    u_xlat85 = ((-(x_4059) * x_4061) + 1.0f);
    let x_4064 : f32 = u_xlat85;
    u_xlat85 = max(x_4064, 0.0f);
    let x_4066 : f32 = u_xlat85;
    let x_4067 : f32 = u_xlat85;
    u_xlat85 = (x_4066 * x_4067);
    let x_4069 : f32 = u_xlat85;
    let x_4070 : f32 = u_xlat87;
    u_xlat85 = (x_4069 * x_4070);
    let x_4072 : i32 = u_xlati84;
    let x_4074 : vec4<f32> = x_4021.x_AdditionalLightsSpotDir[x_4072];
    let x_4076 : vec4<f32> = u_xlat10;
    u_xlat87 = dot(vec3<f32>(x_4074.x, x_4074.y, x_4074.z), vec3<f32>(x_4076.x, x_4076.y, x_4076.z));
    let x_4079 : f32 = u_xlat87;
    let x_4080 : i32 = u_xlati84;
    let x_4082 : f32 = x_4021.x_AdditionalLightsAttenuation[x_4080].z;
    let x_4084 : i32 = u_xlati84;
    let x_4086 : f32 = x_4021.x_AdditionalLightsAttenuation[x_4084].w;
    u_xlat87 = ((x_4079 * x_4082) + x_4086);
    let x_4088 : f32 = u_xlat87;
    u_xlat87 = clamp(x_4088, 0.0f, 1.0f);
    let x_4090 : f32 = u_xlat87;
    let x_4091 : f32 = u_xlat87;
    u_xlat87 = (x_4090 * x_4091);
    let x_4093 : f32 = u_xlat85;
    let x_4094 : f32 = u_xlat87;
    u_xlat85 = (x_4093 * x_4094);
    let x_4098 : i32 = u_xlati84;
    let x_4100 : f32 = x_372.x_AdditionalShadowParams[x_4098].w;
    u_xlati87 = i32(x_4100);
    let x_4103 : i32 = u_xlati87;
    u_xlatb88 = (x_4103 >= 0i);
    let x_4105 : bool = u_xlatb88;
    if (x_4105) {
      let x_4109 : i32 = u_xlati84;
      let x_4111 : f32 = x_372.x_AdditionalShadowParams[x_4109].z;
      u_xlatb88 = any(!((vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) == vec4<f32>(x_4111, x_4111, x_4111, x_4111))));
      let x_4115 : bool = u_xlatb88;
      if (x_4115) {
        let x_4119 : vec4<f32> = u_xlat10;
        let x_4122 : vec4<f32> = u_xlat10;
        let x_4125 : vec4<bool> = (abs(vec4<f32>(x_4119.z, x_4119.z, x_4119.y, x_4119.z)) >= abs(vec4<f32>(x_4122.x, x_4122.y, x_4122.x, x_4122.x)));
        let x_4127 : vec3<bool> = vec3<bool>(x_4125.x, x_4125.y, x_4125.z);
        let x_4128 : vec4<bool> = u_xlatb11;
        u_xlatb11 = vec4<bool>(x_4127.x, x_4127.y, x_4127.z, x_4128.w);
        let x_4131 : bool = u_xlatb11.y;
        let x_4133 : bool = u_xlatb11.x;
        u_xlatb88 = (x_4131 & x_4133);
        let x_4135 : vec4<f32> = u_xlat10;
        let x_4138 : vec4<bool> = (-(vec4<f32>(x_4135.z, x_4135.y, x_4135.z, x_4135.x)) < vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f));
        let x_4139 : vec3<bool> = vec3<bool>(x_4138.x, x_4138.y, x_4138.w);
        let x_4140 : vec4<bool> = u_xlatb11;
        u_xlatb11 = vec4<bool>(x_4139.x, x_4139.y, x_4140.z, x_4139.z);
        let x_4143 : bool = u_xlatb11.x;
        u_xlat11.x = select(4.0f, 5.0f, x_4143);
        let x_4148 : bool = u_xlatb11.y;
        u_xlat11.y = select(2.0f, 3.0f, x_4148);
        let x_4153 : bool = u_xlatb11.w;
        u_xlat89 = select(0.0f, 1.0f, x_4153);
        let x_4157 : bool = u_xlatb11.z;
        if (x_4157) {
          let x_4162 : f32 = u_xlat11.y;
          x_4158 = x_4162;
        } else {
          let x_4164 : f32 = u_xlat89;
          x_4158 = x_4164;
        }
        let x_4165 : f32 = x_4158;
        u_xlat37.x = x_4165;
        let x_4168 : bool = u_xlatb88;
        if (x_4168) {
          let x_4173 : f32 = u_xlat11.x;
          x_4169 = x_4173;
        } else {
          let x_4176 : f32 = u_xlat37.x;
          x_4169 = x_4176;
        }
        let x_4177 : f32 = x_4169;
        u_xlat88 = x_4177;
        let x_4178 : i32 = u_xlati84;
        let x_4180 : f32 = x_372.x_AdditionalShadowParams[x_4178].w;
        u_xlat11.x = trunc(x_4180);
        let x_4183 : f32 = u_xlat88;
        let x_4185 : f32 = u_xlat11.x;
        u_xlat88 = (x_4183 + x_4185);
        let x_4187 : f32 = u_xlat88;
        u_xlati87 = i32(x_4187);
      }
      let x_4189 : i32 = u_xlati87;
      u_xlati87 = (x_4189 << bitcast<u32>(2i));
      let x_4191 : vec3<f32> = vs_INTERP8;
      let x_4194 : i32 = u_xlati87;
      let x_4197 : i32 = u_xlati87;
      let x_4201 : vec4<f32> = x_372.x_AdditionalLightsWorldToShadow[((x_4194 + 1i) / 4i)][((x_4197 + 1i) % 4i)];
      u_xlat11 = (vec4<f32>(x_4191.y, x_4191.y, x_4191.y, x_4191.y) * x_4201);
      let x_4203 : i32 = u_xlati87;
      let x_4205 : i32 = u_xlati87;
      let x_4208 : vec4<f32> = x_372.x_AdditionalLightsWorldToShadow[(x_4203 / 4i)][(x_4205 % 4i)];
      let x_4209 : vec3<f32> = vs_INTERP8;
      let x_4212 : vec4<f32> = u_xlat11;
      u_xlat11 = ((x_4208 * vec4<f32>(x_4209.x, x_4209.x, x_4209.x, x_4209.x)) + x_4212);
      let x_4214 : i32 = u_xlati87;
      let x_4217 : i32 = u_xlati87;
      let x_4221 : vec4<f32> = x_372.x_AdditionalLightsWorldToShadow[((x_4214 + 2i) / 4i)][((x_4217 + 2i) % 4i)];
      let x_4222 : vec3<f32> = vs_INTERP8;
      let x_4225 : vec4<f32> = u_xlat11;
      u_xlat11 = ((x_4221 * vec4<f32>(x_4222.z, x_4222.z, x_4222.z, x_4222.z)) + x_4225);
      let x_4227 : vec4<f32> = u_xlat11;
      let x_4228 : i32 = u_xlati87;
      let x_4231 : i32 = u_xlati87;
      let x_4235 : vec4<f32> = x_372.x_AdditionalLightsWorldToShadow[((x_4228 + 3i) / 4i)][((x_4231 + 3i) % 4i)];
      u_xlat11 = (x_4227 + x_4235);
      let x_4237 : vec4<f32> = u_xlat11;
      let x_4239 : vec4<f32> = u_xlat11;
      let x_4241 : vec3<f32> = (vec3<f32>(x_4237.x, x_4237.y, x_4237.z) / vec3<f32>(x_4239.w, x_4239.w, x_4239.w));
      let x_4242 : vec4<f32> = u_xlat11;
      u_xlat11 = vec4<f32>(x_4241.x, x_4241.y, x_4241.z, x_4242.w);
      let x_4245 : i32 = u_xlati84;
      let x_4247 : f32 = x_372.x_AdditionalShadowParams[x_4245].y;
      u_xlatb87 = (0.0f < x_4247);
      let x_4249 : bool = u_xlatb87;
      if (x_4249) {
        let x_4252 : i32 = u_xlati84;
        let x_4254 : f32 = x_372.x_AdditionalShadowParams[x_4252].y;
        u_xlatb87 = (1.0f == x_4254);
        let x_4256 : bool = u_xlatb87;
        if (x_4256) {
          let x_4259 : vec4<f32> = u_xlat11;
          let x_4263 : vec4<f32> = x_372.x_AdditionalShadowOffset0;
          u_xlat12 = (vec4<f32>(x_4259.x, x_4259.y, x_4259.x, x_4259.y) + x_4263);
          let x_4266 : vec4<f32> = u_xlat12;
          let x_4267 : vec2<f32> = vec2<f32>(x_4266.x, x_4266.y);
          let x_4269 : f32 = u_xlat11.z;
          txVec60 = vec3<f32>(x_4267.x, x_4267.y, x_4269);
          let x_4277 : vec3<f32> = txVec60;
          let x_4279 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4277.xy, x_4277.z);
          u_xlat13.x = x_4279;
          let x_4282 : vec4<f32> = u_xlat12;
          let x_4283 : vec2<f32> = vec2<f32>(x_4282.z, x_4282.w);
          let x_4285 : f32 = u_xlat11.z;
          txVec61 = vec3<f32>(x_4283.x, x_4283.y, x_4285);
          let x_4292 : vec3<f32> = txVec61;
          let x_4294 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4292.xy, x_4292.z);
          u_xlat13.y = x_4294;
          let x_4296 : vec4<f32> = u_xlat11;
          let x_4299 : vec4<f32> = x_372.x_AdditionalShadowOffset1;
          u_xlat12 = (vec4<f32>(x_4296.x, x_4296.y, x_4296.x, x_4296.y) + x_4299);
          let x_4302 : vec4<f32> = u_xlat12;
          let x_4303 : vec2<f32> = vec2<f32>(x_4302.x, x_4302.y);
          let x_4305 : f32 = u_xlat11.z;
          txVec62 = vec3<f32>(x_4303.x, x_4303.y, x_4305);
          let x_4312 : vec3<f32> = txVec62;
          let x_4314 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4312.xy, x_4312.z);
          u_xlat13.z = x_4314;
          let x_4317 : vec4<f32> = u_xlat12;
          let x_4318 : vec2<f32> = vec2<f32>(x_4317.z, x_4317.w);
          let x_4320 : f32 = u_xlat11.z;
          txVec63 = vec3<f32>(x_4318.x, x_4318.y, x_4320);
          let x_4327 : vec3<f32> = txVec63;
          let x_4329 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4327.xy, x_4327.z);
          u_xlat13.w = x_4329;
          let x_4331 : vec4<f32> = u_xlat13;
          u_xlat87 = dot(x_4331, vec4<f32>(0.25f, 0.25f, 0.25f, 0.25f));
        } else {
          let x_4334 : i32 = u_xlati84;
          let x_4336 : f32 = x_372.x_AdditionalShadowParams[x_4334].y;
          u_xlatb88 = (2.0f == x_4336);
          let x_4338 : bool = u_xlatb88;
          if (x_4338) {
            let x_4341 : vec4<f32> = u_xlat11;
            let x_4345 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4348 : vec2<f32> = ((vec2<f32>(x_4341.x, x_4341.y) * vec2<f32>(x_4345.z, x_4345.w)) + vec2<f32>(0.5f, 0.5f));
            let x_4349 : vec4<f32> = u_xlat12;
            u_xlat12 = vec4<f32>(x_4348.x, x_4348.y, x_4349.z, x_4349.w);
            let x_4351 : vec4<f32> = u_xlat12;
            let x_4353 : vec2<f32> = floor(vec2<f32>(x_4351.x, x_4351.y));
            let x_4354 : vec4<f32> = u_xlat12;
            u_xlat12 = vec4<f32>(x_4353.x, x_4353.y, x_4354.z, x_4354.w);
            let x_4357 : vec4<f32> = u_xlat11;
            let x_4360 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4363 : vec4<f32> = u_xlat12;
            u_xlat64 = ((vec2<f32>(x_4357.x, x_4357.y) * vec2<f32>(x_4360.z, x_4360.w)) + -(vec2<f32>(x_4363.x, x_4363.y)));
            let x_4367 : vec2<f32> = u_xlat64;
            u_xlat13 = (vec4<f32>(x_4367.x, x_4367.x, x_4367.y, x_4367.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
            let x_4370 : vec4<f32> = u_xlat13;
            let x_4372 : vec4<f32> = u_xlat13;
            u_xlat14 = (vec4<f32>(x_4370.x, x_4370.x, x_4370.z, x_4370.z) * vec4<f32>(x_4372.x, x_4372.x, x_4372.z, x_4372.z));
            let x_4375 : vec4<f32> = u_xlat14;
            let x_4377 : vec2<f32> = (vec2<f32>(x_4375.y, x_4375.w) * vec2<f32>(0.07999999821186065674f, 0.07999999821186065674f));
            let x_4378 : vec4<f32> = u_xlat13;
            u_xlat13 = vec4<f32>(x_4377.x, x_4378.y, x_4377.y, x_4378.w);
            let x_4380 : vec4<f32> = u_xlat14;
            let x_4383 : vec2<f32> = u_xlat64;
            let x_4385 : vec2<f32> = ((vec2<f32>(x_4380.x, x_4380.z) * vec2<f32>(0.5f, 0.5f)) + -(x_4383));
            let x_4386 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4385.x, x_4385.y, x_4386.z, x_4386.w);
            let x_4388 : vec2<f32> = u_xlat64;
            u_xlat66 = (-(x_4388) + vec2<f32>(1.0f, 1.0f));
            let x_4391 : vec2<f32> = u_xlat64;
            let x_4392 : vec2<f32> = min(x_4391, vec2<f32>(0.0f, 0.0f));
            let x_4393 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4392.x, x_4392.y, x_4393.z, x_4393.w);
            let x_4395 : vec4<f32> = u_xlat15;
            let x_4398 : vec4<f32> = u_xlat15;
            let x_4401 : vec2<f32> = u_xlat66;
            let x_4402 : vec2<f32> = ((-(vec2<f32>(x_4395.x, x_4395.y)) * vec2<f32>(x_4398.x, x_4398.y)) + x_4401);
            let x_4403 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4402.x, x_4402.y, x_4403.z, x_4403.w);
            let x_4405 : vec2<f32> = u_xlat64;
            u_xlat64 = max(x_4405, vec2<f32>(0.0f, 0.0f));
            let x_4407 : vec2<f32> = u_xlat64;
            let x_4409 : vec2<f32> = u_xlat64;
            let x_4411 : vec4<f32> = u_xlat13;
            u_xlat64 = ((-(x_4407) * x_4409) + vec2<f32>(x_4411.y, x_4411.w));
            let x_4414 : vec4<f32> = u_xlat15;
            let x_4416 : vec2<f32> = (vec2<f32>(x_4414.x, x_4414.y) + vec2<f32>(1.0f, 1.0f));
            let x_4417 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4416.x, x_4416.y, x_4417.z, x_4417.w);
            let x_4419 : vec2<f32> = u_xlat64;
            u_xlat64 = (x_4419 + vec2<f32>(1.0f, 1.0f));
            let x_4421 : vec4<f32> = u_xlat14;
            let x_4423 : vec2<f32> = (vec2<f32>(x_4421.x, x_4421.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
            let x_4424 : vec4<f32> = u_xlat16;
            u_xlat16 = vec4<f32>(x_4423.x, x_4423.y, x_4424.z, x_4424.w);
            let x_4426 : vec2<f32> = u_xlat66;
            let x_4427 : vec2<f32> = (x_4426 * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
            let x_4428 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4427.x, x_4427.y, x_4428.z, x_4428.w);
            let x_4430 : vec4<f32> = u_xlat15;
            let x_4432 : vec2<f32> = (vec2<f32>(x_4430.x, x_4430.y) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
            let x_4433 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4432.x, x_4432.y, x_4433.z, x_4433.w);
            let x_4435 : vec2<f32> = u_xlat64;
            let x_4436 : vec2<f32> = (x_4435 * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
            let x_4437 : vec4<f32> = u_xlat17;
            u_xlat17 = vec4<f32>(x_4436.x, x_4436.y, x_4437.z, x_4437.w);
            let x_4439 : vec4<f32> = u_xlat13;
            u_xlat64 = (vec2<f32>(x_4439.y, x_4439.w) * vec2<f32>(0.15999999642372131348f, 0.15999999642372131348f));
            let x_4443 : f32 = u_xlat15.x;
            u_xlat16.z = x_4443;
            let x_4446 : f32 = u_xlat64.x;
            u_xlat16.w = x_4446;
            let x_4449 : f32 = u_xlat17.x;
            u_xlat14.z = x_4449;
            let x_4452 : f32 = u_xlat13.x;
            u_xlat14.w = x_4452;
            let x_4454 : vec4<f32> = u_xlat14;
            let x_4456 : vec4<f32> = u_xlat16;
            u_xlat18 = (vec4<f32>(x_4454.z, x_4454.w, x_4454.x, x_4454.z) + vec4<f32>(x_4456.z, x_4456.w, x_4456.x, x_4456.z));
            let x_4460 : f32 = u_xlat16.y;
            u_xlat15.z = x_4460;
            let x_4463 : f32 = u_xlat64.y;
            u_xlat15.w = x_4463;
            let x_4466 : f32 = u_xlat14.y;
            u_xlat17.z = x_4466;
            let x_4469 : f32 = u_xlat13.z;
            u_xlat17.w = x_4469;
            let x_4471 : vec4<f32> = u_xlat15;
            let x_4473 : vec4<f32> = u_xlat17;
            let x_4475 : vec3<f32> = (vec3<f32>(x_4471.z, x_4471.y, x_4471.w) + vec3<f32>(x_4473.z, x_4473.y, x_4473.w));
            let x_4476 : vec4<f32> = u_xlat13;
            u_xlat13 = vec4<f32>(x_4475.x, x_4475.y, x_4475.z, x_4476.w);
            let x_4478 : vec4<f32> = u_xlat14;
            let x_4480 : vec4<f32> = u_xlat18;
            let x_4482 : vec3<f32> = (vec3<f32>(x_4478.x, x_4478.z, x_4478.w) / vec3<f32>(x_4480.z, x_4480.w, x_4480.y));
            let x_4483 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4482.x, x_4482.y, x_4482.z, x_4483.w);
            let x_4485 : vec4<f32> = u_xlat14;
            let x_4487 : vec3<f32> = (vec3<f32>(x_4485.x, x_4485.y, x_4485.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
            let x_4488 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4487.x, x_4487.y, x_4487.z, x_4488.w);
            let x_4490 : vec4<f32> = u_xlat17;
            let x_4492 : vec4<f32> = u_xlat13;
            let x_4494 : vec3<f32> = (vec3<f32>(x_4490.z, x_4490.y, x_4490.w) / vec3<f32>(x_4492.x, x_4492.y, x_4492.z));
            let x_4495 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4494.x, x_4494.y, x_4494.z, x_4495.w);
            let x_4497 : vec4<f32> = u_xlat15;
            let x_4499 : vec3<f32> = (vec3<f32>(x_4497.x, x_4497.y, x_4497.z) + vec3<f32>(-2.5f, -0.5f, 1.5f));
            let x_4500 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4499.x, x_4499.y, x_4499.z, x_4500.w);
            let x_4502 : vec4<f32> = u_xlat14;
            let x_4505 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4507 : vec3<f32> = (vec3<f32>(x_4502.y, x_4502.x, x_4502.z) * vec3<f32>(x_4505.x, x_4505.x, x_4505.x));
            let x_4508 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4507.x, x_4507.y, x_4507.z, x_4508.w);
            let x_4510 : vec4<f32> = u_xlat15;
            let x_4513 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4515 : vec3<f32> = (vec3<f32>(x_4510.x, x_4510.y, x_4510.z) * vec3<f32>(x_4513.y, x_4513.y, x_4513.y));
            let x_4516 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4515.x, x_4515.y, x_4515.z, x_4516.w);
            let x_4519 : f32 = u_xlat15.x;
            u_xlat14.w = x_4519;
            let x_4521 : vec4<f32> = u_xlat12;
            let x_4524 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4527 : vec4<f32> = u_xlat14;
            u_xlat16 = ((vec4<f32>(x_4521.x, x_4521.y, x_4521.x, x_4521.y) * vec4<f32>(x_4524.x, x_4524.y, x_4524.x, x_4524.y)) + vec4<f32>(x_4527.y, x_4527.w, x_4527.x, x_4527.w));
            let x_4530 : vec4<f32> = u_xlat12;
            let x_4533 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4536 : vec4<f32> = u_xlat14;
            u_xlat64 = ((vec2<f32>(x_4530.x, x_4530.y) * vec2<f32>(x_4533.x, x_4533.y)) + vec2<f32>(x_4536.z, x_4536.w));
            let x_4540 : f32 = u_xlat14.y;
            u_xlat15.w = x_4540;
            let x_4542 : vec4<f32> = u_xlat15;
            let x_4543 : vec2<f32> = vec2<f32>(x_4542.y, x_4542.z);
            let x_4544 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4544.x, x_4543.x, x_4544.z, x_4543.y);
            let x_4546 : vec4<f32> = u_xlat12;
            let x_4549 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4552 : vec4<f32> = u_xlat14;
            u_xlat17 = ((vec4<f32>(x_4546.x, x_4546.y, x_4546.x, x_4546.y) * vec4<f32>(x_4549.x, x_4549.y, x_4549.x, x_4549.y)) + vec4<f32>(x_4552.x, x_4552.y, x_4552.z, x_4552.y));
            let x_4555 : vec4<f32> = u_xlat12;
            let x_4558 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4561 : vec4<f32> = u_xlat15;
            u_xlat15 = ((vec4<f32>(x_4555.x, x_4555.y, x_4555.x, x_4555.y) * vec4<f32>(x_4558.x, x_4558.y, x_4558.x, x_4558.y)) + vec4<f32>(x_4561.w, x_4561.y, x_4561.w, x_4561.z));
            let x_4564 : vec4<f32> = u_xlat12;
            let x_4567 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4570 : vec4<f32> = u_xlat14;
            u_xlat14 = ((vec4<f32>(x_4564.x, x_4564.y, x_4564.x, x_4564.y) * vec4<f32>(x_4567.x, x_4567.y, x_4567.x, x_4567.y)) + vec4<f32>(x_4570.x, x_4570.w, x_4570.z, x_4570.w));
            let x_4573 : vec4<f32> = u_xlat13;
            let x_4575 : vec4<f32> = u_xlat18;
            u_xlat19 = (vec4<f32>(x_4573.x, x_4573.x, x_4573.x, x_4573.y) * vec4<f32>(x_4575.z, x_4575.w, x_4575.y, x_4575.z));
            let x_4578 : vec4<f32> = u_xlat13;
            let x_4580 : vec4<f32> = u_xlat18;
            u_xlat20 = (vec4<f32>(x_4578.y, x_4578.y, x_4578.z, x_4578.z) * x_4580);
            let x_4583 : f32 = u_xlat13.z;
            let x_4585 : f32 = u_xlat18.y;
            u_xlat88 = (x_4583 * x_4585);
            let x_4588 : vec4<f32> = u_xlat16;
            let x_4589 : vec2<f32> = vec2<f32>(x_4588.x, x_4588.y);
            let x_4591 : f32 = u_xlat11.z;
            txVec64 = vec3<f32>(x_4589.x, x_4589.y, x_4591);
            let x_4598 : vec3<f32> = txVec64;
            let x_4600 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4598.xy, x_4598.z);
            u_xlat89 = x_4600;
            let x_4602 : vec4<f32> = u_xlat16;
            let x_4603 : vec2<f32> = vec2<f32>(x_4602.z, x_4602.w);
            let x_4605 : f32 = u_xlat11.z;
            txVec65 = vec3<f32>(x_4603.x, x_4603.y, x_4605);
            let x_4612 : vec3<f32> = txVec65;
            let x_4614 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4612.xy, x_4612.z);
            u_xlat12.x = x_4614;
            let x_4617 : f32 = u_xlat12.x;
            let x_4619 : f32 = u_xlat19.y;
            u_xlat12.x = (x_4617 * x_4619);
            let x_4623 : f32 = u_xlat19.x;
            let x_4624 : f32 = u_xlat89;
            let x_4627 : f32 = u_xlat12.x;
            u_xlat89 = ((x_4623 * x_4624) + x_4627);
            let x_4630 : vec2<f32> = u_xlat64;
            let x_4632 : f32 = u_xlat11.z;
            txVec66 = vec3<f32>(x_4630.x, x_4630.y, x_4632);
            let x_4639 : vec3<f32> = txVec66;
            let x_4641 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4639.xy, x_4639.z);
            u_xlat12.x = x_4641;
            let x_4644 : f32 = u_xlat19.z;
            let x_4646 : f32 = u_xlat12.x;
            let x_4648 : f32 = u_xlat89;
            u_xlat89 = ((x_4644 * x_4646) + x_4648);
            let x_4651 : vec4<f32> = u_xlat15;
            let x_4652 : vec2<f32> = vec2<f32>(x_4651.x, x_4651.y);
            let x_4654 : f32 = u_xlat11.z;
            txVec67 = vec3<f32>(x_4652.x, x_4652.y, x_4654);
            let x_4661 : vec3<f32> = txVec67;
            let x_4663 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4661.xy, x_4661.z);
            u_xlat12.x = x_4663;
            let x_4666 : f32 = u_xlat19.w;
            let x_4668 : f32 = u_xlat12.x;
            let x_4670 : f32 = u_xlat89;
            u_xlat89 = ((x_4666 * x_4668) + x_4670);
            let x_4673 : vec4<f32> = u_xlat17;
            let x_4674 : vec2<f32> = vec2<f32>(x_4673.x, x_4673.y);
            let x_4676 : f32 = u_xlat11.z;
            txVec68 = vec3<f32>(x_4674.x, x_4674.y, x_4676);
            let x_4683 : vec3<f32> = txVec68;
            let x_4685 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4683.xy, x_4683.z);
            u_xlat12.x = x_4685;
            let x_4688 : f32 = u_xlat20.x;
            let x_4690 : f32 = u_xlat12.x;
            let x_4692 : f32 = u_xlat89;
            u_xlat89 = ((x_4688 * x_4690) + x_4692);
            let x_4695 : vec4<f32> = u_xlat17;
            let x_4696 : vec2<f32> = vec2<f32>(x_4695.z, x_4695.w);
            let x_4698 : f32 = u_xlat11.z;
            txVec69 = vec3<f32>(x_4696.x, x_4696.y, x_4698);
            let x_4705 : vec3<f32> = txVec69;
            let x_4707 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4705.xy, x_4705.z);
            u_xlat12.x = x_4707;
            let x_4710 : f32 = u_xlat20.y;
            let x_4712 : f32 = u_xlat12.x;
            let x_4714 : f32 = u_xlat89;
            u_xlat89 = ((x_4710 * x_4712) + x_4714);
            let x_4717 : vec4<f32> = u_xlat15;
            let x_4718 : vec2<f32> = vec2<f32>(x_4717.z, x_4717.w);
            let x_4720 : f32 = u_xlat11.z;
            txVec70 = vec3<f32>(x_4718.x, x_4718.y, x_4720);
            let x_4727 : vec3<f32> = txVec70;
            let x_4729 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4727.xy, x_4727.z);
            u_xlat12.x = x_4729;
            let x_4732 : f32 = u_xlat20.z;
            let x_4734 : f32 = u_xlat12.x;
            let x_4736 : f32 = u_xlat89;
            u_xlat89 = ((x_4732 * x_4734) + x_4736);
            let x_4739 : vec4<f32> = u_xlat14;
            let x_4740 : vec2<f32> = vec2<f32>(x_4739.x, x_4739.y);
            let x_4742 : f32 = u_xlat11.z;
            txVec71 = vec3<f32>(x_4740.x, x_4740.y, x_4742);
            let x_4749 : vec3<f32> = txVec71;
            let x_4751 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4749.xy, x_4749.z);
            u_xlat12.x = x_4751;
            let x_4754 : f32 = u_xlat20.w;
            let x_4756 : f32 = u_xlat12.x;
            let x_4758 : f32 = u_xlat89;
            u_xlat89 = ((x_4754 * x_4756) + x_4758);
            let x_4761 : vec4<f32> = u_xlat14;
            let x_4762 : vec2<f32> = vec2<f32>(x_4761.z, x_4761.w);
            let x_4764 : f32 = u_xlat11.z;
            txVec72 = vec3<f32>(x_4762.x, x_4762.y, x_4764);
            let x_4771 : vec3<f32> = txVec72;
            let x_4773 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_4771.xy, x_4771.z);
            u_xlat12.x = x_4773;
            let x_4775 : f32 = u_xlat88;
            let x_4777 : f32 = u_xlat12.x;
            let x_4779 : f32 = u_xlat89;
            u_xlat87 = ((x_4775 * x_4777) + x_4779);
          } else {
            let x_4782 : vec4<f32> = u_xlat11;
            let x_4785 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4788 : vec2<f32> = ((vec2<f32>(x_4782.x, x_4782.y) * vec2<f32>(x_4785.z, x_4785.w)) + vec2<f32>(0.5f, 0.5f));
            let x_4789 : vec4<f32> = u_xlat12;
            u_xlat12 = vec4<f32>(x_4788.x, x_4788.y, x_4789.z, x_4789.w);
            let x_4791 : vec4<f32> = u_xlat12;
            let x_4793 : vec2<f32> = floor(vec2<f32>(x_4791.x, x_4791.y));
            let x_4794 : vec4<f32> = u_xlat12;
            u_xlat12 = vec4<f32>(x_4793.x, x_4793.y, x_4794.z, x_4794.w);
            let x_4796 : vec4<f32> = u_xlat11;
            let x_4799 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4802 : vec4<f32> = u_xlat12;
            u_xlat64 = ((vec2<f32>(x_4796.x, x_4796.y) * vec2<f32>(x_4799.z, x_4799.w)) + -(vec2<f32>(x_4802.x, x_4802.y)));
            let x_4806 : vec2<f32> = u_xlat64;
            u_xlat13 = (vec4<f32>(x_4806.x, x_4806.x, x_4806.y, x_4806.y) + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
            let x_4809 : vec4<f32> = u_xlat13;
            let x_4811 : vec4<f32> = u_xlat13;
            u_xlat14 = (vec4<f32>(x_4809.x, x_4809.x, x_4809.z, x_4809.z) * vec4<f32>(x_4811.x, x_4811.x, x_4811.z, x_4811.z));
            let x_4814 : vec4<f32> = u_xlat14;
            let x_4816 : vec2<f32> = (vec2<f32>(x_4814.y, x_4814.w) * vec2<f32>(0.04081600159406661987f, 0.04081600159406661987f));
            let x_4817 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4817.x, x_4816.x, x_4817.z, x_4816.y);
            let x_4819 : vec4<f32> = u_xlat14;
            let x_4822 : vec2<f32> = u_xlat64;
            let x_4824 : vec2<f32> = ((vec2<f32>(x_4819.x, x_4819.z) * vec2<f32>(0.5f, 0.5f)) + -(x_4822));
            let x_4825 : vec4<f32> = u_xlat13;
            u_xlat13 = vec4<f32>(x_4824.x, x_4825.y, x_4824.y, x_4825.w);
            let x_4827 : vec2<f32> = u_xlat64;
            let x_4829 : vec2<f32> = (-(x_4827) + vec2<f32>(1.0f, 1.0f));
            let x_4830 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4829.x, x_4829.y, x_4830.z, x_4830.w);
            let x_4832 : vec2<f32> = u_xlat64;
            u_xlat66 = min(x_4832, vec2<f32>(0.0f, 0.0f));
            let x_4834 : vec2<f32> = u_xlat66;
            let x_4836 : vec2<f32> = u_xlat66;
            let x_4838 : vec4<f32> = u_xlat14;
            let x_4840 : vec2<f32> = ((-(x_4834) * x_4836) + vec2<f32>(x_4838.x, x_4838.y));
            let x_4841 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4840.x, x_4840.y, x_4841.z, x_4841.w);
            let x_4843 : vec2<f32> = u_xlat64;
            u_xlat66 = max(x_4843, vec2<f32>(0.0f, 0.0f));
            let x_4846 : vec2<f32> = u_xlat66;
            let x_4848 : vec2<f32> = u_xlat66;
            let x_4850 : vec4<f32> = u_xlat13;
            let x_4852 : vec2<f32> = ((-(x_4846) * x_4848) + vec2<f32>(x_4850.y, x_4850.w));
            let x_4853 : vec3<f32> = u_xlat39;
            u_xlat39 = vec3<f32>(x_4852.x, x_4853.y, x_4852.y);
            let x_4855 : vec4<f32> = u_xlat14;
            let x_4857 : vec2<f32> = (vec2<f32>(x_4855.x, x_4855.y) + vec2<f32>(2.0f, 2.0f));
            let x_4858 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4857.x, x_4857.y, x_4858.z, x_4858.w);
            let x_4860 : vec3<f32> = u_xlat39;
            let x_4862 : vec2<f32> = (vec2<f32>(x_4860.x, x_4860.z) + vec2<f32>(2.0f, 2.0f));
            let x_4863 : vec4<f32> = u_xlat13;
            u_xlat13 = vec4<f32>(x_4863.x, x_4862.x, x_4863.z, x_4862.y);
            let x_4866 : f32 = u_xlat13.y;
            u_xlat16.z = (x_4866 * 0.08163200318813323975f);
            let x_4869 : vec4<f32> = u_xlat13;
            let x_4871 : vec3<f32> = (vec3<f32>(x_4869.z, x_4869.x, x_4869.w) * vec3<f32>(0.08163200318813323975f, 0.08163200318813323975f, 0.08163200318813323975f));
            let x_4872 : vec4<f32> = u_xlat17;
            u_xlat17 = vec4<f32>(x_4871.x, x_4871.y, x_4871.z, x_4872.w);
            let x_4874 : vec4<f32> = u_xlat14;
            let x_4876 : vec2<f32> = (vec2<f32>(x_4874.x, x_4874.y) * vec2<f32>(0.08163200318813323975f, 0.08163200318813323975f));
            let x_4877 : vec4<f32> = u_xlat13;
            u_xlat13 = vec4<f32>(x_4876.x, x_4876.y, x_4877.z, x_4877.w);
            let x_4880 : f32 = u_xlat17.y;
            u_xlat16.x = x_4880;
            let x_4882 : vec2<f32> = u_xlat64;
            let x_4885 : vec2<f32> = ((vec2<f32>(x_4882.x, x_4882.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
            let x_4886 : vec4<f32> = u_xlat16;
            u_xlat16 = vec4<f32>(x_4886.x, x_4885.x, x_4886.z, x_4885.y);
            let x_4888 : vec2<f32> = u_xlat64;
            let x_4891 : vec2<f32> = ((vec2<f32>(x_4888.x, x_4888.x) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
            let x_4892 : vec4<f32> = u_xlat14;
            u_xlat14 = vec4<f32>(x_4891.x, x_4892.y, x_4891.y, x_4892.w);
            let x_4895 : f32 = u_xlat13.x;
            u_xlat14.y = x_4895;
            let x_4898 : f32 = u_xlat15.y;
            u_xlat14.w = x_4898;
            let x_4900 : vec4<f32> = u_xlat14;
            let x_4901 : vec4<f32> = u_xlat16;
            u_xlat16 = (x_4900 + x_4901);
            let x_4903 : vec2<f32> = u_xlat64;
            let x_4906 : vec2<f32> = ((vec2<f32>(x_4903.y, x_4903.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
            let x_4907 : vec4<f32> = u_xlat17;
            u_xlat17 = vec4<f32>(x_4907.x, x_4906.x, x_4907.z, x_4906.y);
            let x_4909 : vec2<f32> = u_xlat64;
            let x_4912 : vec2<f32> = ((vec2<f32>(x_4909.y, x_4909.y) * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
            let x_4913 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_4912.x, x_4913.y, x_4912.y, x_4913.w);
            let x_4916 : f32 = u_xlat13.y;
            u_xlat15.y = x_4916;
            let x_4918 : vec4<f32> = u_xlat15;
            let x_4919 : vec4<f32> = u_xlat17;
            u_xlat13 = (x_4918 + x_4919);
            let x_4921 : vec4<f32> = u_xlat14;
            let x_4922 : vec4<f32> = u_xlat16;
            u_xlat14 = (x_4921 / x_4922);
            let x_4924 : vec4<f32> = u_xlat14;
            u_xlat14 = (x_4924 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
            let x_4926 : vec4<f32> = u_xlat15;
            let x_4927 : vec4<f32> = u_xlat13;
            u_xlat15 = (x_4926 / x_4927);
            let x_4929 : vec4<f32> = u_xlat15;
            u_xlat15 = (x_4929 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
            let x_4931 : vec4<f32> = u_xlat14;
            let x_4934 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            u_xlat14 = (vec4<f32>(x_4931.w, x_4931.x, x_4931.y, x_4931.z) * vec4<f32>(x_4934.x, x_4934.x, x_4934.x, x_4934.x));
            let x_4937 : vec4<f32> = u_xlat15;
            let x_4940 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            u_xlat15 = (vec4<f32>(x_4937.x, x_4937.w, x_4937.y, x_4937.z) * vec4<f32>(x_4940.y, x_4940.y, x_4940.y, x_4940.y));
            let x_4943 : vec4<f32> = u_xlat14;
            let x_4944 : vec3<f32> = vec3<f32>(x_4943.y, x_4943.z, x_4943.w);
            let x_4945 : vec4<f32> = u_xlat17;
            u_xlat17 = vec4<f32>(x_4944.x, x_4945.y, x_4944.y, x_4944.z);
            let x_4948 : f32 = u_xlat15.x;
            u_xlat17.y = x_4948;
            let x_4950 : vec4<f32> = u_xlat12;
            let x_4953 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4956 : vec4<f32> = u_xlat17;
            u_xlat18 = ((vec4<f32>(x_4950.x, x_4950.y, x_4950.x, x_4950.y) * vec4<f32>(x_4953.x, x_4953.y, x_4953.x, x_4953.y)) + vec4<f32>(x_4956.x, x_4956.y, x_4956.z, x_4956.y));
            let x_4959 : vec4<f32> = u_xlat12;
            let x_4962 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4965 : vec4<f32> = u_xlat17;
            u_xlat64 = ((vec2<f32>(x_4959.x, x_4959.y) * vec2<f32>(x_4962.x, x_4962.y)) + vec2<f32>(x_4965.w, x_4965.y));
            let x_4969 : f32 = u_xlat17.y;
            u_xlat14.y = x_4969;
            let x_4972 : f32 = u_xlat15.z;
            u_xlat17.y = x_4972;
            let x_4974 : vec4<f32> = u_xlat12;
            let x_4977 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4980 : vec4<f32> = u_xlat17;
            u_xlat19 = ((vec4<f32>(x_4974.x, x_4974.y, x_4974.x, x_4974.y) * vec4<f32>(x_4977.x, x_4977.y, x_4977.x, x_4977.y)) + vec4<f32>(x_4980.x, x_4980.y, x_4980.z, x_4980.y));
            let x_4983 : vec4<f32> = u_xlat12;
            let x_4986 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_4989 : vec4<f32> = u_xlat17;
            let x_4991 : vec2<f32> = ((vec2<f32>(x_4983.x, x_4983.y) * vec2<f32>(x_4986.x, x_4986.y)) + vec2<f32>(x_4989.w, x_4989.y));
            let x_4992 : vec4<f32> = u_xlat20;
            u_xlat20 = vec4<f32>(x_4991.x, x_4991.y, x_4992.z, x_4992.w);
            let x_4995 : f32 = u_xlat17.y;
            u_xlat14.z = x_4995;
            let x_4997 : vec4<f32> = u_xlat12;
            let x_5000 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5003 : vec4<f32> = u_xlat14;
            u_xlat21 = ((vec4<f32>(x_4997.x, x_4997.y, x_4997.x, x_4997.y) * vec4<f32>(x_5000.x, x_5000.y, x_5000.x, x_5000.y)) + vec4<f32>(x_5003.x, x_5003.y, x_5003.x, x_5003.z));
            let x_5007 : f32 = u_xlat15.w;
            u_xlat17.y = x_5007;
            let x_5010 : vec4<f32> = u_xlat12;
            let x_5013 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5016 : vec4<f32> = u_xlat17;
            u_xlat22 = ((vec4<f32>(x_5010.x, x_5010.y, x_5010.x, x_5010.y) * vec4<f32>(x_5013.x, x_5013.y, x_5013.x, x_5013.y)) + vec4<f32>(x_5016.x, x_5016.y, x_5016.z, x_5016.y));
            let x_5020 : vec4<f32> = u_xlat12;
            let x_5023 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5026 : vec4<f32> = u_xlat17;
            u_xlat40 = ((vec2<f32>(x_5020.x, x_5020.y) * vec2<f32>(x_5023.x, x_5023.y)) + vec2<f32>(x_5026.w, x_5026.y));
            let x_5030 : f32 = u_xlat17.y;
            u_xlat14.w = x_5030;
            let x_5033 : vec4<f32> = u_xlat12;
            let x_5036 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5039 : vec4<f32> = u_xlat14;
            u_xlat72 = ((vec2<f32>(x_5033.x, x_5033.y) * vec2<f32>(x_5036.x, x_5036.y)) + vec2<f32>(x_5039.x, x_5039.w));
            let x_5042 : vec4<f32> = u_xlat17;
            let x_5043 : vec3<f32> = vec3<f32>(x_5042.x, x_5042.z, x_5042.w);
            let x_5044 : vec4<f32> = u_xlat15;
            u_xlat15 = vec4<f32>(x_5043.x, x_5044.y, x_5043.y, x_5043.z);
            let x_5046 : vec4<f32> = u_xlat12;
            let x_5049 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5052 : vec4<f32> = u_xlat15;
            u_xlat17 = ((vec4<f32>(x_5046.x, x_5046.y, x_5046.x, x_5046.y) * vec4<f32>(x_5049.x, x_5049.y, x_5049.x, x_5049.y)) + vec4<f32>(x_5052.x, x_5052.y, x_5052.z, x_5052.y));
            let x_5056 : vec4<f32> = u_xlat12;
            let x_5059 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5062 : vec4<f32> = u_xlat15;
            u_xlat67 = ((vec2<f32>(x_5056.x, x_5056.y) * vec2<f32>(x_5059.x, x_5059.y)) + vec2<f32>(x_5062.w, x_5062.y));
            let x_5066 : f32 = u_xlat14.x;
            u_xlat15.x = x_5066;
            let x_5068 : vec4<f32> = u_xlat12;
            let x_5071 : vec4<f32> = x_372.x_AdditionalShadowmapSize;
            let x_5074 : vec4<f32> = u_xlat15;
            let x_5076 : vec2<f32> = ((vec2<f32>(x_5068.x, x_5068.y) * vec2<f32>(x_5071.x, x_5071.y)) + vec2<f32>(x_5074.x, x_5074.y));
            let x_5077 : vec4<f32> = u_xlat12;
            u_xlat12 = vec4<f32>(x_5076.x, x_5076.y, x_5077.z, x_5077.w);
            let x_5080 : vec4<f32> = u_xlat13;
            let x_5082 : vec4<f32> = u_xlat16;
            u_xlat23 = (vec4<f32>(x_5080.x, x_5080.x, x_5080.x, x_5080.x) * x_5082);
            let x_5085 : vec4<f32> = u_xlat13;
            let x_5087 : vec4<f32> = u_xlat16;
            u_xlat24 = (vec4<f32>(x_5085.y, x_5085.y, x_5085.y, x_5085.y) * x_5087);
            let x_5090 : vec4<f32> = u_xlat13;
            let x_5092 : vec4<f32> = u_xlat16;
            u_xlat25 = (vec4<f32>(x_5090.z, x_5090.z, x_5090.z, x_5090.z) * x_5092);
            let x_5094 : vec4<f32> = u_xlat13;
            let x_5096 : vec4<f32> = u_xlat16;
            u_xlat13 = (vec4<f32>(x_5094.w, x_5094.w, x_5094.w, x_5094.w) * x_5096);
            let x_5099 : vec4<f32> = u_xlat18;
            let x_5100 : vec2<f32> = vec2<f32>(x_5099.x, x_5099.y);
            let x_5102 : f32 = u_xlat11.z;
            txVec73 = vec3<f32>(x_5100.x, x_5100.y, x_5102);
            let x_5109 : vec3<f32> = txVec73;
            let x_5111 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5109.xy, x_5109.z);
            u_xlat88 = x_5111;
            let x_5113 : vec4<f32> = u_xlat18;
            let x_5114 : vec2<f32> = vec2<f32>(x_5113.z, x_5113.w);
            let x_5116 : f32 = u_xlat11.z;
            txVec74 = vec3<f32>(x_5114.x, x_5114.y, x_5116);
            let x_5123 : vec3<f32> = txVec74;
            let x_5125 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5123.xy, x_5123.z);
            u_xlat89 = x_5125;
            let x_5126 : f32 = u_xlat89;
            let x_5128 : f32 = u_xlat23.y;
            u_xlat89 = (x_5126 * x_5128);
            let x_5131 : f32 = u_xlat23.x;
            let x_5132 : f32 = u_xlat88;
            let x_5134 : f32 = u_xlat89;
            u_xlat88 = ((x_5131 * x_5132) + x_5134);
            let x_5137 : vec2<f32> = u_xlat64;
            let x_5139 : f32 = u_xlat11.z;
            txVec75 = vec3<f32>(x_5137.x, x_5137.y, x_5139);
            let x_5146 : vec3<f32> = txVec75;
            let x_5148 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5146.xy, x_5146.z);
            u_xlat89 = x_5148;
            let x_5150 : f32 = u_xlat23.z;
            let x_5151 : f32 = u_xlat89;
            let x_5153 : f32 = u_xlat88;
            u_xlat88 = ((x_5150 * x_5151) + x_5153);
            let x_5156 : vec4<f32> = u_xlat21;
            let x_5157 : vec2<f32> = vec2<f32>(x_5156.x, x_5156.y);
            let x_5159 : f32 = u_xlat11.z;
            txVec76 = vec3<f32>(x_5157.x, x_5157.y, x_5159);
            let x_5166 : vec3<f32> = txVec76;
            let x_5168 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5166.xy, x_5166.z);
            u_xlat89 = x_5168;
            let x_5170 : f32 = u_xlat23.w;
            let x_5171 : f32 = u_xlat89;
            let x_5173 : f32 = u_xlat88;
            u_xlat88 = ((x_5170 * x_5171) + x_5173);
            let x_5176 : vec4<f32> = u_xlat19;
            let x_5177 : vec2<f32> = vec2<f32>(x_5176.x, x_5176.y);
            let x_5179 : f32 = u_xlat11.z;
            txVec77 = vec3<f32>(x_5177.x, x_5177.y, x_5179);
            let x_5186 : vec3<f32> = txVec77;
            let x_5188 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5186.xy, x_5186.z);
            u_xlat89 = x_5188;
            let x_5190 : f32 = u_xlat24.x;
            let x_5191 : f32 = u_xlat89;
            let x_5193 : f32 = u_xlat88;
            u_xlat88 = ((x_5190 * x_5191) + x_5193);
            let x_5196 : vec4<f32> = u_xlat19;
            let x_5197 : vec2<f32> = vec2<f32>(x_5196.z, x_5196.w);
            let x_5199 : f32 = u_xlat11.z;
            txVec78 = vec3<f32>(x_5197.x, x_5197.y, x_5199);
            let x_5206 : vec3<f32> = txVec78;
            let x_5208 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5206.xy, x_5206.z);
            u_xlat89 = x_5208;
            let x_5210 : f32 = u_xlat24.y;
            let x_5211 : f32 = u_xlat89;
            let x_5213 : f32 = u_xlat88;
            u_xlat88 = ((x_5210 * x_5211) + x_5213);
            let x_5216 : vec4<f32> = u_xlat20;
            let x_5217 : vec2<f32> = vec2<f32>(x_5216.x, x_5216.y);
            let x_5219 : f32 = u_xlat11.z;
            txVec79 = vec3<f32>(x_5217.x, x_5217.y, x_5219);
            let x_5226 : vec3<f32> = txVec79;
            let x_5228 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5226.xy, x_5226.z);
            u_xlat89 = x_5228;
            let x_5230 : f32 = u_xlat24.z;
            let x_5231 : f32 = u_xlat89;
            let x_5233 : f32 = u_xlat88;
            u_xlat88 = ((x_5230 * x_5231) + x_5233);
            let x_5236 : vec4<f32> = u_xlat21;
            let x_5237 : vec2<f32> = vec2<f32>(x_5236.z, x_5236.w);
            let x_5239 : f32 = u_xlat11.z;
            txVec80 = vec3<f32>(x_5237.x, x_5237.y, x_5239);
            let x_5246 : vec3<f32> = txVec80;
            let x_5248 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5246.xy, x_5246.z);
            u_xlat89 = x_5248;
            let x_5250 : f32 = u_xlat24.w;
            let x_5251 : f32 = u_xlat89;
            let x_5253 : f32 = u_xlat88;
            u_xlat88 = ((x_5250 * x_5251) + x_5253);
            let x_5256 : vec4<f32> = u_xlat22;
            let x_5257 : vec2<f32> = vec2<f32>(x_5256.x, x_5256.y);
            let x_5259 : f32 = u_xlat11.z;
            txVec81 = vec3<f32>(x_5257.x, x_5257.y, x_5259);
            let x_5266 : vec3<f32> = txVec81;
            let x_5268 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5266.xy, x_5266.z);
            u_xlat89 = x_5268;
            let x_5270 : f32 = u_xlat25.x;
            let x_5271 : f32 = u_xlat89;
            let x_5273 : f32 = u_xlat88;
            u_xlat88 = ((x_5270 * x_5271) + x_5273);
            let x_5276 : vec4<f32> = u_xlat22;
            let x_5277 : vec2<f32> = vec2<f32>(x_5276.z, x_5276.w);
            let x_5279 : f32 = u_xlat11.z;
            txVec82 = vec3<f32>(x_5277.x, x_5277.y, x_5279);
            let x_5286 : vec3<f32> = txVec82;
            let x_5288 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5286.xy, x_5286.z);
            u_xlat89 = x_5288;
            let x_5290 : f32 = u_xlat25.y;
            let x_5291 : f32 = u_xlat89;
            let x_5293 : f32 = u_xlat88;
            u_xlat88 = ((x_5290 * x_5291) + x_5293);
            let x_5296 : vec2<f32> = u_xlat40;
            let x_5298 : f32 = u_xlat11.z;
            txVec83 = vec3<f32>(x_5296.x, x_5296.y, x_5298);
            let x_5305 : vec3<f32> = txVec83;
            let x_5307 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5305.xy, x_5305.z);
            u_xlat89 = x_5307;
            let x_5309 : f32 = u_xlat25.z;
            let x_5310 : f32 = u_xlat89;
            let x_5312 : f32 = u_xlat88;
            u_xlat88 = ((x_5309 * x_5310) + x_5312);
            let x_5315 : vec2<f32> = u_xlat72;
            let x_5317 : f32 = u_xlat11.z;
            txVec84 = vec3<f32>(x_5315.x, x_5315.y, x_5317);
            let x_5324 : vec3<f32> = txVec84;
            let x_5326 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5324.xy, x_5324.z);
            u_xlat89 = x_5326;
            let x_5328 : f32 = u_xlat25.w;
            let x_5329 : f32 = u_xlat89;
            let x_5331 : f32 = u_xlat88;
            u_xlat88 = ((x_5328 * x_5329) + x_5331);
            let x_5334 : vec4<f32> = u_xlat17;
            let x_5335 : vec2<f32> = vec2<f32>(x_5334.x, x_5334.y);
            let x_5337 : f32 = u_xlat11.z;
            txVec85 = vec3<f32>(x_5335.x, x_5335.y, x_5337);
            let x_5344 : vec3<f32> = txVec85;
            let x_5346 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5344.xy, x_5344.z);
            u_xlat89 = x_5346;
            let x_5348 : f32 = u_xlat13.x;
            let x_5349 : f32 = u_xlat89;
            let x_5351 : f32 = u_xlat88;
            u_xlat88 = ((x_5348 * x_5349) + x_5351);
            let x_5354 : vec4<f32> = u_xlat17;
            let x_5355 : vec2<f32> = vec2<f32>(x_5354.z, x_5354.w);
            let x_5357 : f32 = u_xlat11.z;
            txVec86 = vec3<f32>(x_5355.x, x_5355.y, x_5357);
            let x_5364 : vec3<f32> = txVec86;
            let x_5366 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5364.xy, x_5364.z);
            u_xlat89 = x_5366;
            let x_5368 : f32 = u_xlat13.y;
            let x_5369 : f32 = u_xlat89;
            let x_5371 : f32 = u_xlat88;
            u_xlat88 = ((x_5368 * x_5369) + x_5371);
            let x_5374 : vec2<f32> = u_xlat67;
            let x_5376 : f32 = u_xlat11.z;
            txVec87 = vec3<f32>(x_5374.x, x_5374.y, x_5376);
            let x_5383 : vec3<f32> = txVec87;
            let x_5385 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5383.xy, x_5383.z);
            u_xlat89 = x_5385;
            let x_5387 : f32 = u_xlat13.z;
            let x_5388 : f32 = u_xlat89;
            let x_5390 : f32 = u_xlat88;
            u_xlat88 = ((x_5387 * x_5388) + x_5390);
            let x_5393 : vec4<f32> = u_xlat12;
            let x_5394 : vec2<f32> = vec2<f32>(x_5393.x, x_5393.y);
            let x_5396 : f32 = u_xlat11.z;
            txVec88 = vec3<f32>(x_5394.x, x_5394.y, x_5396);
            let x_5403 : vec3<f32> = txVec88;
            let x_5405 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5403.xy, x_5403.z);
            u_xlat89 = x_5405;
            let x_5407 : f32 = u_xlat13.w;
            let x_5408 : f32 = u_xlat89;
            let x_5410 : f32 = u_xlat88;
            u_xlat87 = ((x_5407 * x_5408) + x_5410);
          }
        }
      } else {
        let x_5414 : vec4<f32> = u_xlat11;
        let x_5415 : vec2<f32> = vec2<f32>(x_5414.x, x_5414.y);
        let x_5417 : f32 = u_xlat11.z;
        txVec89 = vec3<f32>(x_5415.x, x_5415.y, x_5417);
        let x_5424 : vec3<f32> = txVec89;
        let x_5426 : f32 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, x_5424.xy, x_5424.z);
        u_xlat87 = x_5426;
      }
      let x_5427 : i32 = u_xlati84;
      let x_5429 : f32 = x_372.x_AdditionalShadowParams[x_5427].x;
      u_xlat88 = (1.0f + -(x_5429));
      let x_5432 : f32 = u_xlat87;
      let x_5433 : i32 = u_xlati84;
      let x_5435 : f32 = x_372.x_AdditionalShadowParams[x_5433].x;
      let x_5437 : f32 = u_xlat88;
      u_xlat87 = ((x_5432 * x_5435) + x_5437);
      let x_5440 : f32 = u_xlat11.z;
      u_xlatb88 = (0.0f >= x_5440);
      let x_5443 : f32 = u_xlat11.z;
      u_xlatb11.x = (x_5443 >= 1.0f);
      let x_5446 : bool = u_xlatb88;
      let x_5448 : bool = u_xlatb11.x;
      u_xlatb88 = (x_5446 | x_5448);
      let x_5450 : bool = u_xlatb88;
      let x_5451 : f32 = u_xlat87;
      u_xlat87 = select(x_5451, 1.0f, x_5450);
    } else {
      u_xlat87 = 1.0f;
    }
    let x_5454 : f32 = u_xlat87;
    u_xlat88 = (-(x_5454) + 1.0f);
    let x_5458 : f32 = u_xlat2.x;
    let x_5459 : f32 = u_xlat88;
    let x_5461 : f32 = u_xlat87;
    u_xlat87 = ((x_5458 * x_5459) + x_5461);
    let x_5464 : i32 = u_xlati84;
    u_xlati88 = (1i << bitcast<u32>((x_5464 & 31i)));
    let x_5468 : i32 = u_xlati88;
    let x_5471 : f32 = x_3536.x_AdditionalLightsCookieEnableBits;
    u_xlati88 = bitcast<i32>((bitcast<u32>(x_5468) & bitcast<u32>(x_5471)));
    let x_5475 : i32 = u_xlati88;
    if ((x_5475 != 0i)) {
      let x_5479 : i32 = u_xlati84;
      let x_5481 : f32 = x_3536.x_AdditionalLightsLightTypes[x_5479].el;
      u_xlati88 = i32(x_5481);
      let x_5484 : i32 = u_xlati88;
      u_xlati11 = select(1i, 0i, (x_5484 != 0i));
      let x_5488 : i32 = u_xlati84;
      u_xlati37 = (x_5488 << bitcast<u32>(2i));
      let x_5490 : i32 = u_xlati11;
      if ((x_5490 != 0i)) {
        let x_5494 : vec3<f32> = vs_INTERP8;
        let x_5496 : i32 = u_xlati37;
        let x_5499 : i32 = u_xlati37;
        let x_5503 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5496 + 1i) / 4i)][((x_5499 + 1i) % 4i)];
        let x_5505 : vec3<f32> = (vec3<f32>(x_5494.y, x_5494.y, x_5494.y) * vec3<f32>(x_5503.x, x_5503.y, x_5503.w));
        let x_5506 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5505.x, x_5506.y, x_5505.y, x_5505.z);
        let x_5508 : i32 = u_xlati37;
        let x_5510 : i32 = u_xlati37;
        let x_5513 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[(x_5508 / 4i)][(x_5510 % 4i)];
        let x_5515 : vec3<f32> = vs_INTERP8;
        let x_5518 : vec4<f32> = u_xlat11;
        let x_5520 : vec3<f32> = ((vec3<f32>(x_5513.x, x_5513.y, x_5513.w) * vec3<f32>(x_5515.x, x_5515.x, x_5515.x)) + vec3<f32>(x_5518.x, x_5518.z, x_5518.w));
        let x_5521 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5520.x, x_5521.y, x_5520.y, x_5520.z);
        let x_5523 : i32 = u_xlati37;
        let x_5526 : i32 = u_xlati37;
        let x_5530 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5523 + 2i) / 4i)][((x_5526 + 2i) % 4i)];
        let x_5532 : vec3<f32> = vs_INTERP8;
        let x_5535 : vec4<f32> = u_xlat11;
        let x_5537 : vec3<f32> = ((vec3<f32>(x_5530.x, x_5530.y, x_5530.w) * vec3<f32>(x_5532.z, x_5532.z, x_5532.z)) + vec3<f32>(x_5535.x, x_5535.z, x_5535.w));
        let x_5538 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5537.x, x_5538.y, x_5537.y, x_5537.z);
        let x_5540 : vec4<f32> = u_xlat11;
        let x_5542 : i32 = u_xlati37;
        let x_5545 : i32 = u_xlati37;
        let x_5549 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5542 + 3i) / 4i)][((x_5545 + 3i) % 4i)];
        let x_5551 : vec3<f32> = (vec3<f32>(x_5540.x, x_5540.z, x_5540.w) + vec3<f32>(x_5549.x, x_5549.y, x_5549.w));
        let x_5552 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5551.x, x_5552.y, x_5551.y, x_5551.z);
        let x_5554 : vec4<f32> = u_xlat11;
        let x_5556 : vec4<f32> = u_xlat11;
        let x_5558 : vec2<f32> = (vec2<f32>(x_5554.x, x_5554.z) / vec2<f32>(x_5556.w, x_5556.w));
        let x_5559 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5558.x, x_5559.y, x_5558.y, x_5559.w);
        let x_5561 : vec4<f32> = u_xlat11;
        let x_5564 : vec2<f32> = ((vec2<f32>(x_5561.x, x_5561.z) * vec2<f32>(0.5f, 0.5f)) + vec2<f32>(0.5f, 0.5f));
        let x_5565 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5564.x, x_5565.y, x_5564.y, x_5565.w);
        let x_5567 : vec4<f32> = u_xlat11;
        let x_5571 : vec2<f32> = clamp(vec2<f32>(x_5567.x, x_5567.z), vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
        let x_5572 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5571.x, x_5572.y, x_5571.y, x_5572.w);
        let x_5574 : i32 = u_xlati84;
        let x_5576 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5574];
        let x_5578 : vec4<f32> = u_xlat11;
        let x_5581 : i32 = u_xlati84;
        let x_5583 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5581];
        let x_5585 : vec2<f32> = ((vec2<f32>(x_5576.x, x_5576.y) * vec2<f32>(x_5578.x, x_5578.z)) + vec2<f32>(x_5583.z, x_5583.w));
        let x_5586 : vec4<f32> = u_xlat11;
        u_xlat11 = vec4<f32>(x_5585.x, x_5586.y, x_5585.y, x_5586.w);
      } else {
        let x_5589 : i32 = u_xlati88;
        u_xlatb88 = (x_5589 == 1i);
        let x_5591 : bool = u_xlatb88;
        u_xlati88 = select(0i, 1i, x_5591);
        let x_5593 : i32 = u_xlati88;
        if ((x_5593 != 0i)) {
          let x_5597 : vec3<f32> = vs_INTERP8;
          let x_5599 : i32 = u_xlati37;
          let x_5602 : i32 = u_xlati37;
          let x_5606 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5599 + 1i) / 4i)][((x_5602 + 1i) % 4i)];
          let x_5608 : vec2<f32> = (vec2<f32>(x_5597.y, x_5597.y) * vec2<f32>(x_5606.x, x_5606.y));
          let x_5609 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5608.x, x_5608.y, x_5609.z, x_5609.w);
          let x_5611 : i32 = u_xlati37;
          let x_5613 : i32 = u_xlati37;
          let x_5616 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[(x_5611 / 4i)][(x_5613 % 4i)];
          let x_5618 : vec3<f32> = vs_INTERP8;
          let x_5621 : vec4<f32> = u_xlat12;
          let x_5623 : vec2<f32> = ((vec2<f32>(x_5616.x, x_5616.y) * vec2<f32>(x_5618.x, x_5618.x)) + vec2<f32>(x_5621.x, x_5621.y));
          let x_5624 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5623.x, x_5623.y, x_5624.z, x_5624.w);
          let x_5626 : i32 = u_xlati37;
          let x_5629 : i32 = u_xlati37;
          let x_5633 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5626 + 2i) / 4i)][((x_5629 + 2i) % 4i)];
          let x_5635 : vec3<f32> = vs_INTERP8;
          let x_5638 : vec4<f32> = u_xlat12;
          let x_5640 : vec2<f32> = ((vec2<f32>(x_5633.x, x_5633.y) * vec2<f32>(x_5635.z, x_5635.z)) + vec2<f32>(x_5638.x, x_5638.y));
          let x_5641 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5640.x, x_5640.y, x_5641.z, x_5641.w);
          let x_5643 : vec4<f32> = u_xlat12;
          let x_5645 : i32 = u_xlati37;
          let x_5648 : i32 = u_xlati37;
          let x_5652 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5645 + 3i) / 4i)][((x_5648 + 3i) % 4i)];
          let x_5654 : vec2<f32> = (vec2<f32>(x_5643.x, x_5643.y) + vec2<f32>(x_5652.x, x_5652.y));
          let x_5655 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5654.x, x_5654.y, x_5655.z, x_5655.w);
          let x_5657 : vec4<f32> = u_xlat12;
          let x_5660 : vec2<f32> = ((vec2<f32>(x_5657.x, x_5657.y) * vec2<f32>(0.5f, 0.5f)) + vec2<f32>(0.5f, 0.5f));
          let x_5661 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5660.x, x_5660.y, x_5661.z, x_5661.w);
          let x_5663 : vec4<f32> = u_xlat12;
          let x_5665 : vec2<f32> = fract(vec2<f32>(x_5663.x, x_5663.y));
          let x_5666 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5665.x, x_5665.y, x_5666.z, x_5666.w);
          let x_5668 : i32 = u_xlati84;
          let x_5670 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5668];
          let x_5672 : vec4<f32> = u_xlat12;
          let x_5675 : i32 = u_xlati84;
          let x_5677 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5675];
          let x_5679 : vec2<f32> = ((vec2<f32>(x_5670.x, x_5670.y) * vec2<f32>(x_5672.x, x_5672.y)) + vec2<f32>(x_5677.z, x_5677.w));
          let x_5680 : vec4<f32> = u_xlat11;
          u_xlat11 = vec4<f32>(x_5679.x, x_5680.y, x_5679.y, x_5680.w);
        } else {
          let x_5683 : vec3<f32> = vs_INTERP8;
          let x_5685 : i32 = u_xlati37;
          let x_5688 : i32 = u_xlati37;
          let x_5692 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5685 + 1i) / 4i)][((x_5688 + 1i) % 4i)];
          u_xlat12 = (vec4<f32>(x_5683.y, x_5683.y, x_5683.y, x_5683.y) * x_5692);
          let x_5694 : i32 = u_xlati37;
          let x_5696 : i32 = u_xlati37;
          let x_5699 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[(x_5694 / 4i)][(x_5696 % 4i)];
          let x_5700 : vec3<f32> = vs_INTERP8;
          let x_5703 : vec4<f32> = u_xlat12;
          u_xlat12 = ((x_5699 * vec4<f32>(x_5700.x, x_5700.x, x_5700.x, x_5700.x)) + x_5703);
          let x_5705 : i32 = u_xlati37;
          let x_5708 : i32 = u_xlati37;
          let x_5712 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5705 + 2i) / 4i)][((x_5708 + 2i) % 4i)];
          let x_5713 : vec3<f32> = vs_INTERP8;
          let x_5716 : vec4<f32> = u_xlat12;
          u_xlat12 = ((x_5712 * vec4<f32>(x_5713.z, x_5713.z, x_5713.z, x_5713.z)) + x_5716);
          let x_5718 : vec4<f32> = u_xlat12;
          let x_5719 : i32 = u_xlati37;
          let x_5722 : i32 = u_xlati37;
          let x_5726 : vec4<f32> = x_3536.x_AdditionalLightsWorldToLights[((x_5719 + 3i) / 4i)][((x_5722 + 3i) % 4i)];
          u_xlat12 = (x_5718 + x_5726);
          let x_5728 : vec4<f32> = u_xlat12;
          let x_5730 : vec4<f32> = u_xlat12;
          let x_5732 : vec3<f32> = (vec3<f32>(x_5728.x, x_5728.y, x_5728.z) / vec3<f32>(x_5730.w, x_5730.w, x_5730.w));
          let x_5733 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5732.x, x_5732.y, x_5732.z, x_5733.w);
          let x_5735 : vec4<f32> = u_xlat12;
          let x_5737 : vec4<f32> = u_xlat12;
          u_xlat88 = dot(vec3<f32>(x_5735.x, x_5735.y, x_5735.z), vec3<f32>(x_5737.x, x_5737.y, x_5737.z));
          let x_5740 : f32 = u_xlat88;
          u_xlat88 = inverseSqrt(x_5740);
          let x_5742 : f32 = u_xlat88;
          let x_5744 : vec4<f32> = u_xlat12;
          let x_5746 : vec3<f32> = (vec3<f32>(x_5742, x_5742, x_5742) * vec3<f32>(x_5744.x, x_5744.y, x_5744.z));
          let x_5747 : vec4<f32> = u_xlat12;
          u_xlat12 = vec4<f32>(x_5746.x, x_5746.y, x_5746.z, x_5747.w);
          let x_5749 : vec4<f32> = u_xlat12;
          u_xlat88 = dot(abs(vec3<f32>(x_5749.x, x_5749.y, x_5749.z)), vec3<f32>(1.0f, 1.0f, 1.0f));
          let x_5754 : f32 = u_xlat88;
          u_xlat88 = max(x_5754, 0.00000099999999747524f);
          let x_5757 : f32 = u_xlat88;
          u_xlat88 = (1.0f / x_5757);
          let x_5759 : f32 = u_xlat88;
          let x_5761 : vec4<f32> = u_xlat12;
          let x_5763 : vec3<f32> = (vec3<f32>(x_5759, x_5759, x_5759) * vec3<f32>(x_5761.z, x_5761.x, x_5761.y));
          let x_5764 : vec4<f32> = u_xlat13;
          u_xlat13 = vec4<f32>(x_5763.x, x_5763.y, x_5763.z, x_5764.w);
          let x_5767 : f32 = u_xlat13.x;
          u_xlat13.x = -(x_5767);
          let x_5771 : f32 = u_xlat13.x;
          u_xlat13.x = clamp(x_5771, 0.0f, 1.0f);
          let x_5776 : vec4<f32> = u_xlat13;
          let x_5778 : vec4<bool> = (vec4<f32>(x_5776.y, x_5776.y, x_5776.z, x_5776.z) >= vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f));
          let x_5779 : vec2<bool> = vec2<bool>(x_5778.x, x_5778.z);
          let x_5780 : vec3<bool> = u_xlatb37;
          u_xlatb37 = vec3<bool>(x_5779.x, x_5780.y, x_5779.y);
          let x_5783 : bool = u_xlatb37.x;
          if (x_5783) {
            let x_5788 : f32 = u_xlat13.x;
            x_5784 = x_5788;
          } else {
            let x_5791 : f32 = u_xlat13.x;
            x_5784 = -(x_5791);
          }
          let x_5793 : f32 = x_5784;
          u_xlat37.x = x_5793;
          let x_5796 : bool = u_xlatb37.z;
          if (x_5796) {
            let x_5801 : f32 = u_xlat13.x;
            x_5797 = x_5801;
          } else {
            let x_5804 : f32 = u_xlat13.x;
            x_5797 = -(x_5804);
          }
          let x_5806 : f32 = x_5797;
          u_xlat37.z = x_5806;
          let x_5808 : vec4<f32> = u_xlat12;
          let x_5810 : f32 = u_xlat88;
          let x_5813 : vec3<f32> = u_xlat37;
          let x_5815 : vec2<f32> = ((vec2<f32>(x_5808.x, x_5808.y) * vec2<f32>(x_5810, x_5810)) + vec2<f32>(x_5813.x, x_5813.z));
          let x_5816 : vec3<f32> = u_xlat37;
          u_xlat37 = vec3<f32>(x_5815.x, x_5816.y, x_5815.y);
          let x_5818 : vec3<f32> = u_xlat37;
          let x_5821 : vec2<f32> = ((vec2<f32>(x_5818.x, x_5818.z) * vec2<f32>(0.5f, 0.5f)) + vec2<f32>(0.5f, 0.5f));
          let x_5822 : vec3<f32> = u_xlat37;
          u_xlat37 = vec3<f32>(x_5821.x, x_5822.y, x_5821.y);
          let x_5824 : vec3<f32> = u_xlat37;
          let x_5828 : vec2<f32> = clamp(vec2<f32>(x_5824.x, x_5824.z), vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
          let x_5829 : vec3<f32> = u_xlat37;
          u_xlat37 = vec3<f32>(x_5828.x, x_5829.y, x_5828.y);
          let x_5831 : i32 = u_xlati84;
          let x_5833 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5831];
          let x_5835 : vec3<f32> = u_xlat37;
          let x_5838 : i32 = u_xlati84;
          let x_5840 : vec4<f32> = x_3536.x_AdditionalLightsCookieAtlasUVRects[x_5838];
          let x_5842 : vec2<f32> = ((vec2<f32>(x_5833.x, x_5833.y) * vec2<f32>(x_5835.x, x_5835.z)) + vec2<f32>(x_5840.z, x_5840.w));
          let x_5843 : vec4<f32> = u_xlat11;
          u_xlat11 = vec4<f32>(x_5842.x, x_5843.y, x_5842.y, x_5843.w);
        }
      }
      let x_5850 : vec4<f32> = u_xlat11;
      let x_5852 : vec4<f32> = textureSampleLevel(x_AdditionalLightsCookieAtlasTexture, sampler_LinearClamp, vec2<f32>(x_5850.x, x_5850.z), 0.0f);
      u_xlat11 = x_5852;
      let x_5854 : bool = u_xlatb3.w;
      if (x_5854) {
        let x_5859 : f32 = u_xlat11.w;
        x_5855 = x_5859;
      } else {
        let x_5862 : f32 = u_xlat11.x;
        x_5855 = x_5862;
      }
      let x_5863 : f32 = x_5855;
      u_xlat88 = x_5863;
      let x_5865 : bool = u_xlatb3.x;
      if (x_5865) {
        let x_5869 : vec4<f32> = u_xlat11;
        x_5866 = vec3<f32>(x_5869.x, x_5869.y, x_5869.z);
      } else {
        let x_5872 : f32 = u_xlat88;
        x_5866 = vec3<f32>(x_5872, x_5872, x_5872);
      }
      let x_5874 : vec3<f32> = x_5866;
      let x_5875 : vec4<f32> = u_xlat11;
      u_xlat11 = vec4<f32>(x_5874.x, x_5874.y, x_5874.z, x_5875.w);
    } else {
      u_xlat11.x = 1.0f;
      u_xlat11.y = 1.0f;
      u_xlat11.z = 1.0f;
    }
    let x_5881 : vec4<f32> = u_xlat11;
    let x_5883 : i32 = u_xlati84;
    let x_5885 : vec4<f32> = x_4021.x_AdditionalLightsColor[x_5883];
    let x_5887 : vec3<f32> = (vec3<f32>(x_5881.x, x_5881.y, x_5881.z) * vec3<f32>(x_5885.x, x_5885.y, x_5885.z));
    let x_5888 : vec4<f32> = u_xlat11;
    u_xlat11 = vec4<f32>(x_5887.x, x_5887.y, x_5887.z, x_5888.w);
    let x_5890 : f32 = u_xlat85;
    let x_5891 : f32 = u_xlat87;
    u_xlat84 = (x_5890 * x_5891);
    let x_5893 : vec3<f32> = u_xlat26;
    let x_5894 : vec4<f32> = u_xlat10;
    u_xlat85 = dot(x_5893, vec3<f32>(x_5894.x, x_5894.y, x_5894.z));
    let x_5897 : f32 = u_xlat85;
    u_xlat85 = clamp(x_5897, 0.0f, 1.0f);
    let x_5899 : f32 = u_xlat84;
    let x_5900 : f32 = u_xlat85;
    u_xlat84 = (x_5899 * x_5900);
    let x_5902 : f32 = u_xlat84;
    let x_5904 : vec4<f32> = u_xlat11;
    let x_5906 : vec3<f32> = (vec3<f32>(x_5902, x_5902, x_5902) * vec3<f32>(x_5904.x, x_5904.y, x_5904.z));
    let x_5907 : vec4<f32> = u_xlat11;
    u_xlat11 = vec4<f32>(x_5906.x, x_5906.y, x_5906.z, x_5907.w);
    let x_5909 : vec4<f32> = u_xlat9;
    let x_5911 : f32 = u_xlat86;
    let x_5914 : vec3<f32> = u_xlat4;
    let x_5915 : vec3<f32> = ((vec3<f32>(x_5909.x, x_5909.y, x_5909.z) * vec3<f32>(x_5911, x_5911, x_5911)) + x_5914);
    let x_5916 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_5915.x, x_5915.y, x_5915.z, x_5916.w);
    let x_5918 : vec4<f32> = u_xlat9;
    let x_5920 : vec4<f32> = u_xlat9;
    u_xlat84 = dot(vec3<f32>(x_5918.x, x_5918.y, x_5918.z), vec3<f32>(x_5920.x, x_5920.y, x_5920.z));
    let x_5923 : f32 = u_xlat84;
    u_xlat84 = max(x_5923, 1.17549435e-38f);
    let x_5925 : f32 = u_xlat84;
    u_xlat84 = inverseSqrt(x_5925);
    let x_5927 : f32 = u_xlat84;
    let x_5929 : vec4<f32> = u_xlat9;
    let x_5931 : vec3<f32> = (vec3<f32>(x_5927, x_5927, x_5927) * vec3<f32>(x_5929.x, x_5929.y, x_5929.z));
    let x_5932 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_5931.x, x_5931.y, x_5931.z, x_5932.w);
    let x_5934 : vec3<f32> = u_xlat26;
    let x_5935 : vec4<f32> = u_xlat9;
    u_xlat84 = dot(x_5934, vec3<f32>(x_5935.x, x_5935.y, x_5935.z));
    let x_5938 : f32 = u_xlat84;
    u_xlat84 = clamp(x_5938, 0.0f, 1.0f);
    let x_5940 : vec4<f32> = u_xlat10;
    let x_5942 : vec4<f32> = u_xlat9;
    u_xlat85 = dot(vec3<f32>(x_5940.x, x_5940.y, x_5940.z), vec3<f32>(x_5942.x, x_5942.y, x_5942.z));
    let x_5945 : f32 = u_xlat85;
    u_xlat85 = clamp(x_5945, 0.0f, 1.0f);
    let x_5947 : f32 = u_xlat84;
    let x_5948 : f32 = u_xlat84;
    u_xlat84 = (x_5947 * x_5948);
    let x_5950 : f32 = u_xlat84;
    let x_5952 : f32 = u_xlat59.x;
    u_xlat84 = ((x_5950 * x_5952) + 1.00001001358032226562f);
    let x_5955 : f32 = u_xlat85;
    let x_5956 : f32 = u_xlat85;
    u_xlat85 = (x_5955 * x_5956);
    let x_5958 : f32 = u_xlat84;
    let x_5959 : f32 = u_xlat84;
    u_xlat84 = (x_5958 * x_5959);
    let x_5961 : f32 = u_xlat85;
    u_xlat85 = max(x_5961, 0.10000000149011611938f);
    let x_5963 : f32 = u_xlat84;
    let x_5964 : f32 = u_xlat85;
    u_xlat84 = (x_5963 * x_5964);
    let x_5967 : f32 = u_xlat7.x;
    let x_5968 : f32 = u_xlat84;
    u_xlat84 = (x_5967 * x_5968);
    let x_5970 : f32 = u_xlat83;
    let x_5971 : f32 = u_xlat84;
    u_xlat84 = (x_5970 / x_5971);
    let x_5973 : f32 = u_xlat84;
    let x_5976 : vec4<f32> = u_xlat6;
    let x_5978 : vec3<f32> = ((vec3<f32>(x_5973, x_5973, x_5973) * vec3<f32>(0.03999999910593032837f, 0.03999999910593032837f, 0.03999999910593032837f)) + vec3<f32>(x_5976.x, x_5976.y, x_5976.z));
    let x_5979 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_5978.x, x_5978.y, x_5978.z, x_5979.w);
    let x_5981 : vec4<f32> = u_xlat9;
    let x_5983 : vec4<f32> = u_xlat11;
    let x_5986 : vec4<f32> = u_xlat8;
    let x_5988 : vec3<f32> = ((vec3<f32>(x_5981.x, x_5981.y, x_5981.z) * vec3<f32>(x_5983.x, x_5983.y, x_5983.z)) + vec3<f32>(x_5986.x, x_5986.y, x_5986.z));
    let x_5989 : vec4<f32> = u_xlat8;
    u_xlat8 = vec4<f32>(x_5988.x, x_5988.y, x_5988.z, x_5989.w);

    continuing {
      let x_5991 : u32 = u_xlatu_loop_1;
      u_xlatu_loop_1 = (x_5991 + bitcast<u32>(1i));
    }
  }
  let x_5993 : vec4<f32> = u_xlat5;
  let x_5995 : f32 = u_xlat33;
  let x_5998 : vec3<f32> = u_xlat28;
  u_xlat0 = ((vec3<f32>(x_5993.x, x_5993.y, x_5993.z) * vec3<f32>(x_5995, x_5995, x_5995)) + x_5998);
  let x_6000 : vec4<f32> = u_xlat8;
  let x_6002 : vec3<f32> = u_xlat0;
  u_xlat0 = (vec3<f32>(x_6000.x, x_6000.y, x_6000.z) + x_6002);
  let x_6004 : vec4<f32> = vs_INTERP6;
  let x_6006 : vec3<f32> = u_xlat1;
  let x_6008 : vec3<f32> = u_xlat0;
  u_xlat0 = ((vec3<f32>(x_6004.w, x_6004.w, x_6004.w) * x_6006) + x_6008);
  let x_6012 : f32 = u_xlat55.x;
  let x_6014 : f32 = u_xlat55.x;
  u_xlat78 = (x_6012 * -(x_6014));
  let x_6017 : f32 = u_xlat78;
  u_xlat78 = exp2(x_6017);
  let x_6019 : vec3<f32> = u_xlat0;
  let x_6021 : vec4<f32> = x_149.unity_FogColor;
  u_xlat0 = (x_6019 + -(vec3<f32>(x_6021.x, x_6021.y, x_6021.z)));
  let x_6027 : f32 = u_xlat78;
  let x_6029 : vec3<f32> = u_xlat0;
  let x_6032 : vec4<f32> = x_149.unity_FogColor;
  let x_6034 : vec3<f32> = ((vec3<f32>(x_6027, x_6027, x_6027) * x_6029) + vec3<f32>(x_6032.x, x_6032.y, x_6032.z));
  let x_6035 : vec4<f32> = SV_Target0;
  SV_Target0 = vec4<f32>(x_6034.x, x_6034.y, x_6034.z, x_6035.w);
  let x_6037 : bool = u_xlatb29;
  let x_6038 : f32 = u_xlat79;
  SV_Target0.w = select(1.0f, x_6038, x_6037);
  return;
}

struct main_out {
  @location(0)
  SV_Target0_1 : vec4<f32>,
}

@fragment
fn main(@location(5) vs_INTERP9_param : vec3<f32>, @location(1) vs_INTERP4_param : vec4<f32>, @location(4) vs_INTERP8_param : vec3<f32>, @location(2) vs_INTERP5_param : vec4<f32>, @location(3) vs_INTERP6_param : vec4<f32>, @location(0) vs_INTERP0_param : vec2<f32>, @builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  vs_INTERP9 = vs_INTERP9_param;
  vs_INTERP4 = vs_INTERP4_param;
  vs_INTERP8 = vs_INTERP8_param;
  vs_INTERP5 = vs_INTERP5_param;
  vs_INTERP6 = vs_INTERP6_param;
  vs_INTERP0 = vs_INTERP0_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(SV_Target0);
}

