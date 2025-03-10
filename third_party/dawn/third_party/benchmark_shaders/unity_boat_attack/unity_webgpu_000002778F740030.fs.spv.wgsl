diagnostic(off, derivative_uniformity);

alias Arr = array<vec4f, 2u>;

struct UnityPerDraw {
  /* @offset(0) */
  unity_ObjectToWorld : mat4x4f,
  /* @offset(64) */
  unity_WorldToObject : mat4x4f,
  /* @offset(128) */
  unity_LODFade : vec4f,
  /* @offset(144) */
  unity_WorldTransformParams : vec4f,
  /* @offset(160) */
  unity_RenderingLayer : vec4f,
  /* @offset(176) */
  unity_LightData : vec4f,
  /* @offset(192) */
  unity_LightIndices : Arr,
  /* @offset(224) */
  unity_ProbesOcclusion : vec4f,
  /* @offset(240) */
  unity_SpecCube0_HDR : vec4f,
  /* @offset(256) */
  unity_SpecCube1_HDR : vec4f,
  /* @offset(272) */
  unity_SpecCube0_BoxMax : vec4f,
  /* @offset(288) */
  unity_SpecCube0_BoxMin : vec4f,
  /* @offset(304) */
  unity_SpecCube0_ProbePosition : vec4f,
  /* @offset(320) */
  unity_SpecCube1_BoxMax : vec4f,
  /* @offset(336) */
  unity_SpecCube1_BoxMin : vec4f,
  /* @offset(352) */
  unity_SpecCube1_ProbePosition : vec4f,
  /* @offset(368) */
  unity_LightmapST : vec4f,
  /* @offset(384) */
  unity_DynamicLightmapST : vec4f,
  /* @offset(400) */
  unity_SHAr : vec4f,
  /* @offset(416) */
  unity_SHAg : vec4f,
  /* @offset(432) */
  unity_SHAb : vec4f,
  /* @offset(448) */
  unity_SHBr : vec4f,
  /* @offset(464) */
  unity_SHBg : vec4f,
  /* @offset(480) */
  unity_SHBb : vec4f,
  /* @offset(496) */
  unity_SHC : vec4f,
  /* @offset(512) */
  unity_RendererBounds_Min : vec4f,
  /* @offset(528) */
  unity_RendererBounds_Max : vec4f,
  /* @offset(544) */
  unity_MatrixPreviousM : mat4x4f,
  /* @offset(608) */
  unity_MatrixPreviousMI : mat4x4f,
  /* @offset(672) */
  unity_MotionVectorsParams : vec4f,
  /* @offset(688) */
  unity_SpriteColor : vec4f,
  /* @offset(704) */
  unity_SpriteProps : vec4f,
}

struct PGlobals {
  /* @offset(0) */
  x_GlobalMipBias : vec2f,
  /* @offset(8) */
  x_AlphaToMaskAvailable : f32,
  /* @offset(16) */
  x_MainLightPosition : vec4f,
  /* @offset(32) */
  x_MainLightColor : vec4f,
  /* @offset(48) */
  x_AdditionalLightsCount : vec4f,
  /* @offset(64) */
  x_WorldSpaceCameraPos : vec3f,
  /* @offset(80) */
  x_ProjectionParams : vec4f,
  /* @offset(96) */
  x_ScreenParams : vec4f,
  /* @offset(112) */
  unity_OrthoParams : vec4f,
  /* @offset(128) */
  unity_FogParams : vec4f,
  /* @offset(144) */
  unity_FogColor : vec4f,
  /* @offset(160) */
  unity_MatrixV : mat4x4f,
}

struct UnityPerMaterial {
  /* @offset(0) */
  Texture2D_B222E8F_TexelSize : vec4f,
  /* @offset(16) */
  Color_C30C7CA3 : vec4f,
  /* @offset(32) */
  Texture2D_D9BFD5F1_TexelSize : vec4f,
}

alias Arr_1 = array<mat4x4f, 5u>;

alias Arr_2 = array<vec4f, 32u>;

alias Arr_3 = array<mat4x4f, 32u>;

struct LightShadows {
  /* @offset(0) */
  x_MainLightWorldToShadow : Arr_1,
  /* @offset(320) */
  x_CascadeShadowSplitSpheres0 : vec4f,
  /* @offset(336) */
  x_CascadeShadowSplitSpheres1 : vec4f,
  /* @offset(352) */
  x_CascadeShadowSplitSpheres2 : vec4f,
  /* @offset(368) */
  x_CascadeShadowSplitSpheres3 : vec4f,
  /* @offset(384) */
  x_CascadeShadowSplitSphereRadii : vec4f,
  /* @offset(400) */
  x_MainLightShadowOffset0 : vec4f,
  /* @offset(416) */
  x_MainLightShadowOffset1 : vec4f,
  /* @offset(432) */
  x_MainLightShadowParams : vec4f,
  /* @offset(448) */
  x_MainLightShadowmapSize : vec4f,
  /* @offset(464) */
  x_AdditionalShadowOffset0 : vec4f,
  /* @offset(480) */
  x_AdditionalShadowOffset1 : vec4f,
  /* @offset(496) */
  x_AdditionalShadowFadeParams : vec4f,
  /* @offset(512) */
  x_AdditionalShadowmapSize : vec4f,
  /* @offset(528) */
  x_AdditionalShadowParams : Arr_2,
  /* @offset(1040) */
  x_AdditionalLightsWorldToShadow : Arr_3,
}

alias Arr_4 = array<mat4x4f, 32u>;

alias Arr_5 = array<vec4f, 32u>;

struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr_6 = array<strided_arr, 32u>;

struct LightCookies {
  /* @offset(0) */
  x_MainLightWorldToLight : mat4x4f,
  /* @offset(64) */
  x_AdditionalLightsCookieEnableBits : f32,
  /* @offset(68) */
  x_MainLightCookieTextureFormat : f32,
  /* @offset(72) */
  x_AdditionalLightsCookieAtlasTextureFormat : f32,
  /* @offset(80) */
  x_AdditionalLightsWorldToLights : Arr_4,
  /* @offset(2128) */
  x_AdditionalLightsCookieAtlasUVRects : Arr_5,
  /* @offset(2640) */
  x_AdditionalLightsLightTypes : Arr_6,
}

alias Arr_7 = array<vec4f, 32u>;

alias Arr_8 = array<vec4f, 32u>;

alias Arr_9 = array<vec4f, 32u>;

alias Arr_10 = array<vec4f, 32u>;

alias Arr_11 = array<vec4f, 32u>;

alias Arr_12 = array<strided_arr, 32u>;

struct AdditionalLights {
  /* @offset(0) */
  x_AdditionalLightsPosition : Arr_7,
  /* @offset(512) */
  x_AdditionalLightsColor : Arr_8,
  /* @offset(1024) */
  x_AdditionalLightsAttenuation : Arr_9,
  /* @offset(1536) */
  x_AdditionalLightsSpotDir : Arr_10,
  /* @offset(2048) */
  x_AdditionalLightsOcclusionProbes : Arr_11,
  /* @offset(2560) */
  x_AdditionalLightsLayerMasks : Arr_12,
}

var<private> u_xlat0 : vec3f;

var<private> vs_INTERP9 : vec3f;

var<private> u_xlatb26 : vec2<bool>;

var<private> vs_INTERP4 : vec4f;

@group(1) @binding(2) var<uniform> x_83 : UnityPerDraw;

var<private> u_xlat26 : vec3f;

var<private> u_xlat1 : vec3f;

var<private> u_xlat2 : vec4f;

var<private> u_xlat3 : vec4f;

var<private> u_xlatb0 : bool;

@group(1) @binding(0) var<uniform> x_149 : PGlobals;

var<private> u_xlat4 : vec3f;

var<private> vs_INTERP8 : vec3f;

var<private> u_xlat79 : f32;

var<private> u_xlat5 : vec4f;

var<private> u_xlat6 : vec4f;

@group(0) @binding(7) var Texture2D_B222E8F : texture_2d<f32>;

@group(0) @binding(14) var samplerTexture2D_B222E8F : sampler;

var<private> vs_INTERP5 : vec4f;

var<private> u_xlat7 : vec3f;

@group(1) @binding(5) var<uniform> x_267 : UnityPerMaterial;

var<private> u_xlat8 : vec4f;

@group(0) @binding(8) var Texture2D_D9BFD5F1 : texture_2d<f32>;

@group(0) @binding(15) var samplerTexture2D_D9BFD5F1 : sampler;

var<private> u_xlat9 : vec4f;

var<private> u_xlat34 : vec3f;

var<private> vs_INTERP6 : vec4f;

@group(1) @binding(3) var<uniform> x_372 : LightShadows;

var<private> u_xlat10 : vec4f;

var<private> u_xlatb2 : vec4<bool>;

var<private> u_xlatu0 : u32;

var<private> u_xlati0 : i32;

var<private> u_xlatb79 : bool;

@group(0) @binding(3) var x_MainLightShadowmapTexture : texture_depth_2d;

@group(0) @binding(12) var sampler_LinearClampCompare : sampler_comparison;

var<private> u_xlatb80 : bool;

var<private> u_xlat55 : vec2f;

var<private> u_xlat62 : vec2f;

var<private> u_xlat11 : vec4f;

var<private> u_xlat12 : vec4f;

var<private> u_xlat13 : vec4f;

var<private> u_xlat14 : vec4f;

var<private> u_xlat15 : vec4f;

var<private> u_xlat16 : vec4f;

var<private> u_xlat80 : f32;

var<private> u_xlat29 : f32;

var<private> u_xlat35 : vec3f;

var<private> u_xlat17 : vec4f;

var<private> u_xlat18 : vec4f;

var<private> u_xlat36 : vec2f;

var<private> u_xlat68 : vec2f;

var<private> u_xlat63 : vec2f;

var<private> u_xlat19 : vec4f;

var<private> u_xlat20 : vec4f;

var<private> u_xlat21 : vec4f;

var<private> u_xlat82 : f32;

var<private> u_xlatb3 : vec4<bool>;

var<private> u_xlatb29 : bool;

var<private> u_xlat27 : vec3f;

var<private> u_xlatu5 : vec3u;

var<private> u_xlatu55 : u32;

var<private> u_xlatu81 : u32;

var<private> u_xlati55 : i32;

var<private> u_xlat81 : f32;

var<private> u_xlatb55 : bool;

@group(0) @binding(2) var unity_LightmapInd : texture_2d<f32>;

@group(0) @binding(10) var samplerunity_Lightmap : sampler;

var<private> vs_INTERP0 : vec2f;

@group(0) @binding(1) var unity_Lightmap : texture_2d<f32>;

var<private> u_xlat83 : f32;

var<private> u_xlat84 : f32;

var<private> u_xlat33 : f32;

var<private> u_xlatb59 : bool;

var<private> u_xlat59 : vec2f;

var<private> u_xlat60 : vec2f;

var<private> u_xlat85 : f32;

var<private> u_xlat66 : vec2f;

var<private> u_xlat87 : f32;

var<private> u_xlat28 : vec3f;

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

var<private> u_xlat37 : vec3f;

var<private> u_xlat88 : f32;

var<private> u_xlatb87 : bool;

@group(0) @binding(4) var x_AdditionalLightsShadowmapTexture : texture_depth_2d;

var<private> u_xlat64 : vec2f;

var<private> u_xlat39 : vec3f;

var<private> u_xlat22 : vec4f;

var<private> u_xlat40 : vec2f;

var<private> u_xlat72 : vec2f;

var<private> u_xlat67 : vec2f;

var<private> u_xlat23 : vec4f;

var<private> u_xlat24 : vec4f;

var<private> u_xlat25 : vec4f;

var<private> u_xlati88 : i32;

var<private> u_xlati11 : i32;

var<private> u_xlati37 : i32;

var<private> u_xlatb37 : vec3<bool>;

@group(0) @binding(6) var x_AdditionalLightsCookieAtlasTexture : texture_2d<f32>;

@group(0) @binding(11) var sampler_LinearClamp : sampler;

var<private> u_xlat78 : f32;

var<private> SV_Target0 : vec4f;

var<private> gl_FragCoord : vec4f;

var<private> u_xlatu82 : u32;

var<private> u_xlatb84 : bool;

fn int_bitfieldInsert_i1_i1_i1_i1_(base : ptr<function, i32>, insert : ptr<function, i32>, offset_1 : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  mask = (~((4294967295u << bitcast<u32>(*(bits)))) << bitcast<u32>(*(offset_1)));
  let x_26 = *(base);
  let x_28 = mask;
  let x_31 = *(insert);
  let x_33 = *(offset_1);
  let x_36 = mask;
  return bitcast<i32>(((bitcast<u32>(x_26) & ~(x_28)) | ((bitcast<u32>(x_31) << bitcast<u32>(x_33)) & x_36)));
}

const x_288 = vec4f(2.0f);

const x_652 = vec4f(0.25f);

const x_670 = vec2f(0.5f);

const x_693 = vec4f(0.5f, 1.0f, 0.5f, 1.0f);

const x_703 = vec2f(0.07999999821186065674f);

const x_718 = vec2f(1.0f);

const x_756 = vec2f(0.15999999642372131348f);

const x_826 = vec3f(-2.5f, -0.5f, 1.5f);

const x_1159 = vec2f(0.04081600159406661987f);

const x_1201 = vec2f(2.0f);

const x_1217 = vec3f(0.08163200318813323975f);

const x_1223 = vec2f(0.08163200318813323975f);

const x_1233 = vec2f(-0.08163200318813323975f, 0.08163200318813323975f);

const x_1236 = vec2f(0.16326400637626647949f, 0.08163200318813323975f);

const x_1243 = vec2f(0.08163200318813323975f, 0.16326400637626647949f);

const x_1281 = vec4f(-3.5f, -1.5f, 0.5f, 2.5f);

const x_3914 = vec3f(0.03999999910593032837f);

fn main_1() {
  var x_189 : vec3f;
  var txVec0 : vec3f;
  var txVec1 : vec3f;
  var txVec2 : vec3f;
  var txVec3 : vec3f;
  var txVec4 : vec3f;
  var txVec5 : vec3f;
  var txVec6 : vec3f;
  var txVec7 : vec3f;
  var txVec8 : vec3f;
  var txVec9 : vec3f;
  var txVec10 : vec3f;
  var txVec11 : vec3f;
  var txVec12 : vec3f;
  var txVec13 : vec3f;
  var txVec14 : vec3f;
  var txVec15 : vec3f;
  var txVec16 : vec3f;
  var txVec17 : vec3f;
  var txVec18 : vec3f;
  var txVec19 : vec3f;
  var txVec20 : vec3f;
  var txVec21 : vec3f;
  var txVec22 : vec3f;
  var txVec23 : vec3f;
  var txVec24 : vec3f;
  var txVec25 : vec3f;
  var txVec26 : vec3f;
  var txVec27 : vec3f;
  var txVec28 : vec3f;
  var txVec29 : vec3f;
  var x_1876 : f32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var x_2001 : f32;
  var x_2055 : f32;
  var txVec30 : vec3f;
  var txVec31 : vec3f;
  var txVec32 : vec3f;
  var txVec33 : vec3f;
  var txVec34 : vec3f;
  var txVec35 : vec3f;
  var txVec36 : vec3f;
  var txVec37 : vec3f;
  var txVec38 : vec3f;
  var txVec39 : vec3f;
  var txVec40 : vec3f;
  var txVec41 : vec3f;
  var txVec42 : vec3f;
  var txVec43 : vec3f;
  var txVec44 : vec3f;
  var txVec45 : vec3f;
  var txVec46 : vec3f;
  var txVec47 : vec3f;
  var txVec48 : vec3f;
  var txVec49 : vec3f;
  var txVec50 : vec3f;
  var txVec51 : vec3f;
  var txVec52 : vec3f;
  var txVec53 : vec3f;
  var txVec54 : vec3f;
  var txVec55 : vec3f;
  var txVec56 : vec3f;
  var txVec57 : vec3f;
  var txVec58 : vec3f;
  var txVec59 : vec3f;
  var x_3482 : f32;
  var x_3618 : f32;
  var x_3629 : vec3f;
  var u_xlatu_loop_1 : u32;
  var indexable : array<vec4u, 4u>;
  var x_4158 : f32;
  var x_4169 : f32;
  var txVec60 : vec3f;
  var txVec61 : vec3f;
  var txVec62 : vec3f;
  var txVec63 : vec3f;
  var txVec64 : vec3f;
  var txVec65 : vec3f;
  var txVec66 : vec3f;
  var txVec67 : vec3f;
  var txVec68 : vec3f;
  var txVec69 : vec3f;
  var txVec70 : vec3f;
  var txVec71 : vec3f;
  var txVec72 : vec3f;
  var txVec73 : vec3f;
  var txVec74 : vec3f;
  var txVec75 : vec3f;
  var txVec76 : vec3f;
  var txVec77 : vec3f;
  var txVec78 : vec3f;
  var txVec79 : vec3f;
  var txVec80 : vec3f;
  var txVec81 : vec3f;
  var txVec82 : vec3f;
  var txVec83 : vec3f;
  var txVec84 : vec3f;
  var txVec85 : vec3f;
  var txVec86 : vec3f;
  var txVec87 : vec3f;
  var txVec88 : vec3f;
  var txVec89 : vec3f;
  var x_5784 : f32;
  var x_5797 : f32;
  var x_5855 : f32;
  var x_5866 : vec3f;
  var u_xlat_precise_vec4 : vec4f;
  var u_xlat_precise_ivec4 : vec4i;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4u;
  u_xlat0.x = dot(vs_INTERP9, vs_INTERP9);
  u_xlat0.x = sqrt(u_xlat0.x);
  u_xlat0.x = (1.0f / u_xlat0.x);
  u_xlatb26.x = (0.0f < vs_INTERP4.w);
  u_xlatb26.y = (x_83.unity_WorldTransformParams.w >= 0.0f);
  u_xlat26.x = select(-1.0f, 1.0f, u_xlatb26.x);
  u_xlat26.y = select(-1.0f, 1.0f, u_xlatb26.y);
  u_xlat26.x = (u_xlat26.y * u_xlat26.x);
  u_xlat1 = (vs_INTERP4.yzx * vs_INTERP9.zxy);
  u_xlat1 = ((vs_INTERP9.yzx * vs_INTERP4.zxy) + -(u_xlat1));
  u_xlat26 = (u_xlat26.xxx * u_xlat1);
  u_xlat1 = (u_xlat0.xxx * vs_INTERP9);
  u_xlat2 = vec4f(((u_xlat0.xxx * vs_INTERP4.xyz)).xyz, u_xlat2.w);
  u_xlat3 = vec4f(((u_xlat26 * u_xlat0.xxx)).xyz, u_xlat3.w);
  u_xlatb0 = (x_149.unity_OrthoParams.w == 0.0f);
  u_xlat4 = (-(vs_INTERP8) + x_149.x_WorldSpaceCameraPos);
  u_xlat79 = dot(u_xlat4, u_xlat4);
  u_xlat79 = inverseSqrt(u_xlat79);
  u_xlat4 = (vec3f(u_xlat79) * u_xlat4);
  u_xlat5.x = x_149.unity_MatrixV[0i].z;
  u_xlat5.y = x_149.unity_MatrixV[1i].z;
  u_xlat5.z = x_149.unity_MatrixV[2i].z;
  if (u_xlatb0) {
    x_189 = u_xlat4;
  } else {
    x_189 = u_xlat5.xyz;
  }
  u_xlat4 = x_189;
  u_xlat5 = vec4f(((u_xlat4.yyy * x_83.unity_WorldToObject[1i].xyz)).xyz, u_xlat5.w);
  u_xlat5 = vec4f((((x_83.unity_WorldToObject[0i].xyz * u_xlat4.xxx) + u_xlat5.xyz)).xyz, u_xlat5.w);
  u_xlat5 = vec4f((((x_83.unity_WorldToObject[2i].xyz * u_xlat4.zzz) + u_xlat5.xyz)).xyz, u_xlat5.w);
  u_xlat0.x = dot(u_xlat5.xyz, u_xlat5.xyz);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat5 = vec4f(((u_xlat0.xxx * u_xlat5.xyz)).xyz, u_xlat5.w);
  let x_257 = vs_INTERP5;
  let x_260 = x_149.x_GlobalMipBias.x;
  let x_261 = textureSampleBias(Texture2D_B222E8F, samplerTexture2D_B222E8F, x_257.xy, x_260);
  u_xlat6 = x_261;
  u_xlat7 = (u_xlat6.xyz * x_267.Color_C30C7CA3.xyz);
  let x_278 = vs_INTERP5;
  let x_281 = x_149.x_GlobalMipBias.x;
  let x_282 = textureSampleBias(Texture2D_D9BFD5F1, samplerTexture2D_D9BFD5F1, x_278.xy, x_281);
  u_xlat8 = x_282.wxyz;
  u_xlat9 = ((u_xlat8.yzwx * x_288) + vec4f(-1.0f));
  u_xlat0.x = dot(u_xlat9, u_xlat9);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat34 = (u_xlat0.xxx * u_xlat9.xyz);
  u_xlat0.x = (vs_INTERP6.y * 200.0f);
  u_xlat0.x = min(u_xlat0.x, 1.0f);
  u_xlat6 = vec4f(((u_xlat0.xxx * u_xlat6.xyz)).xyz, u_xlat6.w);
  u_xlat3 = vec4f(((u_xlat3.xyz * u_xlat34.yyy)).xyz, u_xlat3.w);
  u_xlat2 = vec4f((((u_xlat34.xxx * u_xlat2.xyz) + u_xlat3.xyz)).xyz, u_xlat2.w);
  u_xlat1 = ((u_xlat34.zzz * u_xlat1) + u_xlat2.xyz);
  u_xlat0.x = dot(u_xlat1, u_xlat1);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat1 = (u_xlat0.xxx * u_xlat1);
  u_xlat2 = vec4f(((vs_INTERP8 + -(x_372.x_CascadeShadowSplitSpheres0.xyz))).xyz, u_xlat2.w);
  u_xlat3 = vec4f(((vs_INTERP8 + -(x_372.x_CascadeShadowSplitSpheres1.xyz))).xyz, u_xlat3.w);
  u_xlat9 = vec4f(((vs_INTERP8 + -(x_372.x_CascadeShadowSplitSpheres2.xyz))).xyz, u_xlat9.w);
  u_xlat10 = vec4f(((vs_INTERP8 + -(x_372.x_CascadeShadowSplitSpheres3.xyz))).xyz, u_xlat10.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat2.y = dot(u_xlat3.xyz, u_xlat3.xyz);
  u_xlat2.z = dot(u_xlat9.xyz, u_xlat9.xyz);
  u_xlat2.w = dot(u_xlat10.xyz, u_xlat10.xyz);
  u_xlatb2 = (u_xlat2 < x_372.x_CascadeShadowSplitSphereRadii);
  u_xlat3.x = select(0.0f, 1.0f, u_xlatb2.x);
  u_xlat3.y = select(0.0f, 1.0f, u_xlatb2.y);
  u_xlat3.z = select(0.0f, 1.0f, u_xlatb2.z);
  u_xlat3.w = select(0.0f, 1.0f, u_xlatb2.w);
  u_xlat2.x = select(-0.0f, -1.0f, u_xlatb2.x);
  u_xlat2.y = select(-0.0f, -1.0f, u_xlatb2.y);
  u_xlat2.z = select(-0.0f, -1.0f, u_xlatb2.z);
  u_xlat2 = vec4f(((u_xlat2.xyz + u_xlat3.yzw)).xyz, u_xlat2.w);
  u_xlat3 = vec4f(u_xlat3.x, max(u_xlat2.xyz, vec3f()).xyz);
  u_xlat0.x = dot(u_xlat3, vec4f(4.0f, 3.0f, 2.0f, 1.0f));
  u_xlat0.x = (-(u_xlat0.x) + 4.0f);
  u_xlatu0 = u32(u_xlat0.x);
  u_xlati0 = (bitcast<i32>(u_xlatu0) << bitcast<u32>(2i));
  u_xlat2 = vec4f(((vs_INTERP8.yyy * x_372.x_MainLightWorldToShadow[((u_xlati0 + 1i) / 4i)][((u_xlati0 + 1i) % 4i)].xyz)).xyz, u_xlat2.w);
  u_xlat2 = vec4f((((x_372.x_MainLightWorldToShadow[(u_xlati0 / 4i)][(u_xlati0 % 4i)].xyz * vs_INTERP8.xxx) + u_xlat2.xyz)).xyz, u_xlat2.w);
  u_xlat2 = vec4f((((x_372.x_MainLightWorldToShadow[((u_xlati0 + 2i) / 4i)][((u_xlati0 + 2i) % 4i)].xyz * vs_INTERP8.zzz) + u_xlat2.xyz)).xyz, u_xlat2.w);
  u_xlat2 = vec4f(((u_xlat2.xyz + x_372.x_MainLightWorldToShadow[((u_xlati0 + 3i) / 4i)][((u_xlati0 + 3i) % 4i)].xyz)).xyz, u_xlat2.w);
  u_xlatb0 = (0.0f < x_372.x_MainLightShadowParams.y);
  if (u_xlatb0) {
    u_xlatb79 = (x_372.x_MainLightShadowParams.y == 1.0f);
    if (u_xlatb79) {
      u_xlat3 = (u_xlat2.xyxy + x_372.x_MainLightShadowOffset0);
      let x_581 = u_xlat3.xy;
      txVec0 = vec3f(x_581.x, x_581.y, u_xlat2.z);
      u_xlat9.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec0.xy, txVec0.z);
      let x_601 = u_xlat3.zw;
      txVec1 = vec3f(x_601.x, x_601.y, u_xlat2.z);
      u_xlat9.y = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec1.xy, txVec1.z);
      u_xlat3 = (u_xlat2.xyxy + x_372.x_MainLightShadowOffset1);
      let x_622 = u_xlat3.xy;
      txVec2 = vec3f(x_622.x, x_622.y, u_xlat2.z);
      u_xlat9.z = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec2.xy, txVec2.z);
      let x_637 = u_xlat3.zw;
      txVec3 = vec3f(x_637.x, x_637.y, u_xlat2.z);
      u_xlat9.w = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec3.xy, txVec3.z);
      u_xlat79 = dot(u_xlat9, x_652);
    } else {
      u_xlatb80 = (x_372.x_MainLightShadowParams.y == 2.0f);
      if (u_xlatb80) {
        u_xlat3 = vec4f((((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + x_670)).xy, u_xlat3.zw);
        u_xlat3 = vec4f(floor(u_xlat3.xy).xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + x_693);
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let x_704 = (u_xlat10.yw * x_703);
        u_xlat9 = vec4f(x_704.x, u_xlat9.y, x_704.y, u_xlat9.w);
        u_xlat10 = vec4f((((u_xlat10.xz * x_670) + -(u_xlat55))).xy, u_xlat10.zw);
        u_xlat62 = (-(u_xlat55) + x_718);
        u_xlat11 = vec4f(min(u_xlat55, vec2f()).xy, u_xlat11.zw);
        u_xlat11 = vec4f((((-(u_xlat11.xy) * u_xlat11.xy) + u_xlat62)).xy, u_xlat11.zw);
        u_xlat55 = max(u_xlat55, vec2f());
        u_xlat55 = ((-(u_xlat55) * u_xlat55) + u_xlat9.yw);
        u_xlat11 = vec4f(((u_xlat11.xy + x_718)).xy, u_xlat11.zw);
        u_xlat55 = (u_xlat55 + x_718);
        u_xlat12 = vec4f(((u_xlat10.xy * x_756)).xy, u_xlat12.zw);
        u_xlat10 = vec4f(((u_xlat62 * x_756)).xy, u_xlat10.zw);
        u_xlat11 = vec4f(((u_xlat11.xy * x_756)).xy, u_xlat11.zw);
        u_xlat13 = vec4f(((u_xlat55 * x_756)).xy, u_xlat13.zw);
        u_xlat55 = (u_xlat9.yw * x_756);
        u_xlat12.z = u_xlat11.x;
        u_xlat12.w = u_xlat55.x;
        u_xlat10.z = u_xlat13.x;
        u_xlat10.w = u_xlat9.x;
        u_xlat14 = (u_xlat10.zwxz + u_xlat12.zwxz);
        u_xlat11.z = u_xlat12.y;
        u_xlat11.w = u_xlat55.y;
        u_xlat13.z = u_xlat10.y;
        u_xlat13.w = u_xlat9.z;
        u_xlat9 = vec4f(((u_xlat11.zyw + u_xlat13.zyw)).xyz, u_xlat9.w);
        u_xlat10 = vec4f(((u_xlat10.xzw / u_xlat14.zwy)).xyz, u_xlat10.w);
        u_xlat10 = vec4f(((u_xlat10.xyz + x_826)).xyz, u_xlat10.w);
        u_xlat11 = vec4f(((u_xlat13.zyw / u_xlat9.xyz)).xyz, u_xlat11.w);
        u_xlat11 = vec4f(((u_xlat11.xyz + x_826)).xyz, u_xlat11.w);
        u_xlat10 = vec4f(((u_xlat10.yxz * x_372.x_MainLightShadowmapSize.xxx)).xyz, u_xlat10.w);
        u_xlat11 = vec4f(((u_xlat11.xyz * x_372.x_MainLightShadowmapSize.yyy)).xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.ywxw);
        u_xlat55 = ((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat10.zw);
        u_xlat11.w = u_xlat10.y;
        let x_883 = u_xlat11.yz;
        u_xlat10 = vec4f(u_xlat10.x, x_883.x, u_xlat10.z, x_883.y);
        u_xlat13 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat9.xxxy * u_xlat14.zwyz);
        u_xlat16 = (u_xlat9.yyzz * u_xlat14);
        u_xlat80 = (u_xlat9.z * u_xlat14.y);
        let x_932 = u_xlat12.xy;
        txVec4 = vec3f(x_932.x, x_932.y, u_xlat2.z);
        u_xlat3.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec4.xy, txVec4.z);
        let x_947 = u_xlat12.zw;
        txVec5 = vec3f(x_947.x, x_947.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec5.xy, txVec5.z);
        u_xlat29 = (u_xlat29 * u_xlat15.y);
        u_xlat3.x = ((u_xlat15.x * u_xlat3.x) + u_xlat29);
        txVec6 = vec3f(u_xlat55.x, u_xlat55.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec6.xy, txVec6.z);
        u_xlat3.x = ((u_xlat15.z * u_xlat29) + u_xlat3.x);
        let x_995 = u_xlat11.xy;
        txVec7 = vec3f(x_995.x, x_995.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec7.xy, txVec7.z);
        u_xlat3.x = ((u_xlat15.w * u_xlat29) + u_xlat3.x);
        let x_1017 = u_xlat13.xy;
        txVec8 = vec3f(x_1017.x, x_1017.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec8.xy, txVec8.z);
        u_xlat3.x = ((u_xlat16.x * u_xlat29) + u_xlat3.x);
        let x_1039 = u_xlat13.zw;
        txVec9 = vec3f(x_1039.x, x_1039.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec9.xy, txVec9.z);
        u_xlat3.x = ((u_xlat16.y * u_xlat29) + u_xlat3.x);
        let x_1061 = u_xlat11.zw;
        txVec10 = vec3f(x_1061.x, x_1061.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec10.xy, txVec10.z);
        u_xlat3.x = ((u_xlat16.z * u_xlat29) + u_xlat3.x);
        let x_1083 = u_xlat10.xy;
        txVec11 = vec3f(x_1083.x, x_1083.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec11.xy, txVec11.z);
        u_xlat3.x = ((u_xlat16.w * u_xlat29) + u_xlat3.x);
        let x_1105 = u_xlat10.zw;
        txVec12 = vec3f(x_1105.x, x_1105.y, u_xlat2.z);
        u_xlat29 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec12.xy, txVec12.z);
        u_xlat79 = ((u_xlat80 * u_xlat29) + u_xlat3.x);
      } else {
        u_xlat3 = vec4f((((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + x_670)).xy, u_xlat3.zw);
        u_xlat3 = vec4f(floor(u_xlat3.xy).xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + x_693);
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let x_1160 = (u_xlat10.yw * x_1159);
        u_xlat11 = vec4f(u_xlat11.x, x_1160.x, u_xlat11.z, x_1160.y);
        let x_1168 = ((u_xlat10.xz * x_670) + -(u_xlat55));
        u_xlat9 = vec4f(x_1168.x, u_xlat9.y, x_1168.y, u_xlat9.w);
        u_xlat10 = vec4f(((-(u_xlat55) + x_718)).xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat55, vec2f());
        u_xlat10 = vec4f((((-(u_xlat62) * u_xlat62) + u_xlat10.xy)).xy, u_xlat10.zw);
        u_xlat62 = max(u_xlat55, vec2f());
        let x_1196 = ((-(u_xlat62) * u_xlat62) + u_xlat9.yw);
        u_xlat35 = vec3f(x_1196.x, u_xlat35.y, x_1196.y);
        u_xlat10 = vec4f(((u_xlat10.xy + x_1201)).xy, u_xlat10.zw);
        let x_1207 = (u_xlat35.xz + x_1201);
        u_xlat9 = vec4f(u_xlat9.x, x_1207.x, u_xlat9.z, x_1207.y);
        u_xlat12.z = (u_xlat9.y * 0.08163200318813323975f);
        u_xlat13 = vec4f(((u_xlat9.zxw * x_1217)).xyz, u_xlat13.w);
        u_xlat9 = vec4f(((u_xlat10.xy * x_1223)).xy, u_xlat9.zw);
        u_xlat12.x = u_xlat13.y;
        let x_1237 = ((u_xlat55.xx * x_1233) + x_1236);
        u_xlat12 = vec4f(u_xlat12.x, x_1237.x, u_xlat12.z, x_1237.y);
        let x_1244 = ((u_xlat55.xx * x_1233) + x_1243);
        u_xlat10 = vec4f(x_1244.x, u_xlat10.y, x_1244.y, u_xlat10.w);
        u_xlat10.y = u_xlat9.x;
        u_xlat10.w = u_xlat11.y;
        u_xlat12 = (u_xlat10 + u_xlat12);
        let x_1259 = ((u_xlat55.yy * x_1233) + x_1236);
        u_xlat13 = vec4f(u_xlat13.x, x_1259.x, u_xlat13.z, x_1259.y);
        let x_1265 = ((u_xlat55.yy * x_1233) + x_1243);
        u_xlat11 = vec4f(x_1265.x, u_xlat11.y, x_1265.y, u_xlat11.w);
        u_xlat11.y = u_xlat9.y;
        u_xlat9 = (u_xlat11 + u_xlat13);
        u_xlat10 = (u_xlat10 / u_xlat12);
        u_xlat10 = (u_xlat10 + x_1281);
        u_xlat11 = (u_xlat11 / u_xlat9);
        u_xlat11 = (u_xlat11 + x_1281);
        u_xlat10 = (u_xlat10.wxyz * x_372.x_MainLightShadowmapSize.xxxx);
        u_xlat11 = (u_xlat11.xwyz * x_372.x_MainLightShadowmapSize.yyyy);
        let x_1301 = u_xlat10.yzw;
        u_xlat13 = vec4f(x_1301.x, u_xlat13.y, x_1301.yz);
        u_xlat13.y = u_xlat11.x;
        u_xlat14 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        u_xlat55 = ((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat13.wy);
        u_xlat10.y = u_xlat13.y;
        u_xlat13.y = u_xlat11.z;
        u_xlat15 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        u_xlat16 = vec4f((((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat13.wy)).xy, u_xlat16.zw);
        u_xlat10.z = u_xlat13.y;
        u_xlat17 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.xyxz);
        u_xlat13.y = u_xlat11.w;
        u_xlat18 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        u_xlat36 = ((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat13.wy);
        u_xlat10.w = u_xlat13.y;
        u_xlat68 = ((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat10.xw);
        let x_1401 = u_xlat13.xzw;
        u_xlat11 = vec4f(x_1401.x, u_xlat11.y, x_1401.yz);
        u_xlat13 = ((u_xlat3.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat10.x;
        u_xlat3 = vec4f((((u_xlat3.xy * x_372.x_MainLightShadowmapSize.xy) + u_xlat11.xy)).xy, u_xlat3.zw);
        u_xlat19 = (u_xlat9.xxxx * u_xlat12);
        u_xlat20 = (u_xlat9.yyyy * u_xlat12);
        u_xlat21 = (u_xlat9.zzzz * u_xlat12);
        u_xlat9 = (u_xlat9.wwww * u_xlat12);
        let x_1458 = u_xlat14.xy;
        txVec13 = vec3f(x_1458.x, x_1458.y, u_xlat2.z);
        u_xlat80 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec13.xy, txVec13.z);
        let x_1472 = u_xlat14.zw;
        txVec14 = vec3f(x_1472.x, x_1472.y, u_xlat2.z);
        u_xlat82 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec14.xy, txVec14.z);
        u_xlat82 = (u_xlat82 * u_xlat19.y);
        u_xlat80 = ((u_xlat19.x * u_xlat80) + u_xlat82);
        txVec15 = vec3f(u_xlat55.x, u_xlat55.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec15.xy, txVec15.z);
        u_xlat80 = ((u_xlat19.z * u_xlat55.x) + u_xlat80);
        let x_1518 = u_xlat17.xy;
        txVec16 = vec3f(x_1518.x, x_1518.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec16.xy, txVec16.z);
        u_xlat80 = ((u_xlat19.w * u_xlat55.x) + u_xlat80);
        let x_1540 = u_xlat15.xy;
        txVec17 = vec3f(x_1540.x, x_1540.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec17.xy, txVec17.z);
        u_xlat80 = ((u_xlat20.x * u_xlat55.x) + u_xlat80);
        let x_1562 = u_xlat15.zw;
        txVec18 = vec3f(x_1562.x, x_1562.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec18.xy, txVec18.z);
        u_xlat80 = ((u_xlat20.y * u_xlat55.x) + u_xlat80);
        let x_1584 = u_xlat16.xy;
        txVec19 = vec3f(x_1584.x, x_1584.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec19.xy, txVec19.z);
        u_xlat80 = ((u_xlat20.z * u_xlat55.x) + u_xlat80);
        let x_1606 = u_xlat17.zw;
        txVec20 = vec3f(x_1606.x, x_1606.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec20.xy, txVec20.z);
        u_xlat80 = ((u_xlat20.w * u_xlat55.x) + u_xlat80);
        let x_1628 = u_xlat18.xy;
        txVec21 = vec3f(x_1628.x, x_1628.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec21.xy, txVec21.z);
        u_xlat80 = ((u_xlat21.x * u_xlat55.x) + u_xlat80);
        let x_1650 = u_xlat18.zw;
        txVec22 = vec3f(x_1650.x, x_1650.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec22.xy, txVec22.z);
        u_xlat80 = ((u_xlat21.y * u_xlat55.x) + u_xlat80);
        txVec23 = vec3f(u_xlat36.x, u_xlat36.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec23.xy, txVec23.z);
        u_xlat80 = ((u_xlat21.z * u_xlat55.x) + u_xlat80);
        txVec24 = vec3f(u_xlat68.x, u_xlat68.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec24.xy, txVec24.z);
        u_xlat80 = ((u_xlat21.w * u_xlat55.x) + u_xlat80);
        let x_1714 = u_xlat13.xy;
        txVec25 = vec3f(x_1714.x, x_1714.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec25.xy, txVec25.z);
        u_xlat80 = ((u_xlat9.x * u_xlat55.x) + u_xlat80);
        let x_1736 = u_xlat13.zw;
        txVec26 = vec3f(x_1736.x, x_1736.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec26.xy, txVec26.z);
        u_xlat80 = ((u_xlat9.y * u_xlat55.x) + u_xlat80);
        txVec27 = vec3f(u_xlat63.x, u_xlat63.y, u_xlat2.z);
        u_xlat55.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec27.xy, txVec27.z);
        u_xlat80 = ((u_xlat9.z * u_xlat55.x) + u_xlat80);
        let x_1779 = u_xlat3.xy;
        txVec28 = vec3f(x_1779.x, x_1779.y, u_xlat2.z);
        u_xlat3.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec28.xy, txVec28.z);
        u_xlat79 = ((u_xlat9.w * u_xlat3.x) + u_xlat80);
      }
    }
  } else {
    let x_1802 = u_xlat2.xy;
    txVec29 = vec3f(x_1802.x, x_1802.y, u_xlat2.z);
    u_xlat79 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec29.xy, txVec29.z);
  }
  u_xlat80 = (-(x_372.x_MainLightShadowParams.x) + 1.0f);
  u_xlat79 = ((u_xlat79 * x_372.x_MainLightShadowParams.x) + u_xlat80);
  u_xlatb3.x = (0.0f >= u_xlat2.z);
  u_xlatb29 = (u_xlat2.z >= 1.0f);
  u_xlatb3.x = (u_xlatb29 | u_xlatb3.x);
  u_xlat79 = select(u_xlat79, 1.0f, u_xlatb3.x);
  u_xlat1.x = dot(u_xlat1, -(x_149.x_MainLightPosition.xyz));
  u_xlat1.x = clamp(u_xlat1.x, 0.0f, 1.0f);
  u_xlat27 = (vec3f(u_xlat79) * x_149.x_MainLightColor.xyz);
  u_xlat1 = (u_xlat27 * u_xlat1.xxx);
  u_xlat1 = (u_xlat1 * u_xlat6.xyz);
  u_xlatb79 = (x_83.unity_LODFade.x < 0.0f);
  u_xlat29 = (x_83.unity_LODFade.x + 1.0f);
  if (u_xlatb79) {
    x_1876 = u_xlat29;
  } else {
    x_1876 = x_83.unity_LODFade.x;
  }
  u_xlat79 = x_1876;
  u_xlatb29 = (0.5f >= u_xlat79);
  u_xlat5 = vec4f(((abs(u_xlat5.xyz) * x_149.x_ScreenParams.xyx)).xyz, u_xlat5.w);
  u_xlatu5 = vec3u(u_xlat5.xyz);
  u_xlatu55 = (u_xlatu5.z * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  u_xlati55 = bitcast<i32>((bitcast<u32>(u_xlati55) ^ u_xlatu5.y));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  u_xlati55 = bitcast<i32>((bitcast<u32>(u_xlati55) ^ u_xlatu5.x));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  param = 1065353216i;
  param_1 = u_xlati55;
  param_2 = 0i;
  param_3 = 23i;
  let x_1988 = int_bitfieldInsert_i1_i1_i1_i1_(&(param), &(param_1), &(param_2), &(param_3));
  u_xlat55.x = bitcast<f32>(x_1988);
  u_xlat55.x = (u_xlat55.x + -1.0f);
  u_xlat81 = (-(u_xlat55.x) + 1.0f);
  if (u_xlatb29) {
    x_2001 = u_xlat55.x;
  } else {
    x_2001 = u_xlat81;
  }
  u_xlat29 = x_2001;
  u_xlat79 = ((u_xlat79 * 2.0f) + -(u_xlat29));
  u_xlat29 = (u_xlat79 * u_xlat6.w);
  u_xlatb55 = (u_xlat29 >= 0.40000000596046447754f);
  u_xlat55.x = select(0.0f, u_xlat29, u_xlatb55);
  u_xlat79 = ((u_xlat6.w * u_xlat79) + -0.40000000596046447754f);
  let x_2032 = u_xlat29;
  u_xlat81 = dpdxCoarse(x_2032);
  let x_2034 = u_xlat29;
  u_xlat29 = dpdyCoarse(x_2034);
  u_xlat29 = (abs(u_xlat29) + abs(u_xlat81));
  u_xlat29 = max(u_xlat29, 0.00009999999747378752f);
  u_xlat79 = (u_xlat79 / u_xlat29);
  u_xlat79 = (u_xlat79 + 0.5f);
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  u_xlatb29 = !((x_149.x_AlphaToMaskAvailable == 0.0f));
  if (u_xlatb29) {
    x_2055 = u_xlat79;
  } else {
    x_2055 = u_xlat55.x;
  }
  u_xlat79 = x_2055;
  u_xlat55.x = (u_xlat79 + -0.00009999999747378752f);
  u_xlatb55 = (u_xlat55.x < 0.0f);
  if (((select(0i, 1i, u_xlatb55) * -1i) != 0i)) {
    discard;
  }
  u_xlat26 = (u_xlat26 * u_xlat34.yyy);
  u_xlat26 = ((u_xlat34.xxx * vs_INTERP4.xyz) + u_xlat26);
  u_xlat26 = ((u_xlat34.zzz * vs_INTERP9) + u_xlat26);
  u_xlat55.x = dot(u_xlat26, u_xlat26);
  u_xlat55.x = inverseSqrt(u_xlat55.x);
  u_xlat26 = (u_xlat26 * u_xlat55.xxx);
  u_xlat55.x = (vs_INTERP8.y * x_149.unity_MatrixV[1i].z);
  u_xlat55.x = ((x_149.unity_MatrixV[0i].z * vs_INTERP8.x) + u_xlat55.x);
  u_xlat55.x = ((x_149.unity_MatrixV[2i].z * vs_INTERP8.z) + u_xlat55.x);
  u_xlat55.x = (u_xlat55.x + x_149.unity_MatrixV[3i].z);
  u_xlat55.x = (-(u_xlat55.x) + -(x_149.x_ProjectionParams.y));
  u_xlat55.x = max(u_xlat55.x, 0.0f);
  u_xlat55.x = (u_xlat55.x * x_149.unity_FogParams.x);
  let x_2162 = vs_INTERP0;
  let x_2164 = x_149.x_GlobalMipBias.x;
  let x_2165 = textureSampleBias(unity_LightmapInd, samplerunity_Lightmap, x_2162, x_2164);
  u_xlat5 = x_2165;
  let x_2170 = vs_INTERP0;
  let x_2172 = x_149.x_GlobalMipBias.x;
  let x_2173 = textureSampleBias(unity_Lightmap, samplerunity_Lightmap, x_2170, x_2172);
  u_xlat6 = vec4f(x_2173.xyz.xyz, u_xlat6.w);
  u_xlat5 = vec4f(((u_xlat5.xyz + vec3f(-0.5f))).xyz, u_xlat5.w);
  u_xlat81 = dot(u_xlat26, u_xlat5.xyz);
  u_xlat81 = (u_xlat81 + 0.5f);
  u_xlat5 = vec4f(((vec3f(u_xlat81) * u_xlat6.xyz)).xyz, u_xlat5.w);
  u_xlat81 = max(u_xlat5.w, 0.00009999999747378752f);
  u_xlat5 = vec4f(((u_xlat5.xyz / vec3f(u_xlat81))).xyz, u_xlat5.w);
  u_xlat8.x = u_xlat8.x;
  u_xlat8.x = clamp(u_xlat8.x, 0.0f, 1.0f);
  u_xlat79 = u_xlat79;
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  u_xlat6 = vec4f(((u_xlat7 * vec3f(0.95999997854232788086f))).xyz, u_xlat6.w);
  u_xlat81 = (-(u_xlat8.x) + 1.0f);
  u_xlat82 = (u_xlat81 * u_xlat81);
  u_xlat82 = max(u_xlat82, 0.0078125f);
  u_xlat83 = (u_xlat82 * u_xlat82);
  u_xlat84 = (u_xlat8.x + 0.04000002145767211914f);
  u_xlat84 = min(u_xlat84, 1.0f);
  u_xlat7.x = ((u_xlat82 * 4.0f) + 2.0f);
  u_xlat33 = min(vs_INTERP6.w, 1.0f);
  if (u_xlatb0) {
    u_xlatb0 = (x_372.x_MainLightShadowParams.y == 1.0f);
    if (u_xlatb0) {
      u_xlat8 = (u_xlat2.xyxy + x_372.x_MainLightShadowOffset0);
      let x_2267 = u_xlat8.xy;
      txVec30 = vec3f(x_2267.x, x_2267.y, u_xlat2.z);
      u_xlat9.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec30.xy, txVec30.z);
      let x_2282 = u_xlat8.zw;
      txVec31 = vec3f(x_2282.x, x_2282.y, u_xlat2.z);
      u_xlat9.y = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec31.xy, txVec31.z);
      u_xlat8 = (u_xlat2.xyxy + x_372.x_MainLightShadowOffset1);
      let x_2302 = u_xlat8.xy;
      txVec32 = vec3f(x_2302.x, x_2302.y, u_xlat2.z);
      u_xlat9.z = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec32.xy, txVec32.z);
      let x_2317 = u_xlat8.zw;
      txVec33 = vec3f(x_2317.x, x_2317.y, u_xlat2.z);
      u_xlat9.w = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec33.xy, txVec33.z);
      u_xlat0.x = dot(u_xlat9, x_652);
    } else {
      u_xlatb59 = (x_372.x_MainLightShadowParams.y == 2.0f);
      if (u_xlatb59) {
        u_xlat59 = ((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + x_670);
        u_xlat59 = floor(u_xlat59);
        u_xlat8 = vec4f((((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + -(u_xlat59))).xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + x_693);
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        u_xlat60 = (u_xlat10.yw * x_703);
        let x_2380 = ((u_xlat10.xz * x_670) + -(u_xlat8.xy));
        u_xlat9 = vec4f(x_2380.x, u_xlat9.y, x_2380.y, u_xlat9.w);
        u_xlat10 = vec4f(((-(u_xlat8.xy) + x_718)).xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat8.xy, vec2f());
        u_xlat62 = ((-(u_xlat62) * u_xlat62) + u_xlat10.xy);
        u_xlat8 = vec4f(max(u_xlat8.xy, vec2f()).xy, u_xlat8.zw);
        u_xlat8 = vec4f((((-(u_xlat8.xy) * u_xlat8.xy) + u_xlat9.yw)).xy, u_xlat8.zw);
        u_xlat62 = (u_xlat62 + x_718);
        u_xlat8 = vec4f(((u_xlat8.xy + x_718)).xy, u_xlat8.zw);
        u_xlat11 = vec4f(((u_xlat9.xz * x_756)).xy, u_xlat11.zw);
        u_xlat12 = vec4f(((u_xlat10.xy * x_756)).xy, u_xlat12.zw);
        u_xlat10 = vec4f(((u_xlat62 * x_756)).xy, u_xlat10.zw);
        u_xlat13 = vec4f(((u_xlat8.xy * x_756)).xy, u_xlat13.zw);
        u_xlat8 = vec4f(((u_xlat9.yw * x_756)).xy, u_xlat8.zw);
        u_xlat11.z = u_xlat10.x;
        u_xlat11.w = u_xlat8.x;
        u_xlat12.z = u_xlat13.x;
        u_xlat12.w = u_xlat60.x;
        u_xlat9 = (u_xlat11.zwxz + u_xlat12.zwxz);
        u_xlat10.z = u_xlat11.y;
        u_xlat10.w = u_xlat8.y;
        u_xlat13.z = u_xlat12.y;
        u_xlat13.w = u_xlat60.y;
        u_xlat8 = vec4f(((u_xlat10.zyw + u_xlat13.zyw)).xyz, u_xlat8.w);
        u_xlat10 = vec4f(((u_xlat12.xzw / u_xlat9.zwy)).xyz, u_xlat10.w);
        u_xlat10 = vec4f(((u_xlat10.xyz + x_826)).xyz, u_xlat10.w);
        u_xlat11 = vec4f(((u_xlat13.zyw / u_xlat8.xyz)).xyz, u_xlat11.w);
        u_xlat11 = vec4f(((u_xlat11.xyz + x_826)).xyz, u_xlat11.w);
        u_xlat10 = vec4f(((u_xlat10.yxz * x_372.x_MainLightShadowmapSize.xxx)).xyz, u_xlat10.w);
        u_xlat11 = vec4f(((u_xlat11.xyz * x_372.x_MainLightShadowmapSize.yyy)).xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.ywxw);
        u_xlat13 = vec4f((((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat10.zw)).xy, u_xlat13.zw);
        u_xlat11.w = u_xlat10.y;
        let x_2548 = u_xlat11.yz;
        u_xlat10 = vec4f(u_xlat10.x, x_2548.x, u_xlat10.z, x_2548.y);
        u_xlat14 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat8.xxxy * u_xlat9.zwyz);
        u_xlat16 = (u_xlat8.yyzz * u_xlat9);
        u_xlat59.x = (u_xlat8.z * u_xlat9.y);
        let x_2595 = u_xlat12.xy;
        txVec34 = vec3f(x_2595.x, x_2595.y, u_xlat2.z);
        u_xlat85 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec34.xy, txVec34.z);
        let x_2610 = u_xlat12.zw;
        txVec35 = vec3f(x_2610.x, x_2610.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec35.xy, txVec35.z);
        u_xlat8.x = (u_xlat8.x * u_xlat15.y);
        u_xlat85 = ((u_xlat15.x * u_xlat85) + u_xlat8.x);
        let x_2638 = u_xlat13.xy;
        txVec36 = vec3f(x_2638.x, x_2638.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec36.xy, txVec36.z);
        u_xlat85 = ((u_xlat15.z * u_xlat8.x) + u_xlat85);
        let x_2660 = u_xlat11.xy;
        txVec37 = vec3f(x_2660.x, x_2660.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec37.xy, txVec37.z);
        u_xlat85 = ((u_xlat15.w * u_xlat8.x) + u_xlat85);
        let x_2682 = u_xlat14.xy;
        txVec38 = vec3f(x_2682.x, x_2682.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec38.xy, txVec38.z);
        u_xlat85 = ((u_xlat16.x * u_xlat8.x) + u_xlat85);
        let x_2704 = u_xlat14.zw;
        txVec39 = vec3f(x_2704.x, x_2704.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec39.xy, txVec39.z);
        u_xlat85 = ((u_xlat16.y * u_xlat8.x) + u_xlat85);
        let x_2726 = u_xlat11.zw;
        txVec40 = vec3f(x_2726.x, x_2726.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec40.xy, txVec40.z);
        u_xlat85 = ((u_xlat16.z * u_xlat8.x) + u_xlat85);
        let x_2748 = u_xlat10.xy;
        txVec41 = vec3f(x_2748.x, x_2748.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec41.xy, txVec41.z);
        u_xlat85 = ((u_xlat16.w * u_xlat8.x) + u_xlat85);
        let x_2770 = u_xlat10.zw;
        txVec42 = vec3f(x_2770.x, x_2770.y, u_xlat2.z);
        u_xlat8.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec42.xy, txVec42.z);
        u_xlat0.x = ((u_xlat59.x * u_xlat8.x) + u_xlat85);
      } else {
        u_xlat59 = ((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + x_670);
        u_xlat59 = floor(u_xlat59);
        u_xlat8 = vec4f((((u_xlat2.xy * x_372.x_MainLightShadowmapSize.zw) + -(u_xlat59))).xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + x_693);
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let x_2822 = (u_xlat10.yw * x_1159);
        u_xlat11 = vec4f(u_xlat11.x, x_2822.x, u_xlat11.z, x_2822.y);
        u_xlat60 = ((u_xlat10.xz * x_670) + -(u_xlat8.xy));
        let x_2835 = (-(u_xlat8.xy) + x_718);
        u_xlat9 = vec4f(x_2835.x, u_xlat9.y, x_2835.y, u_xlat9.w);
        u_xlat10 = vec4f(min(u_xlat8.xy, vec2f()).xy, u_xlat10.zw);
        let x_2851 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.xz);
        u_xlat9 = vec4f(x_2851.x, u_xlat9.y, x_2851.y, u_xlat9.w);
        u_xlat10 = vec4f(max(u_xlat8.xy, vec2f()).xy, u_xlat10.zw);
        let x_2867 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.yw);
        u_xlat9 = vec4f(u_xlat9.x, x_2867.x, u_xlat9.z, x_2867.y);
        u_xlat9 = (u_xlat9 + x_288);
        u_xlat10.z = (u_xlat9.y * 0.08163200318813323975f);
        u_xlat12 = vec4f(((u_xlat60.yx * x_1223)).xy, u_xlat12.zw);
        u_xlat60 = (u_xlat9.xz * x_1223);
        u_xlat12.z = (u_xlat9.w * 0.08163200318813323975f);
        u_xlat10.x = u_xlat12.y;
        let x_2894 = ((u_xlat8.xx * x_1233) + x_1236);
        u_xlat10 = vec4f(u_xlat10.x, x_2894.x, u_xlat10.z, x_2894.y);
        let x_2900 = ((u_xlat8.xx * x_1233) + x_1243);
        u_xlat9 = vec4f(x_2900.x, u_xlat9.y, x_2900.y, u_xlat9.w);
        u_xlat9.y = u_xlat60.x;
        u_xlat9.w = u_xlat11.y;
        u_xlat10 = (u_xlat9 + u_xlat10);
        let x_2915 = ((u_xlat8.yy * x_1233) + x_1236);
        u_xlat12 = vec4f(u_xlat12.x, x_2915.x, u_xlat12.z, x_2915.y);
        let x_2921 = ((u_xlat8.yy * x_1233) + x_1243);
        u_xlat11 = vec4f(x_2921.x, u_xlat11.y, x_2921.y, u_xlat11.w);
        u_xlat11.y = u_xlat60.y;
        u_xlat8 = (u_xlat11 + u_xlat12);
        u_xlat9 = (u_xlat9 / u_xlat10);
        u_xlat9 = (u_xlat9 + x_1281);
        u_xlat11 = (u_xlat11 / u_xlat8);
        u_xlat11 = (u_xlat11 + x_1281);
        u_xlat9 = (u_xlat9.wxyz * x_372.x_MainLightShadowmapSize.xxxx);
        u_xlat11 = (u_xlat11.xwyz * x_372.x_MainLightShadowmapSize.yyyy);
        let x_2953 = u_xlat9.yzw;
        u_xlat12 = vec4f(x_2953.x, u_xlat12.y, x_2953.yz);
        u_xlat12.y = u_xlat11.x;
        u_xlat13 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        u_xlat14 = vec4f((((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat12.wy)).xy, u_xlat14.zw);
        u_xlat9.y = u_xlat12.y;
        u_xlat12.y = u_xlat11.z;
        u_xlat15 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        u_xlat66 = ((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat12.wy);
        u_xlat9.z = u_xlat12.y;
        u_xlat16 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat9.xyxz);
        u_xlat12.y = u_xlat11.w;
        u_xlat17 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        u_xlat35 = vec3f((((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat12.wy)).xy, u_xlat35.z);
        u_xlat9.w = u_xlat12.y;
        u_xlat18 = vec4f((((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat9.xw)).xy, u_xlat18.zw);
        let x_3050 = u_xlat12.xzw;
        u_xlat11 = vec4f(x_3050.x, u_xlat11.y, x_3050.yz);
        u_xlat12 = ((u_xlat59.xyxy * x_372.x_MainLightShadowmapSize.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat9.x;
        u_xlat59 = ((u_xlat59 * x_372.x_MainLightShadowmapSize.xy) + u_xlat11.xy);
        u_xlat19 = (u_xlat8.xxxx * u_xlat10);
        u_xlat20 = (u_xlat8.yyyy * u_xlat10);
        u_xlat21 = (u_xlat8.zzzz * u_xlat10);
        u_xlat8 = (u_xlat8.wwww * u_xlat10);
        let x_3099 = u_xlat13.xy;
        txVec43 = vec3f(x_3099.x, x_3099.y, u_xlat2.z);
        u_xlat9.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec43.xy, txVec43.z);
        let x_3114 = u_xlat13.zw;
        txVec44 = vec3f(x_3114.x, x_3114.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec44.xy, txVec44.z);
        u_xlat87 = (u_xlat87 * u_xlat19.y);
        u_xlat9.x = ((u_xlat19.x * u_xlat9.x) + u_xlat87);
        let x_3141 = u_xlat14.xy;
        txVec45 = vec3f(x_3141.x, x_3141.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec45.xy, txVec45.z);
        u_xlat9.x = ((u_xlat19.z * u_xlat87) + u_xlat9.x);
        let x_3163 = u_xlat16.xy;
        txVec46 = vec3f(x_3163.x, x_3163.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec46.xy, txVec46.z);
        u_xlat9.x = ((u_xlat19.w * u_xlat87) + u_xlat9.x);
        let x_3185 = u_xlat15.xy;
        txVec47 = vec3f(x_3185.x, x_3185.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec47.xy, txVec47.z);
        u_xlat9.x = ((u_xlat20.x * u_xlat87) + u_xlat9.x);
        let x_3207 = u_xlat15.zw;
        txVec48 = vec3f(x_3207.x, x_3207.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec48.xy, txVec48.z);
        u_xlat9.x = ((u_xlat20.y * u_xlat87) + u_xlat9.x);
        txVec49 = vec3f(u_xlat66.x, u_xlat66.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec49.xy, txVec49.z);
        u_xlat9.x = ((u_xlat20.z * u_xlat87) + u_xlat9.x);
        let x_3250 = u_xlat16.zw;
        txVec50 = vec3f(x_3250.x, x_3250.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec50.xy, txVec50.z);
        u_xlat9.x = ((u_xlat20.w * u_xlat87) + u_xlat9.x);
        let x_3272 = u_xlat17.xy;
        txVec51 = vec3f(x_3272.x, x_3272.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec51.xy, txVec51.z);
        u_xlat9.x = ((u_xlat21.x * u_xlat87) + u_xlat9.x);
        let x_3294 = u_xlat17.zw;
        txVec52 = vec3f(x_3294.x, x_3294.y, u_xlat2.z);
        u_xlat87 = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec52.xy, txVec52.z);
        u_xlat9.x = ((u_xlat21.y * u_xlat87) + u_xlat9.x);
        let x_3316 = u_xlat35.xy;
        txVec53 = vec3f(x_3316.x, x_3316.y, u_xlat2.z);
        u_xlat35.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec53.xy, txVec53.z);
        u_xlat9.x = ((u_xlat21.z * u_xlat35.x) + u_xlat9.x);
        let x_3340 = u_xlat18.xy;
        txVec54 = vec3f(x_3340.x, x_3340.y, u_xlat2.z);
        u_xlat35.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec54.xy, txVec54.z);
        u_xlat9.x = ((u_xlat21.w * u_xlat35.x) + u_xlat9.x);
        let x_3364 = u_xlat12.xy;
        txVec55 = vec3f(x_3364.x, x_3364.y, u_xlat2.z);
        u_xlat35.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec55.xy, txVec55.z);
        u_xlat8.x = ((u_xlat8.x * u_xlat35.x) + u_xlat9.x);
        let x_3388 = u_xlat12.zw;
        txVec56 = vec3f(x_3388.x, x_3388.y, u_xlat2.z);
        u_xlat9.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec56.xy, txVec56.z);
        u_xlat8.x = ((u_xlat8.y * u_xlat9.x) + u_xlat8.x);
        txVec57 = vec3f(u_xlat63.x, u_xlat63.y, u_xlat2.z);
        u_xlat34.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec57.xy, txVec57.z);
        u_xlat8.x = ((u_xlat8.z * u_xlat34.x) + u_xlat8.x);
        txVec58 = vec3f(u_xlat59.x, u_xlat59.y, u_xlat2.z);
        u_xlat59.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec58.xy, txVec58.z);
        u_xlat0.x = ((u_xlat8.w * u_xlat59.x) + u_xlat8.x);
      }
    }
  } else {
    let x_3459 = u_xlat2.xy;
    txVec59 = vec3f(x_3459.x, x_3459.y, u_xlat2.z);
    u_xlat0.x = textureSampleCompareLevel(x_MainLightShadowmapTexture, sampler_LinearClampCompare, txVec59.xy, txVec59.z);
  }
  u_xlat0.x = ((u_xlat0.x * x_372.x_MainLightShadowParams.x) + u_xlat80);
  if (u_xlatb3.x) {
    x_3482 = 1.0f;
  } else {
    x_3482 = u_xlat0.x;
  }
  u_xlat0.x = x_3482;
  u_xlat2 = vec4f(((vs_INTERP8 + -(x_149.x_WorldSpaceCameraPos))).xyz, u_xlat2.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat28.x = ((u_xlat2.x * x_372.x_MainLightShadowParams.z) + x_372.x_MainLightShadowParams.w);
  u_xlat28.x = clamp(u_xlat28.x, 0.0f, 1.0f);
  u_xlat54 = (-(u_xlat0.x) + 1.0f);
  u_xlat0.x = ((u_xlat28.x * u_xlat54) + u_xlat0.x);
  u_xlatb28.x = !((x_3536.x_MainLightCookieTextureFormat == -1.0f));
  if (u_xlatb28.x) {
    u_xlat28 = vec3f(((vs_INTERP8.yy * x_3536.x_MainLightWorldToLight[1i].xy)).xy, u_xlat28.z);
    u_xlat28 = vec3f((((x_3536.x_MainLightWorldToLight[0i].xy * vs_INTERP8.xx) + u_xlat28.xy)).xy, u_xlat28.z);
    u_xlat28 = vec3f((((x_3536.x_MainLightWorldToLight[2i].xy * vs_INTERP8.zz) + u_xlat28.xy)).xy, u_xlat28.z);
    u_xlat28 = vec3f(((u_xlat28.xy + x_3536.x_MainLightWorldToLight[3i].xy)).xy, u_xlat28.z);
    u_xlat28 = vec3f((((u_xlat28.xy * x_670) + x_670)).xy, u_xlat28.z);
    let x_3594 = u_xlat28;
    let x_3597 = x_149.x_GlobalMipBias.x;
    let x_3598 = textureSampleBias(x_MainLightCookieTexture, sampler_MainLightCookieTexture, x_3594.xy, x_3597);
    u_xlat8 = x_3598;
    let x_3607 = vec4f(x_3536.x_MainLightCookieTextureFormat, x_3536.x_MainLightCookieTextureFormat, x_3536.x_MainLightCookieTextureFormat, x_3536.x_MainLightCookieTextureFormat);
    u_xlatb28 = ((vec4f(x_3607.x, x_3607.y, x_3607.z, x_3607.w) == vec4f(0.0f, 1.0f, 0.0f, 0.0f))).xy;
    if (u_xlatb28.y) {
      x_3618 = u_xlat8.w;
    } else {
      x_3618 = u_xlat8.x;
    }
    u_xlat54 = x_3618;
    if (u_xlatb28.x) {
      x_3629 = u_xlat8.xyz;
    } else {
      x_3629 = vec3f(u_xlat54);
    }
    u_xlat28 = x_3629;
  } else {
    u_xlat28.x = 1.0f;
    u_xlat28.y = 1.0f;
    u_xlat28.z = 1.0f;
  }
  u_xlat28 = (u_xlat28 * x_149.x_MainLightColor.xyz);
  u_xlat3.x = dot(-(u_xlat4), u_xlat26);
  u_xlat3.x = (u_xlat3.x + u_xlat3.x);
  u_xlat8 = vec4f((((u_xlat26 * -(u_xlat3.xxx)) + -(u_xlat4))).xyz, u_xlat8.w);
  u_xlat3.x = dot(u_xlat26, u_xlat4);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat3.x = (-(u_xlat3.x) + 1.0f);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat59.x = ((-(u_xlat81) * 0.69999998807907104492f) + 1.70000004768371582031f);
  u_xlat81 = (u_xlat81 * u_xlat59.x);
  u_xlat81 = (u_xlat81 * 6.0f);
  u_xlat8 = textureSampleLevel(unity_SpecCube0, samplerunity_SpecCube0, u_xlat8.xyz, u_xlat81);
  u_xlat81 = (u_xlat8.w + -1.0f);
  u_xlat81 = ((x_83.unity_SpecCube0_HDR.w * u_xlat81) + 1.0f);
  u_xlat81 = max(u_xlat81, 0.0f);
  u_xlat81 = log2(u_xlat81);
  u_xlat81 = (u_xlat81 * x_83.unity_SpecCube0_HDR.y);
  u_xlat81 = exp2(u_xlat81);
  u_xlat81 = (u_xlat81 * x_83.unity_SpecCube0_HDR.x);
  u_xlat8 = vec4f(((u_xlat8.xyz * vec3f(u_xlat81))).xyz, u_xlat8.w);
  u_xlat59 = ((vec2f(u_xlat82) * vec2f(u_xlat82)) + vec2f(-1.0f, 1.0f));
  u_xlat81 = (1.0f / u_xlat59.y);
  u_xlat82 = (u_xlat84 + -0.03999999910593032837f);
  u_xlat3.x = ((u_xlat3.x * u_xlat82) + 0.03999999910593032837f);
  u_xlat3.x = (u_xlat3.x * u_xlat81);
  u_xlat8 = vec4f(((u_xlat3.xxx * u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat5 = vec4f((((u_xlat5.xyz * u_xlat6.xyz) + u_xlat8.xyz)).xyz, u_xlat5.w);
  u_xlat0.x = (u_xlat0.x * x_83.unity_LightData.z);
  u_xlat3.x = dot(u_xlat26, x_149.x_MainLightPosition.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat28 = (u_xlat0.xxx * u_xlat28);
  u_xlat8 = vec4f(((u_xlat4 + x_149.x_MainLightPosition.xyz)).xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat8.xyz, u_xlat8.xyz);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat8 = vec4f(((u_xlat0.xxx * u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat26, u_xlat8.xyz);
  u_xlat0.x = clamp(u_xlat0.x, 0.0f, 1.0f);
  u_xlat3.x = dot(x_149.x_MainLightPosition.xyz, u_xlat8.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat0.x = ((u_xlat0.x * u_xlat59.x) + 1.00001001358032226562f);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat3.x = max(u_xlat3.x, 0.10000000149011611938f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat0.x = (u_xlat7.x * u_xlat0.x);
  u_xlat0.x = (u_xlat83 / u_xlat0.x);
  u_xlat8 = vec4f((((u_xlat0.xxx * x_3914) + u_xlat6.xyz)).xyz, u_xlat8.w);
  u_xlat28 = (u_xlat28 * u_xlat8.xyz);
  u_xlat0.x = min(x_149.x_AdditionalLightsCount.x, x_83.unity_LightData.y);
  u_xlatu0 = bitcast<u32>(i32(u_xlat0.x));
  u_xlat2.x = ((u_xlat2.x * x_372.x_AdditionalShadowFadeParams.x) + x_372.x_AdditionalShadowFadeParams.y);
  u_xlat2.x = clamp(u_xlat2.x, 0.0f, 1.0f);
  let x_3957 = vec4f(x_3536.x_AdditionalLightsCookieAtlasTextureFormat, x_3536.x_AdditionalLightsCookieAtlasTextureFormat, x_3536.x_AdditionalLightsCookieAtlasTextureFormat, x_3536.x_AdditionalLightsCookieAtlasTextureFormat);
  let x_3965 = ((vec4f(x_3957.x, x_3957.y, x_3957.z, x_3957.w) == vec4f(0.0f, 0.0f, 0.0f, 1.0f))).xw;
  u_xlatb3 = vec4<bool>(x_3965.x, u_xlatb3.yz, x_3965.y);
  u_xlat8.x = 0.0f;
  u_xlat8.y = 0.0f;
  u_xlat8.z = 0.0f;
  u_xlatu_loop_1 = 0u;
  loop {
    if ((u_xlatu_loop_1 < u_xlatu0)) {
    } else {
      break;
    }
    u_xlatu84 = (u_xlatu_loop_1 >> 2u);
    u_xlati85 = bitcast<i32>((u_xlatu_loop_1 & 3u));
    let x_3990 = x_83.unity_LightIndices[bitcast<i32>(u_xlatu84)];
    let x_4000 = u_xlati85;
    indexable = array<vec4u, 4u>(vec4u(1065353216u, 0u, 0u, 0u), vec4u(0u, 1065353216u, 0u, 0u), vec4u(0u, 0u, 1065353216u, 0u), vec4u(0u, 0u, 0u, 1065353216u));
    u_xlat84 = dot(x_3990, bitcast<vec4f>(indexable[x_4000]));
    u_xlati84 = i32(u_xlat84);
    u_xlat9 = vec4f((((-(vs_INTERP8) * x_4021.x_AdditionalLightsPosition[u_xlati84].www) + x_4021.x_AdditionalLightsPosition[u_xlati84].xyz)).xyz, u_xlat9.w);
    u_xlat85 = dot(u_xlat9.xyz, u_xlat9.xyz);
    u_xlat85 = max(u_xlat85, 0.00006103515625f);
    u_xlat86 = inverseSqrt(u_xlat85);
    u_xlat10 = vec4f(((vec3f(u_xlat86) * u_xlat9.xyz)).xyz, u_xlat10.w);
    u_xlat87 = (1.0f / u_xlat85);
    u_xlat85 = (u_xlat85 * x_4021.x_AdditionalLightsAttenuation[u_xlati84].x);
    u_xlat85 = ((-(u_xlat85) * u_xlat85) + 1.0f);
    u_xlat85 = max(u_xlat85, 0.0f);
    u_xlat85 = (u_xlat85 * u_xlat85);
    u_xlat85 = (u_xlat85 * u_xlat87);
    u_xlat87 = dot(x_4021.x_AdditionalLightsSpotDir[u_xlati84].xyz, u_xlat10.xyz);
    u_xlat87 = ((u_xlat87 * x_4021.x_AdditionalLightsAttenuation[u_xlati84].z) + x_4021.x_AdditionalLightsAttenuation[u_xlati84].w);
    u_xlat87 = clamp(u_xlat87, 0.0f, 1.0f);
    u_xlat87 = (u_xlat87 * u_xlat87);
    u_xlat85 = (u_xlat85 * u_xlat87);
    u_xlati87 = i32(x_372.x_AdditionalShadowParams[u_xlati84].w);
    u_xlatb88 = (u_xlati87 >= 0i);
    if (u_xlatb88) {
      u_xlatb88 = any(!((vec4f() == vec4f(x_372.x_AdditionalShadowParams[u_xlati84].z))));
      if (u_xlatb88) {
        u_xlatb11 = vec4<bool>(((abs(u_xlat10.zzyz) >= abs(u_xlat10.xyxx))).xyz.xyz, u_xlatb11.w);
        u_xlatb88 = (u_xlatb11.y & u_xlatb11.x);
        let x_4139 = ((-(u_xlat10.zyzx) < vec4f())).xyw;
        u_xlatb11 = vec4<bool>(x_4139.xy, u_xlatb11.z, x_4139.z);
        u_xlat11.x = select(4.0f, 5.0f, u_xlatb11.x);
        u_xlat11.y = select(2.0f, 3.0f, u_xlatb11.y);
        u_xlat89 = select(0.0f, 1.0f, u_xlatb11.w);
        if (u_xlatb11.z) {
          x_4158 = u_xlat11.y;
        } else {
          x_4158 = u_xlat89;
        }
        u_xlat37.x = x_4158;
        if (u_xlatb88) {
          x_4169 = u_xlat11.x;
        } else {
          x_4169 = u_xlat37.x;
        }
        u_xlat88 = x_4169;
        u_xlat11.x = trunc(x_372.x_AdditionalShadowParams[u_xlati84].w);
        u_xlat88 = (u_xlat88 + u_xlat11.x);
        u_xlati87 = i32(u_xlat88);
      }
      u_xlati87 = (u_xlati87 << bitcast<u32>(2i));
      u_xlat11 = (vs_INTERP8.yyyy * x_372.x_AdditionalLightsWorldToShadow[((u_xlati87 + 1i) / 4i)][((u_xlati87 + 1i) % 4i)]);
      u_xlat11 = ((x_372.x_AdditionalLightsWorldToShadow[(u_xlati87 / 4i)][(u_xlati87 % 4i)] * vs_INTERP8.xxxx) + u_xlat11);
      u_xlat11 = ((x_372.x_AdditionalLightsWorldToShadow[((u_xlati87 + 2i) / 4i)][((u_xlati87 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat11);
      u_xlat11 = (u_xlat11 + x_372.x_AdditionalLightsWorldToShadow[((u_xlati87 + 3i) / 4i)][((u_xlati87 + 3i) % 4i)]);
      u_xlat11 = vec4f(((u_xlat11.xyz / u_xlat11.www)).xyz, u_xlat11.w);
      u_xlatb87 = (0.0f < x_372.x_AdditionalShadowParams[u_xlati84].y);
      if (u_xlatb87) {
        u_xlatb87 = (1.0f == x_372.x_AdditionalShadowParams[u_xlati84].y);
        if (u_xlatb87) {
          u_xlat12 = (u_xlat11.xyxy + x_372.x_AdditionalShadowOffset0);
          let x_4267 = u_xlat12.xy;
          txVec60 = vec3f(x_4267.x, x_4267.y, u_xlat11.z);
          u_xlat13.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec60.xy, txVec60.z);
          let x_4283 = u_xlat12.zw;
          txVec61 = vec3f(x_4283.x, x_4283.y, u_xlat11.z);
          u_xlat13.y = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec61.xy, txVec61.z);
          u_xlat12 = (u_xlat11.xyxy + x_372.x_AdditionalShadowOffset1);
          let x_4303 = u_xlat12.xy;
          txVec62 = vec3f(x_4303.x, x_4303.y, u_xlat11.z);
          u_xlat13.z = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec62.xy, txVec62.z);
          let x_4318 = u_xlat12.zw;
          txVec63 = vec3f(x_4318.x, x_4318.y, u_xlat11.z);
          u_xlat13.w = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec63.xy, txVec63.z);
          u_xlat87 = dot(u_xlat13, x_652);
        } else {
          u_xlatb88 = (2.0f == x_372.x_AdditionalShadowParams[u_xlati84].y);
          if (u_xlatb88) {
            u_xlat12 = vec4f((((u_xlat11.xy * x_372.x_AdditionalShadowmapSize.zw) + x_670)).xy, u_xlat12.zw);
            u_xlat12 = vec4f(floor(u_xlat12.xy).xy, u_xlat12.zw);
            u_xlat64 = ((u_xlat11.xy * x_372.x_AdditionalShadowmapSize.zw) + -(u_xlat12.xy));
            u_xlat13 = (u_xlat64.xxyy + x_693);
            u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
            let x_4377 = (u_xlat14.yw * x_703);
            u_xlat13 = vec4f(x_4377.x, u_xlat13.y, x_4377.y, u_xlat13.w);
            u_xlat14 = vec4f((((u_xlat14.xz * x_670) + -(u_xlat64))).xy, u_xlat14.zw);
            u_xlat66 = (-(u_xlat64) + x_718);
            u_xlat15 = vec4f(min(u_xlat64, vec2f()).xy, u_xlat15.zw);
            u_xlat15 = vec4f((((-(u_xlat15.xy) * u_xlat15.xy) + u_xlat66)).xy, u_xlat15.zw);
            u_xlat64 = max(u_xlat64, vec2f());
            u_xlat64 = ((-(u_xlat64) * u_xlat64) + u_xlat13.yw);
            u_xlat15 = vec4f(((u_xlat15.xy + x_718)).xy, u_xlat15.zw);
            u_xlat64 = (u_xlat64 + x_718);
            u_xlat16 = vec4f(((u_xlat14.xy * x_756)).xy, u_xlat16.zw);
            u_xlat14 = vec4f(((u_xlat66 * x_756)).xy, u_xlat14.zw);
            u_xlat15 = vec4f(((u_xlat15.xy * x_756)).xy, u_xlat15.zw);
            u_xlat17 = vec4f(((u_xlat64 * x_756)).xy, u_xlat17.zw);
            u_xlat64 = (u_xlat13.yw * x_756);
            u_xlat16.z = u_xlat15.x;
            u_xlat16.w = u_xlat64.x;
            u_xlat14.z = u_xlat17.x;
            u_xlat14.w = u_xlat13.x;
            u_xlat18 = (u_xlat14.zwxz + u_xlat16.zwxz);
            u_xlat15.z = u_xlat16.y;
            u_xlat15.w = u_xlat64.y;
            u_xlat17.z = u_xlat14.y;
            u_xlat17.w = u_xlat13.z;
            u_xlat13 = vec4f(((u_xlat15.zyw + u_xlat17.zyw)).xyz, u_xlat13.w);
            u_xlat14 = vec4f(((u_xlat14.xzw / u_xlat18.zwy)).xyz, u_xlat14.w);
            u_xlat14 = vec4f(((u_xlat14.xyz + x_826)).xyz, u_xlat14.w);
            u_xlat15 = vec4f(((u_xlat17.zyw / u_xlat13.xyz)).xyz, u_xlat15.w);
            u_xlat15 = vec4f(((u_xlat15.xyz + x_826)).xyz, u_xlat15.w);
            u_xlat14 = vec4f(((u_xlat14.yxz * x_372.x_AdditionalShadowmapSize.xxx)).xyz, u_xlat14.w);
            u_xlat15 = vec4f(((u_xlat15.xyz * x_372.x_AdditionalShadowmapSize.yyy)).xyz, u_xlat15.w);
            u_xlat14.w = u_xlat15.x;
            u_xlat16 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat14.ywxw);
            u_xlat64 = ((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat14.zw);
            u_xlat15.w = u_xlat14.y;
            let x_4543 = u_xlat15.yz;
            u_xlat14 = vec4f(u_xlat14.x, x_4543.x, u_xlat14.z, x_4543.y);
            u_xlat17 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat14.xyzy);
            u_xlat15 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat15.wywz);
            u_xlat14 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat14.xwzw);
            u_xlat19 = (u_xlat13.xxxy * u_xlat18.zwyz);
            u_xlat20 = (u_xlat13.yyzz * u_xlat18);
            u_xlat88 = (u_xlat13.z * u_xlat18.y);
            let x_4589 = u_xlat16.xy;
            txVec64 = vec3f(x_4589.x, x_4589.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec64.xy, txVec64.z);
            let x_4603 = u_xlat16.zw;
            txVec65 = vec3f(x_4603.x, x_4603.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec65.xy, txVec65.z);
            u_xlat12.x = (u_xlat12.x * u_xlat19.y);
            u_xlat89 = ((u_xlat19.x * u_xlat89) + u_xlat12.x);
            txVec66 = vec3f(u_xlat64.x, u_xlat64.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec66.xy, txVec66.z);
            u_xlat89 = ((u_xlat19.z * u_xlat12.x) + u_xlat89);
            let x_4652 = u_xlat15.xy;
            txVec67 = vec3f(x_4652.x, x_4652.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec67.xy, txVec67.z);
            u_xlat89 = ((u_xlat19.w * u_xlat12.x) + u_xlat89);
            let x_4674 = u_xlat17.xy;
            txVec68 = vec3f(x_4674.x, x_4674.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec68.xy, txVec68.z);
            u_xlat89 = ((u_xlat20.x * u_xlat12.x) + u_xlat89);
            let x_4696 = u_xlat17.zw;
            txVec69 = vec3f(x_4696.x, x_4696.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec69.xy, txVec69.z);
            u_xlat89 = ((u_xlat20.y * u_xlat12.x) + u_xlat89);
            let x_4718 = u_xlat15.zw;
            txVec70 = vec3f(x_4718.x, x_4718.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec70.xy, txVec70.z);
            u_xlat89 = ((u_xlat20.z * u_xlat12.x) + u_xlat89);
            let x_4740 = u_xlat14.xy;
            txVec71 = vec3f(x_4740.x, x_4740.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec71.xy, txVec71.z);
            u_xlat89 = ((u_xlat20.w * u_xlat12.x) + u_xlat89);
            let x_4762 = u_xlat14.zw;
            txVec72 = vec3f(x_4762.x, x_4762.y, u_xlat11.z);
            u_xlat12.x = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec72.xy, txVec72.z);
            u_xlat87 = ((u_xlat88 * u_xlat12.x) + u_xlat89);
          } else {
            u_xlat12 = vec4f((((u_xlat11.xy * x_372.x_AdditionalShadowmapSize.zw) + x_670)).xy, u_xlat12.zw);
            u_xlat12 = vec4f(floor(u_xlat12.xy).xy, u_xlat12.zw);
            u_xlat64 = ((u_xlat11.xy * x_372.x_AdditionalShadowmapSize.zw) + -(u_xlat12.xy));
            u_xlat13 = (u_xlat64.xxyy + x_693);
            u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
            let x_4816 = (u_xlat14.yw * x_1159);
            u_xlat15 = vec4f(u_xlat15.x, x_4816.x, u_xlat15.z, x_4816.y);
            let x_4824 = ((u_xlat14.xz * x_670) + -(u_xlat64));
            u_xlat13 = vec4f(x_4824.x, u_xlat13.y, x_4824.y, u_xlat13.w);
            u_xlat14 = vec4f(((-(u_xlat64) + x_718)).xy, u_xlat14.zw);
            u_xlat66 = min(u_xlat64, vec2f());
            u_xlat14 = vec4f((((-(u_xlat66) * u_xlat66) + u_xlat14.xy)).xy, u_xlat14.zw);
            u_xlat66 = max(u_xlat64, vec2f());
            let x_4852 = ((-(u_xlat66) * u_xlat66) + u_xlat13.yw);
            u_xlat39 = vec3f(x_4852.x, u_xlat39.y, x_4852.y);
            u_xlat14 = vec4f(((u_xlat14.xy + x_1201)).xy, u_xlat14.zw);
            let x_4862 = (u_xlat39.xz + x_1201);
            u_xlat13 = vec4f(u_xlat13.x, x_4862.x, u_xlat13.z, x_4862.y);
            u_xlat16.z = (u_xlat13.y * 0.08163200318813323975f);
            u_xlat17 = vec4f(((u_xlat13.zxw * x_1217)).xyz, u_xlat17.w);
            u_xlat13 = vec4f(((u_xlat14.xy * x_1223)).xy, u_xlat13.zw);
            u_xlat16.x = u_xlat17.y;
            let x_4885 = ((u_xlat64.xx * x_1233) + x_1236);
            u_xlat16 = vec4f(u_xlat16.x, x_4885.x, u_xlat16.z, x_4885.y);
            let x_4891 = ((u_xlat64.xx * x_1233) + x_1243);
            u_xlat14 = vec4f(x_4891.x, u_xlat14.y, x_4891.y, u_xlat14.w);
            u_xlat14.y = u_xlat13.x;
            u_xlat14.w = u_xlat15.y;
            u_xlat16 = (u_xlat14 + u_xlat16);
            let x_4906 = ((u_xlat64.yy * x_1233) + x_1236);
            u_xlat17 = vec4f(u_xlat17.x, x_4906.x, u_xlat17.z, x_4906.y);
            let x_4912 = ((u_xlat64.yy * x_1233) + x_1243);
            u_xlat15 = vec4f(x_4912.x, u_xlat15.y, x_4912.y, u_xlat15.w);
            u_xlat15.y = u_xlat13.y;
            u_xlat13 = (u_xlat15 + u_xlat17);
            u_xlat14 = (u_xlat14 / u_xlat16);
            u_xlat14 = (u_xlat14 + x_1281);
            u_xlat15 = (u_xlat15 / u_xlat13);
            u_xlat15 = (u_xlat15 + x_1281);
            u_xlat14 = (u_xlat14.wxyz * x_372.x_AdditionalShadowmapSize.xxxx);
            u_xlat15 = (u_xlat15.xwyz * x_372.x_AdditionalShadowmapSize.yyyy);
            let x_4944 = u_xlat14.yzw;
            u_xlat17 = vec4f(x_4944.x, u_xlat17.y, x_4944.yz);
            u_xlat17.y = u_xlat15.x;
            u_xlat18 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
            u_xlat64 = ((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat17.wy);
            u_xlat14.y = u_xlat17.y;
            u_xlat17.y = u_xlat15.z;
            u_xlat19 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
            u_xlat20 = vec4f((((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat17.wy)).xy, u_xlat20.zw);
            u_xlat14.z = u_xlat17.y;
            u_xlat21 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat14.xyxz);
            u_xlat17.y = u_xlat15.w;
            u_xlat22 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
            u_xlat40 = ((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat17.wy);
            u_xlat14.w = u_xlat17.y;
            u_xlat72 = ((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat14.xw);
            let x_5043 = u_xlat17.xzw;
            u_xlat15 = vec4f(x_5043.x, u_xlat15.y, x_5043.yz);
            u_xlat17 = ((u_xlat12.xyxy * x_372.x_AdditionalShadowmapSize.xyxy) + u_xlat15.xyzy);
            u_xlat67 = ((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat15.wy);
            u_xlat15.x = u_xlat14.x;
            u_xlat12 = vec4f((((u_xlat12.xy * x_372.x_AdditionalShadowmapSize.xy) + u_xlat15.xy)).xy, u_xlat12.zw);
            u_xlat23 = (u_xlat13.xxxx * u_xlat16);
            u_xlat24 = (u_xlat13.yyyy * u_xlat16);
            u_xlat25 = (u_xlat13.zzzz * u_xlat16);
            u_xlat13 = (u_xlat13.wwww * u_xlat16);
            let x_5100 = u_xlat18.xy;
            txVec73 = vec3f(x_5100.x, x_5100.y, u_xlat11.z);
            u_xlat88 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec73.xy, txVec73.z);
            let x_5114 = u_xlat18.zw;
            txVec74 = vec3f(x_5114.x, x_5114.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec74.xy, txVec74.z);
            u_xlat89 = (u_xlat89 * u_xlat23.y);
            u_xlat88 = ((u_xlat23.x * u_xlat88) + u_xlat89);
            txVec75 = vec3f(u_xlat64.x, u_xlat64.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec75.xy, txVec75.z);
            u_xlat88 = ((u_xlat23.z * u_xlat89) + u_xlat88);
            let x_5157 = u_xlat21.xy;
            txVec76 = vec3f(x_5157.x, x_5157.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec76.xy, txVec76.z);
            u_xlat88 = ((u_xlat23.w * u_xlat89) + u_xlat88);
            let x_5177 = u_xlat19.xy;
            txVec77 = vec3f(x_5177.x, x_5177.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec77.xy, txVec77.z);
            u_xlat88 = ((u_xlat24.x * u_xlat89) + u_xlat88);
            let x_5197 = u_xlat19.zw;
            txVec78 = vec3f(x_5197.x, x_5197.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec78.xy, txVec78.z);
            u_xlat88 = ((u_xlat24.y * u_xlat89) + u_xlat88);
            let x_5217 = u_xlat20.xy;
            txVec79 = vec3f(x_5217.x, x_5217.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec79.xy, txVec79.z);
            u_xlat88 = ((u_xlat24.z * u_xlat89) + u_xlat88);
            let x_5237 = u_xlat21.zw;
            txVec80 = vec3f(x_5237.x, x_5237.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec80.xy, txVec80.z);
            u_xlat88 = ((u_xlat24.w * u_xlat89) + u_xlat88);
            let x_5257 = u_xlat22.xy;
            txVec81 = vec3f(x_5257.x, x_5257.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec81.xy, txVec81.z);
            u_xlat88 = ((u_xlat25.x * u_xlat89) + u_xlat88);
            let x_5277 = u_xlat22.zw;
            txVec82 = vec3f(x_5277.x, x_5277.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec82.xy, txVec82.z);
            u_xlat88 = ((u_xlat25.y * u_xlat89) + u_xlat88);
            txVec83 = vec3f(u_xlat40.x, u_xlat40.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec83.xy, txVec83.z);
            u_xlat88 = ((u_xlat25.z * u_xlat89) + u_xlat88);
            txVec84 = vec3f(u_xlat72.x, u_xlat72.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec84.xy, txVec84.z);
            u_xlat88 = ((u_xlat25.w * u_xlat89) + u_xlat88);
            let x_5335 = u_xlat17.xy;
            txVec85 = vec3f(x_5335.x, x_5335.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec85.xy, txVec85.z);
            u_xlat88 = ((u_xlat13.x * u_xlat89) + u_xlat88);
            let x_5355 = u_xlat17.zw;
            txVec86 = vec3f(x_5355.x, x_5355.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec86.xy, txVec86.z);
            u_xlat88 = ((u_xlat13.y * u_xlat89) + u_xlat88);
            txVec87 = vec3f(u_xlat67.x, u_xlat67.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec87.xy, txVec87.z);
            u_xlat88 = ((u_xlat13.z * u_xlat89) + u_xlat88);
            let x_5394 = u_xlat12.xy;
            txVec88 = vec3f(x_5394.x, x_5394.y, u_xlat11.z);
            u_xlat89 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec88.xy, txVec88.z);
            u_xlat87 = ((u_xlat13.w * u_xlat89) + u_xlat88);
          }
        }
      } else {
        let x_5415 = u_xlat11.xy;
        txVec89 = vec3f(x_5415.x, x_5415.y, u_xlat11.z);
        u_xlat87 = textureSampleCompareLevel(x_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, txVec89.xy, txVec89.z);
      }
      u_xlat88 = (1.0f + -(x_372.x_AdditionalShadowParams[u_xlati84].x));
      u_xlat87 = ((u_xlat87 * x_372.x_AdditionalShadowParams[u_xlati84].x) + u_xlat88);
      u_xlatb88 = (0.0f >= u_xlat11.z);
      u_xlatb11.x = (u_xlat11.z >= 1.0f);
      u_xlatb88 = (u_xlatb88 | u_xlatb11.x);
      u_xlat87 = select(u_xlat87, 1.0f, u_xlatb88);
    } else {
      u_xlat87 = 1.0f;
    }
    u_xlat88 = (-(u_xlat87) + 1.0f);
    u_xlat87 = ((u_xlat2.x * u_xlat88) + u_xlat87);
    u_xlati88 = (1i << bitcast<u32>((u_xlati84 & 31i)));
    u_xlati88 = bitcast<i32>((bitcast<u32>(u_xlati88) & bitcast<u32>(x_3536.x_AdditionalLightsCookieEnableBits)));
    if ((u_xlati88 != 0i)) {
      u_xlati88 = i32(x_3536.x_AdditionalLightsLightTypes[u_xlati84].el);
      u_xlati11 = select(1i, 0i, (u_xlati88 != 0i));
      u_xlati37 = (u_xlati84 << bitcast<u32>(2i));
      if ((u_xlati11 != 0i)) {
        let x_5505 = (vs_INTERP8.yyy * x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xyw);
        u_xlat11 = vec4f(x_5505.x, u_xlat11.y, x_5505.yz);
        let x_5520 = ((x_3536.x_AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xyw * vs_INTERP8.xxx) + u_xlat11.xzw);
        u_xlat11 = vec4f(x_5520.x, u_xlat11.y, x_5520.yz);
        let x_5537 = ((x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xyw * vs_INTERP8.zzz) + u_xlat11.xzw);
        u_xlat11 = vec4f(x_5537.x, u_xlat11.y, x_5537.yz);
        let x_5551 = (u_xlat11.xzw + x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xyw);
        u_xlat11 = vec4f(x_5551.x, u_xlat11.y, x_5551.yz);
        let x_5558 = (u_xlat11.xz / u_xlat11.ww);
        u_xlat11 = vec4f(x_5558.x, u_xlat11.y, x_5558.y, u_xlat11.w);
        let x_5564 = ((u_xlat11.xz * x_670) + x_670);
        u_xlat11 = vec4f(x_5564.x, u_xlat11.y, x_5564.y, u_xlat11.w);
        let x_5571 = clamp(u_xlat11.xz, vec2f(0.0f), vec2f(1.0f));
        u_xlat11 = vec4f(x_5571.x, u_xlat11.y, x_5571.y, u_xlat11.w);
        let x_5585 = ((x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat11.xz) + x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
        u_xlat11 = vec4f(x_5585.x, u_xlat11.y, x_5585.y, u_xlat11.w);
      } else {
        u_xlatb88 = (u_xlati88 == 1i);
        u_xlati88 = select(0i, 1i, u_xlatb88);
        if ((u_xlati88 != 0i)) {
          u_xlat12 = vec4f(((vs_INTERP8.yy * x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xy)).xy, u_xlat12.zw);
          u_xlat12 = vec4f((((x_3536.x_AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xy * vs_INTERP8.xx) + u_xlat12.xy)).xy, u_xlat12.zw);
          u_xlat12 = vec4f((((x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xy * vs_INTERP8.zz) + u_xlat12.xy)).xy, u_xlat12.zw);
          u_xlat12 = vec4f(((u_xlat12.xy + x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xy)).xy, u_xlat12.zw);
          u_xlat12 = vec4f((((u_xlat12.xy * x_670) + x_670)).xy, u_xlat12.zw);
          u_xlat12 = vec4f(fract(u_xlat12.xy).xy, u_xlat12.zw);
          let x_5679 = ((x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat12.xy) + x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
          u_xlat11 = vec4f(x_5679.x, u_xlat11.y, x_5679.y, u_xlat11.w);
        } else {
          u_xlat12 = (vs_INTERP8.yyyy * x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)]);
          u_xlat12 = ((x_3536.x_AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)] * vs_INTERP8.xxxx) + u_xlat12);
          u_xlat12 = ((x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat12);
          u_xlat12 = (u_xlat12 + x_3536.x_AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)]);
          u_xlat12 = vec4f(((u_xlat12.xyz / u_xlat12.www)).xyz, u_xlat12.w);
          u_xlat88 = dot(u_xlat12.xyz, u_xlat12.xyz);
          u_xlat88 = inverseSqrt(u_xlat88);
          u_xlat12 = vec4f(((vec3f(u_xlat88) * u_xlat12.xyz)).xyz, u_xlat12.w);
          u_xlat88 = dot(abs(u_xlat12.xyz), vec3f(1.0f));
          u_xlat88 = max(u_xlat88, 0.00000099999999747524f);
          u_xlat88 = (1.0f / u_xlat88);
          u_xlat13 = vec4f(((vec3f(u_xlat88) * u_xlat12.zxy)).xyz, u_xlat13.w);
          u_xlat13.x = -(u_xlat13.x);
          u_xlat13.x = clamp(u_xlat13.x, 0.0f, 1.0f);
          let x_5779 = ((u_xlat13.yyzz >= vec4f())).xz;
          u_xlatb37 = vec3<bool>(x_5779.x, u_xlatb37.y, x_5779.y);
          if (u_xlatb37.x) {
            x_5784 = u_xlat13.x;
          } else {
            x_5784 = -(u_xlat13.x);
          }
          u_xlat37.x = x_5784;
          if (u_xlatb37.z) {
            x_5797 = u_xlat13.x;
          } else {
            x_5797 = -(u_xlat13.x);
          }
          u_xlat37.z = x_5797;
          let x_5815 = ((u_xlat12.xy * vec2f(u_xlat88)) + u_xlat37.xz);
          u_xlat37 = vec3f(x_5815.x, u_xlat37.y, x_5815.y);
          let x_5821 = ((u_xlat37.xz * x_670) + x_670);
          u_xlat37 = vec3f(x_5821.x, u_xlat37.y, x_5821.y);
          let x_5828 = clamp(u_xlat37.xz, vec2f(0.0f), vec2f(1.0f));
          u_xlat37 = vec3f(x_5828.x, u_xlat37.y, x_5828.y);
          let x_5842 = ((x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat37.xz) + x_3536.x_AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
          u_xlat11 = vec4f(x_5842.x, u_xlat11.y, x_5842.y, u_xlat11.w);
        }
      }
      u_xlat11 = textureSampleLevel(x_AdditionalLightsCookieAtlasTexture, sampler_LinearClamp, u_xlat11.xz, 0.0f);
      if (u_xlatb3.w) {
        x_5855 = u_xlat11.w;
      } else {
        x_5855 = u_xlat11.x;
      }
      u_xlat88 = x_5855;
      if (u_xlatb3.x) {
        x_5866 = u_xlat11.xyz;
      } else {
        x_5866 = vec3f(u_xlat88);
      }
      u_xlat11 = vec4f(x_5866.xyz, u_xlat11.w);
    } else {
      u_xlat11.x = 1.0f;
      u_xlat11.y = 1.0f;
      u_xlat11.z = 1.0f;
    }
    u_xlat11 = vec4f(((u_xlat11.xyz * x_4021.x_AdditionalLightsColor[u_xlati84].xyz)).xyz, u_xlat11.w);
    u_xlat84 = (u_xlat85 * u_xlat87);
    u_xlat85 = dot(u_xlat26, u_xlat10.xyz);
    u_xlat85 = clamp(u_xlat85, 0.0f, 1.0f);
    u_xlat84 = (u_xlat84 * u_xlat85);
    u_xlat11 = vec4f(((vec3f(u_xlat84) * u_xlat11.xyz)).xyz, u_xlat11.w);
    u_xlat9 = vec4f((((u_xlat9.xyz * vec3f(u_xlat86)) + u_xlat4)).xyz, u_xlat9.w);
    u_xlat84 = dot(u_xlat9.xyz, u_xlat9.xyz);
    u_xlat84 = max(u_xlat84, 1.17549435e-38f);
    u_xlat84 = inverseSqrt(u_xlat84);
    u_xlat9 = vec4f(((vec3f(u_xlat84) * u_xlat9.xyz)).xyz, u_xlat9.w);
    u_xlat84 = dot(u_xlat26, u_xlat9.xyz);
    u_xlat84 = clamp(u_xlat84, 0.0f, 1.0f);
    u_xlat85 = dot(u_xlat10.xyz, u_xlat9.xyz);
    u_xlat85 = clamp(u_xlat85, 0.0f, 1.0f);
    u_xlat84 = (u_xlat84 * u_xlat84);
    u_xlat84 = ((u_xlat84 * u_xlat59.x) + 1.00001001358032226562f);
    u_xlat85 = (u_xlat85 * u_xlat85);
    u_xlat84 = (u_xlat84 * u_xlat84);
    u_xlat85 = max(u_xlat85, 0.10000000149011611938f);
    u_xlat84 = (u_xlat84 * u_xlat85);
    u_xlat84 = (u_xlat7.x * u_xlat84);
    u_xlat84 = (u_xlat83 / u_xlat84);
    u_xlat9 = vec4f((((vec3f(u_xlat84) * x_3914) + u_xlat6.xyz)).xyz, u_xlat9.w);
    u_xlat8 = vec4f((((u_xlat9.xyz * u_xlat11.xyz) + u_xlat8.xyz)).xyz, u_xlat8.w);

    continuing {
      u_xlatu_loop_1 = (u_xlatu_loop_1 + bitcast<u32>(1i));
    }
  }
  u_xlat0 = ((u_xlat5.xyz * vec3f(u_xlat33)) + u_xlat28);
  u_xlat0 = (u_xlat8.xyz + u_xlat0);
  u_xlat0 = ((vs_INTERP6.www * u_xlat1) + u_xlat0);
  u_xlat78 = (u_xlat55.x * -(u_xlat55.x));
  u_xlat78 = exp2(u_xlat78);
  u_xlat0 = (u_xlat0 + -(x_149.unity_FogColor.xyz));
  SV_Target0 = vec4f((((vec3f(u_xlat78) * u_xlat0) + x_149.unity_FogColor.xyz)).xyz, SV_Target0.w);
  SV_Target0.w = select(1.0f, u_xlat79, u_xlatb29);
  return;
}

struct main_out {
  @location(0)
  SV_Target0_1 : vec4f,
}

@fragment
fn main(@location(5) vs_INTERP9_param : vec3f, @location(1) vs_INTERP4_param : vec4f, @location(4) vs_INTERP8_param : vec3f, @location(2) vs_INTERP5_param : vec4f, @location(3) vs_INTERP6_param : vec4f, @location(0) vs_INTERP0_param : vec2f, @builtin(position) gl_FragCoord_param : vec4f) -> main_out {
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

