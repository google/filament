diagnostic(off, derivative_uniformity);

var<private> u_xlat0 : vec3<f32>;

var<private> u_xlatb26 : vec2<bool>;

struct UnityPerDraw_1 {
  unity_ObjectToWorld : mat4x4<f32>,
  unity_WorldToObject : mat4x4<f32>,
  unity_LODFade : vec4<f32>,
  unity_WorldTransformParams : vec4<f32>,
  unity_RenderingLayer : vec4<f32>,
  unity_LightData : vec4<f32>,
  unity_LightIndices : array<vec4<f32>, 2u>,
  unity_ProbesOcclusion : vec4<f32>,
  unity_SpecCube0_HDR : vec4<f32>,
  unity_SpecCube1_HDR : vec4<f32>,
  unity_SpecCube0_BoxMax : vec4<f32>,
  unity_SpecCube0_BoxMin : vec4<f32>,
  unity_SpecCube0_ProbePosition : vec4<f32>,
  unity_SpecCube1_BoxMax : vec4<f32>,
  unity_SpecCube1_BoxMin : vec4<f32>,
  unity_SpecCube1_ProbePosition : vec4<f32>,
  unity_LightmapST : vec4<f32>,
  unity_DynamicLightmapST : vec4<f32>,
  unity_SHAr : vec4<f32>,
  unity_SHAg : vec4<f32>,
  unity_SHAb : vec4<f32>,
  unity_SHBr : vec4<f32>,
  unity_SHBg : vec4<f32>,
  unity_SHBb : vec4<f32>,
  unity_SHC : vec4<f32>,
  unity_RendererBounds_Min : vec4<f32>,
  unity_RendererBounds_Max : vec4<f32>,
  unity_MatrixPreviousM : mat4x4<f32>,
  unity_MatrixPreviousMI : mat4x4<f32>,
  unity_MotionVectorsParams : vec4<f32>,
  unity_SpriteColor : vec4<f32>,
  unity_SpriteProps : vec4<f32>,
}

@group(1u) @binding(2u) var<uniform> v : UnityPerDraw_1;

var<private> u_xlat26 : vec3<f32>;

var<private> u_xlat1 : vec3<f32>;

var<private> u_xlat2 : vec4<f32>;

var<private> u_xlat3 : vec4<f32>;

var<private> u_xlatb0 : bool;

struct PGlobals_1 {
  _GlobalMipBias : vec2<f32>,
  _AlphaToMaskAvailable : f32,
  _MainLightPosition : vec4<f32>,
  _MainLightColor : vec4<f32>,
  _AdditionalLightsCount : vec4<f32>,
  _WorldSpaceCameraPos : vec3<f32>,
  _ProjectionParams : vec4<f32>,
  _ScreenParams : vec4<f32>,
  unity_OrthoParams : vec4<f32>,
  unity_FogParams : vec4<f32>,
  unity_FogColor : vec4<f32>,
  unity_MatrixV : mat4x4<f32>,
}

@group(1u) @binding(0u) var<uniform> v_1 : PGlobals_1;

var<private> u_xlat4 : vec3<f32>;

var<private> u_xlat79 : f32;

var<private> u_xlat5 : vec4<f32>;

var<private> u_xlat6 : vec4<f32>;

@group(0u) @binding(7u) var Texture2D_B222E8F : texture_2d<f32>;

@group(0u) @binding(14u) var samplerTexture2D_B222E8F : sampler;

var<private> u_xlat7 : vec3<f32>;

struct UnityPerMaterial {
  Texture2D_B222E8F_TexelSize : vec4<f32>,
  Color_C30C7CA3 : vec4<f32>,
  Texture2D_D9BFD5F1_TexelSize : vec4<f32>,
}

@group(1u) @binding(5u) var<uniform> v_2 : UnityPerMaterial;

var<private> u_xlat8 : vec4<f32>;

@group(0u) @binding(8u) var Texture2D_D9BFD5F1 : texture_2d<f32>;

@group(0u) @binding(15u) var samplerTexture2D_D9BFD5F1 : sampler;

var<private> u_xlat9 : vec4<f32>;

var<private> u_xlat34 : vec3<f32>;

struct LightShadows_1 {
  _MainLightWorldToShadow : array<mat4x4<f32>, 5u>,
  _CascadeShadowSplitSpheres0 : vec4<f32>,
  _CascadeShadowSplitSpheres1 : vec4<f32>,
  _CascadeShadowSplitSpheres2 : vec4<f32>,
  _CascadeShadowSplitSpheres3 : vec4<f32>,
  _CascadeShadowSplitSphereRadii : vec4<f32>,
  _MainLightShadowOffset0 : vec4<f32>,
  _MainLightShadowOffset1 : vec4<f32>,
  _MainLightShadowParams : vec4<f32>,
  _MainLightShadowmapSize : vec4<f32>,
  _AdditionalShadowOffset0 : vec4<f32>,
  _AdditionalShadowOffset1 : vec4<f32>,
  _AdditionalShadowFadeParams : vec4<f32>,
  _AdditionalShadowmapSize : vec4<f32>,
  _AdditionalShadowParams : array<vec4<f32>, 32u>,
  _AdditionalLightsWorldToShadow : array<mat4x4<f32>, 32u>,
}

@group(1u) @binding(3u) var<uniform> v_3 : LightShadows_1;

var<private> u_xlat10 : vec4<f32>;

var<private> u_xlatb2 : vec4<bool>;

var<private> u_xlatu0 : u32;

var<private> u_xlati0 : i32;

var<private> u_xlatb79 : bool;

@group(0u) @binding(3u) var _MainLightShadowmapTexture : texture_depth_2d;

@group(0u) @binding(12u) var sampler_LinearClampCompare : sampler_comparison;

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

@group(0u) @binding(2u) var unity_LightmapInd : texture_2d<f32>;

@group(0u) @binding(10u) var samplerunity_Lightmap : sampler;

@group(0u) @binding(1u) var unity_Lightmap : texture_2d<f32>;

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

struct tint_padded_array_element {
  @size(16u)
  tint_element : f32,
}

struct LightCookies_1_1 {
  _MainLightWorldToLight : mat4x4<f32>,
  _AdditionalLightsCookieEnableBits : f32,
  _MainLightCookieTextureFormat : f32,
  _AdditionalLightsCookieAtlasTextureFormat : f32,
  _AdditionalLightsWorldToLights : array<mat4x4<f32>, 32u>,
  _AdditionalLightsCookieAtlasUVRects : array<vec4<f32>, 32u>,
  _AdditionalLightsLightTypes : array<tint_padded_array_element, 32u>,
}

@group(1u) @binding(4u) var<uniform> v_4 : LightCookies_1_1;

@group(0u) @binding(5u) var _MainLightCookieTexture : texture_2d<f32>;

@group(0u) @binding(13u) var sampler_MainLightCookieTexture : sampler;

@group(0u) @binding(0u) var unity_SpecCube0 : texture_cube<f32>;

@group(0u) @binding(9u) var samplerunity_SpecCube0 : sampler;

var<private> u_xlatu84 : u32;

var<private> u_xlati85 : i32;

var<private> u_xlati84 : i32;

struct AdditionalLights_1 {
  _AdditionalLightsPosition : array<vec4<f32>, 32u>,
  _AdditionalLightsColor : array<vec4<f32>, 32u>,
  _AdditionalLightsAttenuation : array<vec4<f32>, 32u>,
  _AdditionalLightsSpotDir : array<vec4<f32>, 32u>,
  _AdditionalLightsOcclusionProbes : array<vec4<f32>, 32u>,
  _AdditionalLightsLayerMasks : array<tint_padded_array_element, 32u>,
}

@group(1u) @binding(1u) var<uniform> v_5 : AdditionalLights_1;

var<private> u_xlat86 : f32;

var<private> u_xlati87 : i32;

var<private> u_xlatb88 : bool;

var<private> u_xlatb11 : vec4<bool>;

var<private> u_xlat89 : f32;

var<private> u_xlat37 : vec3<f32>;

var<private> u_xlat88 : f32;

var<private> u_xlatb87 : bool;

@group(0u) @binding(4u) var _AdditionalLightsShadowmapTexture : texture_depth_2d;

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

@group(0u) @binding(6u) var _AdditionalLightsCookieAtlasTexture : texture_2d<f32>;

@group(0u) @binding(11u) var sampler_LinearClamp : sampler;

var<private> u_xlat78 : f32;

var<private> SV_Target0 : vec4<f32>;

fn main_inner(vs_INTERP9 : vec3<f32>, vs_INTERP4 : vec4<f32>, vs_INTERP8 : vec3<f32>, vs_INTERP5 : vec4<f32>, vs_INTERP6 : vec4<f32>, vs_INTERP0 : vec2<f32>, gl_FragCoord : vec4<f32>) {
  var v_6 : vec3<f32>;
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
  var v_7 : f32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var v_8 : f32;
  var v_9 : f32;
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
  var v_10 : f32;
  var v_11 : f32;
  var v_12 : vec3<f32>;
  var u_xlatu_loop_1 : u32;
  var indexable : array<vec4<u32>, 4u>;
  var v_13 : f32;
  var v_14 : f32;
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
  var v_15 : f32;
  var v_16 : f32;
  var v_17 : f32;
  var v_18 : vec3<f32>;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  u_xlat0.x = dot(vs_INTERP9, vs_INTERP9);
  u_xlat0.x = sqrt(u_xlat0.x);
  u_xlat0.x = (1.0f / u_xlat0.x);
  u_xlatb26.x = (0.0f < vs_INTERP4.w);
  u_xlatb26.y = (v.unity_WorldTransformParams.w >= 0.0f);
  u_xlat26.x = select(-1.0f, 1.0f, u_xlatb26.x);
  u_xlat26.y = select(-1.0f, 1.0f, u_xlatb26.y);
  u_xlat26.x = (u_xlat26.y * u_xlat26.x);
  u_xlat1 = (vs_INTERP4.yzx * vs_INTERP9.zxy);
  u_xlat1 = ((vs_INTERP9.yzx * vs_INTERP4.zxy) + -(u_xlat1));
  u_xlat26 = (u_xlat26.xxx * u_xlat1);
  u_xlat1 = (u_xlat0.xxx * vs_INTERP9);
  let v_19 = (u_xlat0.xxx * vs_INTERP4.xyz);
  u_xlat2 = vec4<f32>(v_19.xyz, u_xlat2.w);
  let v_20 = (u_xlat26 * u_xlat0.xxx);
  u_xlat3 = vec4<f32>(v_20.xyz, u_xlat3.w);
  u_xlatb0 = (v_1.unity_OrthoParams.w == 0.0f);
  u_xlat4 = (-(vs_INTERP8) + v_1._WorldSpaceCameraPos);
  u_xlat79 = dot(u_xlat4, u_xlat4);
  u_xlat79 = inverseSqrt(u_xlat79);
  let v_21 = u_xlat79;
  u_xlat4 = (vec3<f32>(v_21, v_21, v_21) * u_xlat4);
  u_xlat5.x = v_1.unity_MatrixV[0i].z;
  u_xlat5.y = v_1.unity_MatrixV[1i].z;
  u_xlat5.z = v_1.unity_MatrixV[2i].z;
  if (u_xlatb0) {
    v_6 = u_xlat4;
  } else {
    v_6 = u_xlat5.xyz;
  }
  u_xlat4 = v_6;
  let v_22 = (u_xlat4.yyy * v.unity_WorldToObject[1i].xyz);
  u_xlat5 = vec4<f32>(v_22.xyz, u_xlat5.w);
  let v_23 = ((v.unity_WorldToObject[0i].xyz * u_xlat4.xxx) + u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_23.xyz, u_xlat5.w);
  let v_24 = ((v.unity_WorldToObject[2i].xyz * u_xlat4.zzz) + u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_24.xyz, u_xlat5.w);
  u_xlat0.x = dot(u_xlat5.xyz, u_xlat5.xyz);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  let v_25 = (u_xlat0.xxx * u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_25.xyz, u_xlat5.w);
  u_xlat6 = textureSampleBias(Texture2D_B222E8F, samplerTexture2D_B222E8F, vs_INTERP5.xy, v_1._GlobalMipBias.x);
  u_xlat7 = (u_xlat6.xyz * v_2.Color_C30C7CA3.xyz);
  u_xlat8 = textureSampleBias(Texture2D_D9BFD5F1, samplerTexture2D_D9BFD5F1, vs_INTERP5.xy, v_1._GlobalMipBias.x).wxyz;
  u_xlat9 = ((u_xlat8.yzwx * vec4<f32>(2.0f)) + vec4<f32>(-1.0f));
  u_xlat0.x = dot(u_xlat9, u_xlat9);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat34 = (u_xlat0.xxx * u_xlat9.xyz);
  u_xlat0.x = (vs_INTERP6.y * 200.0f);
  u_xlat0.x = min(u_xlat0.x, 1.0f);
  let v_26 = (u_xlat0.xxx * u_xlat6.xyz);
  u_xlat6 = vec4<f32>(v_26.xyz, u_xlat6.w);
  let v_27 = (u_xlat3.xyz * u_xlat34.yyy);
  u_xlat3 = vec4<f32>(v_27.xyz, u_xlat3.w);
  let v_28 = ((u_xlat34.xxx * u_xlat2.xyz) + u_xlat3.xyz);
  u_xlat2 = vec4<f32>(v_28.xyz, u_xlat2.w);
  u_xlat1 = ((u_xlat34.zzz * u_xlat1) + u_xlat2.xyz);
  u_xlat0.x = dot(u_xlat1, u_xlat1);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat1 = (u_xlat0.xxx * u_xlat1);
  let v_29 = (vs_INTERP8 + -(v_3._CascadeShadowSplitSpheres0.xyz));
  u_xlat2 = vec4<f32>(v_29.xyz, u_xlat2.w);
  let v_30 = (vs_INTERP8 + -(v_3._CascadeShadowSplitSpheres1.xyz));
  u_xlat3 = vec4<f32>(v_30.xyz, u_xlat3.w);
  let v_31 = (vs_INTERP8 + -(v_3._CascadeShadowSplitSpheres2.xyz));
  u_xlat9 = vec4<f32>(v_31.xyz, u_xlat9.w);
  let v_32 = (vs_INTERP8 + -(v_3._CascadeShadowSplitSpheres3.xyz));
  u_xlat10 = vec4<f32>(v_32.xyz, u_xlat10.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat2.y = dot(u_xlat3.xyz, u_xlat3.xyz);
  u_xlat2.z = dot(u_xlat9.xyz, u_xlat9.xyz);
  u_xlat2.w = dot(u_xlat10.xyz, u_xlat10.xyz);
  u_xlatb2 = (u_xlat2 < v_3._CascadeShadowSplitSphereRadii);
  u_xlat3.x = select(0.0f, 1.0f, u_xlatb2.x);
  u_xlat3.y = select(0.0f, 1.0f, u_xlatb2.y);
  u_xlat3.z = select(0.0f, 1.0f, u_xlatb2.z);
  u_xlat3.w = select(0.0f, 1.0f, u_xlatb2.w);
  u_xlat2.x = select(0.0f, -1.0f, u_xlatb2.x);
  u_xlat2.y = select(0.0f, -1.0f, u_xlatb2.y);
  u_xlat2.z = select(0.0f, -1.0f, u_xlatb2.z);
  let v_33 = (u_xlat2.xyz + u_xlat3.yzw);
  u_xlat2 = vec4<f32>(v_33.xyz, u_xlat2.w);
  let v_34 = max(u_xlat2.xyz, vec3<f32>());
  u_xlat3 = vec4<f32>(u_xlat3.x, v_34.xyz);
  u_xlat0.x = dot(u_xlat3, vec4<f32>(4.0f, 3.0f, 2.0f, 1.0f));
  u_xlat0.x = (-(u_xlat0.x) + 4.0f);
  u_xlatu0 = u32(u_xlat0.x);
  u_xlati0 = (bitcast<i32>(u_xlatu0) << bitcast<u32>(2i));
  let v_35 = (vs_INTERP8.yyy * v_3._MainLightWorldToShadow[((u_xlati0 + 1i) / 4i)][((u_xlati0 + 1i) % 4i)].xyz);
  u_xlat2 = vec4<f32>(v_35.xyz, u_xlat2.w);
  let v_36 = ((v_3._MainLightWorldToShadow[(u_xlati0 / 4i)][(u_xlati0 % 4i)].xyz * vs_INTERP8.xxx) + u_xlat2.xyz);
  u_xlat2 = vec4<f32>(v_36.xyz, u_xlat2.w);
  let v_37 = ((v_3._MainLightWorldToShadow[((u_xlati0 + 2i) / 4i)][((u_xlati0 + 2i) % 4i)].xyz * vs_INTERP8.zzz) + u_xlat2.xyz);
  u_xlat2 = vec4<f32>(v_37.xyz, u_xlat2.w);
  let v_38 = (u_xlat2.xyz + v_3._MainLightWorldToShadow[((u_xlati0 + 3i) / 4i)][((u_xlati0 + 3i) % 4i)].xyz);
  u_xlat2 = vec4<f32>(v_38.xyz, u_xlat2.w);
  u_xlatb0 = (0.0f < v_3._MainLightShadowParams.y);
  if (u_xlatb0) {
    u_xlatb79 = (v_3._MainLightShadowParams.y == 1.0f);
    if (u_xlatb79) {
      u_xlat3 = (u_xlat2.xyxy + v_3._MainLightShadowOffset0);
      let v_39 = u_xlat3.xy;
      txVec0 = vec3<f32>(v_39.x, v_39.y, u_xlat2.z);
      let v_40 = txVec0;
      u_xlat9.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_40.xy, v_40.z);
      let v_41 = u_xlat3.zw;
      txVec1 = vec3<f32>(v_41.x, v_41.y, u_xlat2.z);
      let v_42 = txVec1;
      u_xlat9.y = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_42.xy, v_42.z);
      u_xlat3 = (u_xlat2.xyxy + v_3._MainLightShadowOffset1);
      let v_43 = u_xlat3.xy;
      txVec2 = vec3<f32>(v_43.x, v_43.y, u_xlat2.z);
      let v_44 = txVec2;
      u_xlat9.z = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_44.xy, v_44.z);
      let v_45 = u_xlat3.zw;
      txVec3 = vec3<f32>(v_45.x, v_45.y, u_xlat2.z);
      let v_46 = txVec3;
      u_xlat9.w = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_46.xy, v_46.z);
      u_xlat79 = dot(u_xlat9, vec4<f32>(0.25f));
    } else {
      u_xlatb80 = (v_3._MainLightShadowParams.y == 2.0f);
      if (u_xlatb80) {
        let v_47 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + vec2<f32>(0.5f));
        u_xlat3 = vec4<f32>(v_47.xy, u_xlat3.zw);
        let v_48 = floor(u_xlat3.xy);
        u_xlat3 = vec4<f32>(v_48.xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_49 = (u_xlat10.yw * vec2<f32>(0.07999999821186065674f));
        let v_50 = u_xlat9;
        u_xlat9 = vec4<f32>(v_49.x, v_50.y, v_49.y, v_50.w);
        let v_51 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat55));
        u_xlat10 = vec4<f32>(v_51.xy, u_xlat10.zw);
        u_xlat62 = (-(u_xlat55) + vec2<f32>(1.0f));
        let v_52 = min(u_xlat55, vec2<f32>());
        u_xlat11 = vec4<f32>(v_52.xy, u_xlat11.zw);
        let v_53 = ((-(u_xlat11.xy) * u_xlat11.xy) + u_xlat62);
        u_xlat11 = vec4<f32>(v_53.xy, u_xlat11.zw);
        u_xlat55 = max(u_xlat55, vec2<f32>());
        u_xlat55 = ((-(u_xlat55) * u_xlat55) + u_xlat9.yw);
        let v_54 = (u_xlat11.xy + vec2<f32>(1.0f));
        u_xlat11 = vec4<f32>(v_54.xy, u_xlat11.zw);
        u_xlat55 = (u_xlat55 + vec2<f32>(1.0f));
        let v_55 = (u_xlat10.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat12 = vec4<f32>(v_55.xy, u_xlat12.zw);
        let v_56 = (u_xlat62 * vec2<f32>(0.15999999642372131348f));
        u_xlat10 = vec4<f32>(v_56.xy, u_xlat10.zw);
        let v_57 = (u_xlat11.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat11 = vec4<f32>(v_57.xy, u_xlat11.zw);
        let v_58 = (u_xlat55 * vec2<f32>(0.15999999642372131348f));
        u_xlat13 = vec4<f32>(v_58.xy, u_xlat13.zw);
        u_xlat55 = (u_xlat9.yw * vec2<f32>(0.15999999642372131348f));
        u_xlat12.z = u_xlat11.x;
        u_xlat12.w = u_xlat55.x;
        u_xlat10.z = u_xlat13.x;
        u_xlat10.w = u_xlat9.x;
        u_xlat14 = (u_xlat10.zwxz + u_xlat12.zwxz);
        u_xlat11.z = u_xlat12.y;
        u_xlat11.w = u_xlat55.y;
        u_xlat13.z = u_xlat10.y;
        u_xlat13.w = u_xlat9.z;
        let v_59 = (u_xlat11.zyw + u_xlat13.zyw);
        u_xlat9 = vec4<f32>(v_59.xyz, u_xlat9.w);
        let v_60 = (u_xlat10.xzw / u_xlat14.zwy);
        u_xlat10 = vec4<f32>(v_60.xyz, u_xlat10.w);
        let v_61 = (u_xlat10.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat10 = vec4<f32>(v_61.xyz, u_xlat10.w);
        let v_62 = (u_xlat13.zyw / u_xlat9.xyz);
        u_xlat11 = vec4<f32>(v_62.xyz, u_xlat11.w);
        let v_63 = (u_xlat11.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat11 = vec4<f32>(v_63.xyz, u_xlat11.w);
        let v_64 = (u_xlat10.yxz * v_3._MainLightShadowmapSize.xxx);
        u_xlat10 = vec4<f32>(v_64.xyz, u_xlat10.w);
        let v_65 = (u_xlat11.xyz * v_3._MainLightShadowmapSize.yyy);
        u_xlat11 = vec4<f32>(v_65.xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.ywxw);
        u_xlat55 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat10.zw);
        u_xlat11.w = u_xlat10.y;
        let v_66 = u_xlat11.yz;
        let v_67 = u_xlat10;
        u_xlat10 = vec4<f32>(v_67.x, v_66.x, v_67.z, v_66.y);
        u_xlat13 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat9.xxxy * u_xlat14.zwyz);
        u_xlat16 = (u_xlat9.yyzz * u_xlat14);
        u_xlat80 = (u_xlat9.z * u_xlat14.y);
        let v_68 = u_xlat12.xy;
        txVec4 = vec3<f32>(v_68.x, v_68.y, u_xlat2.z);
        let v_69 = txVec4;
        u_xlat3.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_69.xy, v_69.z);
        let v_70 = u_xlat12.zw;
        txVec5 = vec3<f32>(v_70.x, v_70.y, u_xlat2.z);
        let v_71 = txVec5;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_71.xy, v_71.z);
        u_xlat29 = (u_xlat29 * u_xlat15.y);
        u_xlat3.x = ((u_xlat15.x * u_xlat3.x) + u_xlat29);
        let v_72 = u_xlat55;
        txVec6 = vec3<f32>(v_72.x, v_72.y, u_xlat2.z);
        let v_73 = txVec6;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_73.xy, v_73.z);
        u_xlat3.x = ((u_xlat15.z * u_xlat29) + u_xlat3.x);
        let v_74 = u_xlat11.xy;
        txVec7 = vec3<f32>(v_74.x, v_74.y, u_xlat2.z);
        let v_75 = txVec7;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_75.xy, v_75.z);
        u_xlat3.x = ((u_xlat15.w * u_xlat29) + u_xlat3.x);
        let v_76 = u_xlat13.xy;
        txVec8 = vec3<f32>(v_76.x, v_76.y, u_xlat2.z);
        let v_77 = txVec8;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_77.xy, v_77.z);
        u_xlat3.x = ((u_xlat16.x * u_xlat29) + u_xlat3.x);
        let v_78 = u_xlat13.zw;
        txVec9 = vec3<f32>(v_78.x, v_78.y, u_xlat2.z);
        let v_79 = txVec9;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_79.xy, v_79.z);
        u_xlat3.x = ((u_xlat16.y * u_xlat29) + u_xlat3.x);
        let v_80 = u_xlat11.zw;
        txVec10 = vec3<f32>(v_80.x, v_80.y, u_xlat2.z);
        let v_81 = txVec10;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_81.xy, v_81.z);
        u_xlat3.x = ((u_xlat16.z * u_xlat29) + u_xlat3.x);
        let v_82 = u_xlat10.xy;
        txVec11 = vec3<f32>(v_82.x, v_82.y, u_xlat2.z);
        let v_83 = txVec11;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_83.xy, v_83.z);
        u_xlat3.x = ((u_xlat16.w * u_xlat29) + u_xlat3.x);
        let v_84 = u_xlat10.zw;
        txVec12 = vec3<f32>(v_84.x, v_84.y, u_xlat2.z);
        let v_85 = txVec12;
        u_xlat29 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_85.xy, v_85.z);
        u_xlat79 = ((u_xlat80 * u_xlat29) + u_xlat3.x);
      } else {
        let v_86 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + vec2<f32>(0.5f));
        u_xlat3 = vec4<f32>(v_86.xy, u_xlat3.zw);
        let v_87 = floor(u_xlat3.xy);
        u_xlat3 = vec4<f32>(v_87.xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_88 = (u_xlat10.yw * vec2<f32>(0.04081600159406661987f));
        let v_89 = u_xlat11;
        u_xlat11 = vec4<f32>(v_89.x, v_88.x, v_89.z, v_88.y);
        let v_90 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat55));
        let v_91 = u_xlat9;
        u_xlat9 = vec4<f32>(v_90.x, v_91.y, v_90.y, v_91.w);
        let v_92 = (-(u_xlat55) + vec2<f32>(1.0f));
        u_xlat10 = vec4<f32>(v_92.xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat55, vec2<f32>());
        let v_93 = ((-(u_xlat62) * u_xlat62) + u_xlat10.xy);
        u_xlat10 = vec4<f32>(v_93.xy, u_xlat10.zw);
        u_xlat62 = max(u_xlat55, vec2<f32>());
        let v_94 = ((-(u_xlat62) * u_xlat62) + u_xlat9.yw);
        u_xlat35 = vec3<f32>(v_94.x, u_xlat35.y, v_94.y);
        let v_95 = (u_xlat10.xy + vec2<f32>(2.0f));
        u_xlat10 = vec4<f32>(v_95.xy, u_xlat10.zw);
        let v_96 = (u_xlat35.xz + vec2<f32>(2.0f));
        let v_97 = u_xlat9;
        u_xlat9 = vec4<f32>(v_97.x, v_96.x, v_97.z, v_96.y);
        u_xlat12.z = (u_xlat9.y * 0.08163200318813323975f);
        let v_98 = (u_xlat9.zxw * vec3<f32>(0.08163200318813323975f));
        u_xlat13 = vec4<f32>(v_98.xyz, u_xlat13.w);
        let v_99 = (u_xlat10.xy * vec2<f32>(0.08163200318813323975f));
        u_xlat9 = vec4<f32>(v_99.xy, u_xlat9.zw);
        u_xlat12.x = u_xlat13.y;
        let v_100 = ((u_xlat55.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_101 = u_xlat12;
        u_xlat12 = vec4<f32>(v_101.x, v_100.x, v_101.z, v_100.y);
        let v_102 = ((u_xlat55.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_103 = u_xlat10;
        u_xlat10 = vec4<f32>(v_102.x, v_103.y, v_102.y, v_103.w);
        u_xlat10.y = u_xlat9.x;
        u_xlat10.w = u_xlat11.y;
        u_xlat12 = (u_xlat10 + u_xlat12);
        let v_104 = ((u_xlat55.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_105 = u_xlat13;
        u_xlat13 = vec4<f32>(v_105.x, v_104.x, v_105.z, v_104.y);
        let v_106 = ((u_xlat55.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_107 = u_xlat11;
        u_xlat11 = vec4<f32>(v_106.x, v_107.y, v_106.y, v_107.w);
        u_xlat11.y = u_xlat9.y;
        u_xlat9 = (u_xlat11 + u_xlat13);
        u_xlat10 = (u_xlat10 / u_xlat12);
        u_xlat10 = (u_xlat10 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat11 = (u_xlat11 / u_xlat9);
        u_xlat11 = (u_xlat11 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat10 = (u_xlat10.wxyz * v_3._MainLightShadowmapSize.xxxx);
        u_xlat11 = (u_xlat11.xwyz * v_3._MainLightShadowmapSize.yyyy);
        let v_108 = u_xlat10.yzw;
        u_xlat13 = vec4<f32>(v_108.x, u_xlat13.y, v_108.yz);
        u_xlat13.y = u_xlat11.x;
        u_xlat14 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        u_xlat55 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat13.wy);
        u_xlat10.y = u_xlat13.y;
        u_xlat13.y = u_xlat11.z;
        u_xlat15 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        let v_109 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat13.wy);
        u_xlat16 = vec4<f32>(v_109.xy, u_xlat16.zw);
        u_xlat10.z = u_xlat13.y;
        u_xlat17 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.xyxz);
        u_xlat13.y = u_xlat11.w;
        u_xlat18 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat13.xyzy);
        u_xlat36 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat13.wy);
        u_xlat10.w = u_xlat13.y;
        u_xlat68 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat10.xw);
        let v_110 = u_xlat13.xzw;
        u_xlat11 = vec4<f32>(v_110.x, u_xlat11.y, v_110.yz);
        u_xlat13 = ((u_xlat3.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat10.x;
        let v_111 = ((u_xlat3.xy * v_3._MainLightShadowmapSize.xy) + u_xlat11.xy);
        u_xlat3 = vec4<f32>(v_111.xy, u_xlat3.zw);
        u_xlat19 = (u_xlat9.xxxx * u_xlat12);
        u_xlat20 = (u_xlat9.yyyy * u_xlat12);
        u_xlat21 = (u_xlat9.zzzz * u_xlat12);
        u_xlat9 = (u_xlat9.wwww * u_xlat12);
        let v_112 = u_xlat14.xy;
        txVec13 = vec3<f32>(v_112.x, v_112.y, u_xlat2.z);
        let v_113 = txVec13;
        u_xlat80 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_113.xy, v_113.z);
        let v_114 = u_xlat14.zw;
        txVec14 = vec3<f32>(v_114.x, v_114.y, u_xlat2.z);
        let v_115 = txVec14;
        u_xlat82 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_115.xy, v_115.z);
        u_xlat82 = (u_xlat82 * u_xlat19.y);
        u_xlat80 = ((u_xlat19.x * u_xlat80) + u_xlat82);
        let v_116 = u_xlat55;
        txVec15 = vec3<f32>(v_116.x, v_116.y, u_xlat2.z);
        let v_117 = txVec15;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_117.xy, v_117.z);
        u_xlat80 = ((u_xlat19.z * u_xlat55.x) + u_xlat80);
        let v_118 = u_xlat17.xy;
        txVec16 = vec3<f32>(v_118.x, v_118.y, u_xlat2.z);
        let v_119 = txVec16;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_119.xy, v_119.z);
        u_xlat80 = ((u_xlat19.w * u_xlat55.x) + u_xlat80);
        let v_120 = u_xlat15.xy;
        txVec17 = vec3<f32>(v_120.x, v_120.y, u_xlat2.z);
        let v_121 = txVec17;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_121.xy, v_121.z);
        u_xlat80 = ((u_xlat20.x * u_xlat55.x) + u_xlat80);
        let v_122 = u_xlat15.zw;
        txVec18 = vec3<f32>(v_122.x, v_122.y, u_xlat2.z);
        let v_123 = txVec18;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_123.xy, v_123.z);
        u_xlat80 = ((u_xlat20.y * u_xlat55.x) + u_xlat80);
        let v_124 = u_xlat16.xy;
        txVec19 = vec3<f32>(v_124.x, v_124.y, u_xlat2.z);
        let v_125 = txVec19;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_125.xy, v_125.z);
        u_xlat80 = ((u_xlat20.z * u_xlat55.x) + u_xlat80);
        let v_126 = u_xlat17.zw;
        txVec20 = vec3<f32>(v_126.x, v_126.y, u_xlat2.z);
        let v_127 = txVec20;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_127.xy, v_127.z);
        u_xlat80 = ((u_xlat20.w * u_xlat55.x) + u_xlat80);
        let v_128 = u_xlat18.xy;
        txVec21 = vec3<f32>(v_128.x, v_128.y, u_xlat2.z);
        let v_129 = txVec21;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_129.xy, v_129.z);
        u_xlat80 = ((u_xlat21.x * u_xlat55.x) + u_xlat80);
        let v_130 = u_xlat18.zw;
        txVec22 = vec3<f32>(v_130.x, v_130.y, u_xlat2.z);
        let v_131 = txVec22;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_131.xy, v_131.z);
        u_xlat80 = ((u_xlat21.y * u_xlat55.x) + u_xlat80);
        let v_132 = u_xlat36;
        txVec23 = vec3<f32>(v_132.x, v_132.y, u_xlat2.z);
        let v_133 = txVec23;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_133.xy, v_133.z);
        u_xlat80 = ((u_xlat21.z * u_xlat55.x) + u_xlat80);
        let v_134 = u_xlat68;
        txVec24 = vec3<f32>(v_134.x, v_134.y, u_xlat2.z);
        let v_135 = txVec24;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_135.xy, v_135.z);
        u_xlat80 = ((u_xlat21.w * u_xlat55.x) + u_xlat80);
        let v_136 = u_xlat13.xy;
        txVec25 = vec3<f32>(v_136.x, v_136.y, u_xlat2.z);
        let v_137 = txVec25;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_137.xy, v_137.z);
        u_xlat80 = ((u_xlat9.x * u_xlat55.x) + u_xlat80);
        let v_138 = u_xlat13.zw;
        txVec26 = vec3<f32>(v_138.x, v_138.y, u_xlat2.z);
        let v_139 = txVec26;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_139.xy, v_139.z);
        u_xlat80 = ((u_xlat9.y * u_xlat55.x) + u_xlat80);
        let v_140 = u_xlat63;
        txVec27 = vec3<f32>(v_140.x, v_140.y, u_xlat2.z);
        let v_141 = txVec27;
        u_xlat55.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_141.xy, v_141.z);
        u_xlat80 = ((u_xlat9.z * u_xlat55.x) + u_xlat80);
        let v_142 = u_xlat3.xy;
        txVec28 = vec3<f32>(v_142.x, v_142.y, u_xlat2.z);
        let v_143 = txVec28;
        u_xlat3.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_143.xy, v_143.z);
        u_xlat79 = ((u_xlat9.w * u_xlat3.x) + u_xlat80);
      }
    }
  } else {
    let v_144 = u_xlat2.xy;
    txVec29 = vec3<f32>(v_144.x, v_144.y, u_xlat2.z);
    let v_145 = txVec29;
    u_xlat79 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_145.xy, v_145.z);
  }
  u_xlat80 = (-(v_3._MainLightShadowParams.x) + 1.0f);
  u_xlat79 = ((u_xlat79 * v_3._MainLightShadowParams.x) + u_xlat80);
  u_xlatb3.x = (0.0f >= u_xlat2.z);
  u_xlatb29 = (u_xlat2.z >= 1.0f);
  u_xlatb3.x = (u_xlatb29 | u_xlatb3.x);
  let v_146 = u_xlatb3.x;
  u_xlat79 = select(u_xlat79, 1.0f, v_146);
  u_xlat1.x = dot(u_xlat1, -(v_1._MainLightPosition.xyz));
  u_xlat1.x = clamp(u_xlat1.x, 0.0f, 1.0f);
  let v_147 = u_xlat79;
  u_xlat27 = (vec3<f32>(v_147, v_147, v_147) * v_1._MainLightColor.xyz);
  u_xlat1 = (u_xlat27 * u_xlat1.xxx);
  u_xlat1 = (u_xlat1 * u_xlat6.xyz);
  u_xlatb79 = (v.unity_LODFade.x < 0.0f);
  u_xlat29 = (v.unity_LODFade.x + 1.0f);
  if (u_xlatb79) {
    v_7 = u_xlat29;
  } else {
    v_7 = v.unity_LODFade.x;
  }
  u_xlat79 = v_7;
  u_xlatb29 = (0.5f >= u_xlat79);
  let v_148 = (abs(u_xlat5.xyz) * v_1._ScreenParams.xyx);
  u_xlat5 = vec4<f32>(v_148.xyz, u_xlat5.w);
  u_xlatu5 = vec3<u32>(u_xlat5.xyz);
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
  u_xlat55.x = bitcast<f32>(v_149(&(param), &(param_1), &(param_2), &(param_3)));
  u_xlat55.x = (u_xlat55.x + -1.0f);
  u_xlat81 = (-(u_xlat55.x) + 1.0f);
  if (u_xlatb29) {
    v_8 = u_xlat55.x;
  } else {
    v_8 = u_xlat81;
  }
  u_xlat29 = v_8;
  u_xlat79 = ((u_xlat79 * 2.0f) + -(u_xlat29));
  u_xlat29 = (u_xlat79 * u_xlat6.w);
  u_xlatb55 = (u_xlat29 >= 0.40000000596046447754f);
  let v_150 = u_xlatb55;
  u_xlat55.x = select(0.0f, u_xlat29, v_150);
  u_xlat79 = ((u_xlat6.w * u_xlat79) + -0.40000000596046447754f);
  u_xlat81 = dpdxCoarse(u_xlat29);
  u_xlat29 = dpdyCoarse(u_xlat29);
  u_xlat29 = (abs(u_xlat29) + abs(u_xlat81));
  u_xlat29 = max(u_xlat29, 0.00009999999747378752f);
  u_xlat79 = (u_xlat79 / u_xlat29);
  u_xlat79 = (u_xlat79 + 0.5f);
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  u_xlatb29 = !((v_1._AlphaToMaskAvailable == 0.0f));
  if (u_xlatb29) {
    v_9 = u_xlat79;
  } else {
    v_9 = u_xlat55.x;
  }
  u_xlat79 = v_9;
  u_xlat55.x = (u_xlat79 + -0.00009999999747378752f);
  u_xlatb55 = (u_xlat55.x < 0.0f);
  if (((select(0i, 1i, u_xlatb55) * -1i) != 0i)) {
    discard;
    return;
  }
  u_xlat26 = (u_xlat26 * u_xlat34.yyy);
  u_xlat26 = ((u_xlat34.xxx * vs_INTERP4.xyz) + u_xlat26);
  u_xlat26 = ((u_xlat34.zzz * vs_INTERP9) + u_xlat26);
  u_xlat55.x = dot(u_xlat26, u_xlat26);
  u_xlat55.x = inverseSqrt(u_xlat55.x);
  u_xlat26 = (u_xlat26 * u_xlat55.xxx);
  u_xlat55.x = (vs_INTERP8.y * v_1.unity_MatrixV[1i].z);
  u_xlat55.x = ((v_1.unity_MatrixV[0i].z * vs_INTERP8.x) + u_xlat55.x);
  u_xlat55.x = ((v_1.unity_MatrixV[2i].z * vs_INTERP8.z) + u_xlat55.x);
  u_xlat55.x = (u_xlat55.x + v_1.unity_MatrixV[3i].z);
  u_xlat55.x = (-(u_xlat55.x) + -(v_1._ProjectionParams.y));
  u_xlat55.x = max(u_xlat55.x, 0.0f);
  u_xlat55.x = (u_xlat55.x * v_1.unity_FogParams.x);
  u_xlat5 = textureSampleBias(unity_LightmapInd, samplerunity_Lightmap, vs_INTERP0, v_1._GlobalMipBias.x);
  let v_151 = textureSampleBias(unity_Lightmap, samplerunity_Lightmap, vs_INTERP0, v_1._GlobalMipBias.x).xyz;
  u_xlat6 = vec4<f32>(v_151.xyz, u_xlat6.w);
  let v_152 = (u_xlat5.xyz + vec3<f32>(-0.5f));
  u_xlat5 = vec4<f32>(v_152.xyz, u_xlat5.w);
  u_xlat81 = dot(u_xlat26, u_xlat5.xyz);
  u_xlat81 = (u_xlat81 + 0.5f);
  let v_153 = u_xlat81;
  let v_154 = (vec3<f32>(v_153, v_153, v_153) * u_xlat6.xyz);
  u_xlat5 = vec4<f32>(v_154.xyz, u_xlat5.w);
  u_xlat81 = max(u_xlat5.w, 0.00009999999747378752f);
  let v_155 = u_xlat5.xyz;
  let v_156 = u_xlat81;
  u_xlat5 = vec4<f32>(((v_155 / vec3<f32>(v_156, v_156, v_156))).xyz, u_xlat5.w);
  u_xlat8.x = u_xlat8.x;
  u_xlat8.x = clamp(u_xlat8.x, 0.0f, 1.0f);
  u_xlat79 = u_xlat79;
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  let v_157 = (u_xlat7 * vec3<f32>(0.95999997854232788086f));
  u_xlat6 = vec4<f32>(v_157.xyz, u_xlat6.w);
  u_xlat81 = (-(u_xlat8.x) + 1.0f);
  u_xlat82 = (u_xlat81 * u_xlat81);
  u_xlat82 = max(u_xlat82, 0.0078125f);
  u_xlat83 = (u_xlat82 * u_xlat82);
  u_xlat84 = (u_xlat8.x + 0.04000002145767211914f);
  u_xlat84 = min(u_xlat84, 1.0f);
  u_xlat7.x = ((u_xlat82 * 4.0f) + 2.0f);
  u_xlat33 = min(vs_INTERP6.w, 1.0f);
  if (u_xlatb0) {
    u_xlatb0 = (v_3._MainLightShadowParams.y == 1.0f);
    if (u_xlatb0) {
      u_xlat8 = (u_xlat2.xyxy + v_3._MainLightShadowOffset0);
      let v_158 = u_xlat8.xy;
      txVec30 = vec3<f32>(v_158.x, v_158.y, u_xlat2.z);
      let v_159 = txVec30;
      u_xlat9.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_159.xy, v_159.z);
      let v_160 = u_xlat8.zw;
      txVec31 = vec3<f32>(v_160.x, v_160.y, u_xlat2.z);
      let v_161 = txVec31;
      u_xlat9.y = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_161.xy, v_161.z);
      u_xlat8 = (u_xlat2.xyxy + v_3._MainLightShadowOffset1);
      let v_162 = u_xlat8.xy;
      txVec32 = vec3<f32>(v_162.x, v_162.y, u_xlat2.z);
      let v_163 = txVec32;
      u_xlat9.z = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_163.xy, v_163.z);
      let v_164 = u_xlat8.zw;
      txVec33 = vec3<f32>(v_164.x, v_164.y, u_xlat2.z);
      let v_165 = txVec33;
      u_xlat9.w = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_165.xy, v_165.z);
      u_xlat0.x = dot(u_xlat9, vec4<f32>(0.25f));
    } else {
      u_xlatb59 = (v_3._MainLightShadowParams.y == 2.0f);
      if (u_xlatb59) {
        u_xlat59 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + vec2<f32>(0.5f));
        u_xlat59 = floor(u_xlat59);
        let v_166 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + -(u_xlat59));
        u_xlat8 = vec4<f32>(v_166.xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        u_xlat60 = (u_xlat10.yw * vec2<f32>(0.07999999821186065674f));
        let v_167 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat8.xy));
        let v_168 = u_xlat9;
        u_xlat9 = vec4<f32>(v_167.x, v_168.y, v_167.y, v_168.w);
        let v_169 = (-(u_xlat8.xy) + vec2<f32>(1.0f));
        u_xlat10 = vec4<f32>(v_169.xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat8.xy, vec2<f32>());
        u_xlat62 = ((-(u_xlat62) * u_xlat62) + u_xlat10.xy);
        let v_170 = max(u_xlat8.xy, vec2<f32>());
        u_xlat8 = vec4<f32>(v_170.xy, u_xlat8.zw);
        let v_171 = ((-(u_xlat8.xy) * u_xlat8.xy) + u_xlat9.yw);
        u_xlat8 = vec4<f32>(v_171.xy, u_xlat8.zw);
        u_xlat62 = (u_xlat62 + vec2<f32>(1.0f));
        let v_172 = (u_xlat8.xy + vec2<f32>(1.0f));
        u_xlat8 = vec4<f32>(v_172.xy, u_xlat8.zw);
        let v_173 = (u_xlat9.xz * vec2<f32>(0.15999999642372131348f));
        u_xlat11 = vec4<f32>(v_173.xy, u_xlat11.zw);
        let v_174 = (u_xlat10.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat12 = vec4<f32>(v_174.xy, u_xlat12.zw);
        let v_175 = (u_xlat62 * vec2<f32>(0.15999999642372131348f));
        u_xlat10 = vec4<f32>(v_175.xy, u_xlat10.zw);
        let v_176 = (u_xlat8.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat13 = vec4<f32>(v_176.xy, u_xlat13.zw);
        let v_177 = (u_xlat9.yw * vec2<f32>(0.15999999642372131348f));
        u_xlat8 = vec4<f32>(v_177.xy, u_xlat8.zw);
        u_xlat11.z = u_xlat10.x;
        u_xlat11.w = u_xlat8.x;
        u_xlat12.z = u_xlat13.x;
        u_xlat12.w = u_xlat60.x;
        u_xlat9 = (u_xlat11.zwxz + u_xlat12.zwxz);
        u_xlat10.z = u_xlat11.y;
        u_xlat10.w = u_xlat8.y;
        u_xlat13.z = u_xlat12.y;
        u_xlat13.w = u_xlat60.y;
        let v_178 = (u_xlat10.zyw + u_xlat13.zyw);
        u_xlat8 = vec4<f32>(v_178.xyz, u_xlat8.w);
        let v_179 = (u_xlat12.xzw / u_xlat9.zwy);
        u_xlat10 = vec4<f32>(v_179.xyz, u_xlat10.w);
        let v_180 = (u_xlat10.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat10 = vec4<f32>(v_180.xyz, u_xlat10.w);
        let v_181 = (u_xlat13.zyw / u_xlat8.xyz);
        u_xlat11 = vec4<f32>(v_181.xyz, u_xlat11.w);
        let v_182 = (u_xlat11.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat11 = vec4<f32>(v_182.xyz, u_xlat11.w);
        let v_183 = (u_xlat10.yxz * v_3._MainLightShadowmapSize.xxx);
        u_xlat10 = vec4<f32>(v_183.xyz, u_xlat10.w);
        let v_184 = (u_xlat11.xyz * v_3._MainLightShadowmapSize.yyy);
        u_xlat11 = vec4<f32>(v_184.xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.ywxw);
        let v_185 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat10.zw);
        u_xlat13 = vec4<f32>(v_185.xy, u_xlat13.zw);
        u_xlat11.w = u_xlat10.y;
        let v_186 = u_xlat11.yz;
        let v_187 = u_xlat10;
        u_xlat10 = vec4<f32>(v_187.x, v_186.x, v_187.z, v_186.y);
        u_xlat14 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat8.xxxy * u_xlat9.zwyz);
        u_xlat16 = (u_xlat8.yyzz * u_xlat9);
        u_xlat59.x = (u_xlat8.z * u_xlat9.y);
        let v_188 = u_xlat12.xy;
        txVec34 = vec3<f32>(v_188.x, v_188.y, u_xlat2.z);
        let v_189 = txVec34;
        u_xlat85 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_189.xy, v_189.z);
        let v_190 = u_xlat12.zw;
        txVec35 = vec3<f32>(v_190.x, v_190.y, u_xlat2.z);
        let v_191 = txVec35;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_191.xy, v_191.z);
        u_xlat8.x = (u_xlat8.x * u_xlat15.y);
        u_xlat85 = ((u_xlat15.x * u_xlat85) + u_xlat8.x);
        let v_192 = u_xlat13.xy;
        txVec36 = vec3<f32>(v_192.x, v_192.y, u_xlat2.z);
        let v_193 = txVec36;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_193.xy, v_193.z);
        u_xlat85 = ((u_xlat15.z * u_xlat8.x) + u_xlat85);
        let v_194 = u_xlat11.xy;
        txVec37 = vec3<f32>(v_194.x, v_194.y, u_xlat2.z);
        let v_195 = txVec37;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_195.xy, v_195.z);
        u_xlat85 = ((u_xlat15.w * u_xlat8.x) + u_xlat85);
        let v_196 = u_xlat14.xy;
        txVec38 = vec3<f32>(v_196.x, v_196.y, u_xlat2.z);
        let v_197 = txVec38;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_197.xy, v_197.z);
        u_xlat85 = ((u_xlat16.x * u_xlat8.x) + u_xlat85);
        let v_198 = u_xlat14.zw;
        txVec39 = vec3<f32>(v_198.x, v_198.y, u_xlat2.z);
        let v_199 = txVec39;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_199.xy, v_199.z);
        u_xlat85 = ((u_xlat16.y * u_xlat8.x) + u_xlat85);
        let v_200 = u_xlat11.zw;
        txVec40 = vec3<f32>(v_200.x, v_200.y, u_xlat2.z);
        let v_201 = txVec40;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_201.xy, v_201.z);
        u_xlat85 = ((u_xlat16.z * u_xlat8.x) + u_xlat85);
        let v_202 = u_xlat10.xy;
        txVec41 = vec3<f32>(v_202.x, v_202.y, u_xlat2.z);
        let v_203 = txVec41;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_203.xy, v_203.z);
        u_xlat85 = ((u_xlat16.w * u_xlat8.x) + u_xlat85);
        let v_204 = u_xlat10.zw;
        txVec42 = vec3<f32>(v_204.x, v_204.y, u_xlat2.z);
        let v_205 = txVec42;
        u_xlat8.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_205.xy, v_205.z);
        u_xlat0.x = ((u_xlat59.x * u_xlat8.x) + u_xlat85);
      } else {
        u_xlat59 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + vec2<f32>(0.5f));
        u_xlat59 = floor(u_xlat59);
        let v_206 = ((u_xlat2.xy * v_3._MainLightShadowmapSize.zw) + -(u_xlat59));
        u_xlat8 = vec4<f32>(v_206.xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_207 = (u_xlat10.yw * vec2<f32>(0.04081600159406661987f));
        let v_208 = u_xlat11;
        u_xlat11 = vec4<f32>(v_208.x, v_207.x, v_208.z, v_207.y);
        u_xlat60 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat8.xy));
        let v_209 = (-(u_xlat8.xy) + vec2<f32>(1.0f));
        let v_210 = u_xlat9;
        u_xlat9 = vec4<f32>(v_209.x, v_210.y, v_209.y, v_210.w);
        let v_211 = min(u_xlat8.xy, vec2<f32>());
        u_xlat10 = vec4<f32>(v_211.xy, u_xlat10.zw);
        let v_212 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.xz);
        let v_213 = u_xlat9;
        u_xlat9 = vec4<f32>(v_212.x, v_213.y, v_212.y, v_213.w);
        let v_214 = max(u_xlat8.xy, vec2<f32>());
        u_xlat10 = vec4<f32>(v_214.xy, u_xlat10.zw);
        let v_215 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.yw);
        let v_216 = u_xlat9;
        u_xlat9 = vec4<f32>(v_216.x, v_215.x, v_216.z, v_215.y);
        u_xlat9 = (u_xlat9 + vec4<f32>(2.0f));
        u_xlat10.z = (u_xlat9.y * 0.08163200318813323975f);
        let v_217 = (u_xlat60.yx * vec2<f32>(0.08163200318813323975f));
        u_xlat12 = vec4<f32>(v_217.xy, u_xlat12.zw);
        u_xlat60 = (u_xlat9.xz * vec2<f32>(0.08163200318813323975f));
        u_xlat12.z = (u_xlat9.w * 0.08163200318813323975f);
        u_xlat10.x = u_xlat12.y;
        let v_218 = ((u_xlat8.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_219 = u_xlat10;
        u_xlat10 = vec4<f32>(v_219.x, v_218.x, v_219.z, v_218.y);
        let v_220 = ((u_xlat8.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_221 = u_xlat9;
        u_xlat9 = vec4<f32>(v_220.x, v_221.y, v_220.y, v_221.w);
        u_xlat9.y = u_xlat60.x;
        u_xlat9.w = u_xlat11.y;
        u_xlat10 = (u_xlat9 + u_xlat10);
        let v_222 = ((u_xlat8.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_223 = u_xlat12;
        u_xlat12 = vec4<f32>(v_223.x, v_222.x, v_223.z, v_222.y);
        let v_224 = ((u_xlat8.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_225 = u_xlat11;
        u_xlat11 = vec4<f32>(v_224.x, v_225.y, v_224.y, v_225.w);
        u_xlat11.y = u_xlat60.y;
        u_xlat8 = (u_xlat11 + u_xlat12);
        u_xlat9 = (u_xlat9 / u_xlat10);
        u_xlat9 = (u_xlat9 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat11 = (u_xlat11 / u_xlat8);
        u_xlat11 = (u_xlat11 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat9 = (u_xlat9.wxyz * v_3._MainLightShadowmapSize.xxxx);
        u_xlat11 = (u_xlat11.xwyz * v_3._MainLightShadowmapSize.yyyy);
        let v_226 = u_xlat9.yzw;
        u_xlat12 = vec4<f32>(v_226.x, u_xlat12.y, v_226.yz);
        u_xlat12.y = u_xlat11.x;
        u_xlat13 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        let v_227 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat12.wy);
        u_xlat14 = vec4<f32>(v_227.xy, u_xlat14.zw);
        u_xlat9.y = u_xlat12.y;
        u_xlat12.y = u_xlat11.z;
        u_xlat15 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        u_xlat66 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat12.wy);
        u_xlat9.z = u_xlat12.y;
        u_xlat16 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat9.xyxz);
        u_xlat12.y = u_xlat11.w;
        u_xlat17 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat12.xyzy);
        let v_228 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat12.wy);
        u_xlat35 = vec3<f32>(v_228.xy, u_xlat35.z);
        u_xlat9.w = u_xlat12.y;
        let v_229 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat9.xw);
        u_xlat18 = vec4<f32>(v_229.xy, u_xlat18.zw);
        let v_230 = u_xlat12.xzw;
        u_xlat11 = vec4<f32>(v_230.x, u_xlat11.y, v_230.yz);
        u_xlat12 = ((u_xlat59.xyxy * v_3._MainLightShadowmapSize.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat9.x;
        u_xlat59 = ((u_xlat59 * v_3._MainLightShadowmapSize.xy) + u_xlat11.xy);
        u_xlat19 = (u_xlat8.xxxx * u_xlat10);
        u_xlat20 = (u_xlat8.yyyy * u_xlat10);
        u_xlat21 = (u_xlat8.zzzz * u_xlat10);
        u_xlat8 = (u_xlat8.wwww * u_xlat10);
        let v_231 = u_xlat13.xy;
        txVec43 = vec3<f32>(v_231.x, v_231.y, u_xlat2.z);
        let v_232 = txVec43;
        u_xlat9.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_232.xy, v_232.z);
        let v_233 = u_xlat13.zw;
        txVec44 = vec3<f32>(v_233.x, v_233.y, u_xlat2.z);
        let v_234 = txVec44;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_234.xy, v_234.z);
        u_xlat87 = (u_xlat87 * u_xlat19.y);
        u_xlat9.x = ((u_xlat19.x * u_xlat9.x) + u_xlat87);
        let v_235 = u_xlat14.xy;
        txVec45 = vec3<f32>(v_235.x, v_235.y, u_xlat2.z);
        let v_236 = txVec45;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_236.xy, v_236.z);
        u_xlat9.x = ((u_xlat19.z * u_xlat87) + u_xlat9.x);
        let v_237 = u_xlat16.xy;
        txVec46 = vec3<f32>(v_237.x, v_237.y, u_xlat2.z);
        let v_238 = txVec46;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_238.xy, v_238.z);
        u_xlat9.x = ((u_xlat19.w * u_xlat87) + u_xlat9.x);
        let v_239 = u_xlat15.xy;
        txVec47 = vec3<f32>(v_239.x, v_239.y, u_xlat2.z);
        let v_240 = txVec47;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_240.xy, v_240.z);
        u_xlat9.x = ((u_xlat20.x * u_xlat87) + u_xlat9.x);
        let v_241 = u_xlat15.zw;
        txVec48 = vec3<f32>(v_241.x, v_241.y, u_xlat2.z);
        let v_242 = txVec48;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_242.xy, v_242.z);
        u_xlat9.x = ((u_xlat20.y * u_xlat87) + u_xlat9.x);
        let v_243 = u_xlat66;
        txVec49 = vec3<f32>(v_243.x, v_243.y, u_xlat2.z);
        let v_244 = txVec49;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_244.xy, v_244.z);
        u_xlat9.x = ((u_xlat20.z * u_xlat87) + u_xlat9.x);
        let v_245 = u_xlat16.zw;
        txVec50 = vec3<f32>(v_245.x, v_245.y, u_xlat2.z);
        let v_246 = txVec50;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_246.xy, v_246.z);
        u_xlat9.x = ((u_xlat20.w * u_xlat87) + u_xlat9.x);
        let v_247 = u_xlat17.xy;
        txVec51 = vec3<f32>(v_247.x, v_247.y, u_xlat2.z);
        let v_248 = txVec51;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_248.xy, v_248.z);
        u_xlat9.x = ((u_xlat21.x * u_xlat87) + u_xlat9.x);
        let v_249 = u_xlat17.zw;
        txVec52 = vec3<f32>(v_249.x, v_249.y, u_xlat2.z);
        let v_250 = txVec52;
        u_xlat87 = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_250.xy, v_250.z);
        u_xlat9.x = ((u_xlat21.y * u_xlat87) + u_xlat9.x);
        let v_251 = u_xlat35.xy;
        txVec53 = vec3<f32>(v_251.x, v_251.y, u_xlat2.z);
        let v_252 = txVec53;
        u_xlat35.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_252.xy, v_252.z);
        u_xlat9.x = ((u_xlat21.z * u_xlat35.x) + u_xlat9.x);
        let v_253 = u_xlat18.xy;
        txVec54 = vec3<f32>(v_253.x, v_253.y, u_xlat2.z);
        let v_254 = txVec54;
        u_xlat35.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_254.xy, v_254.z);
        u_xlat9.x = ((u_xlat21.w * u_xlat35.x) + u_xlat9.x);
        let v_255 = u_xlat12.xy;
        txVec55 = vec3<f32>(v_255.x, v_255.y, u_xlat2.z);
        let v_256 = txVec55;
        u_xlat35.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_256.xy, v_256.z);
        u_xlat8.x = ((u_xlat8.x * u_xlat35.x) + u_xlat9.x);
        let v_257 = u_xlat12.zw;
        txVec56 = vec3<f32>(v_257.x, v_257.y, u_xlat2.z);
        let v_258 = txVec56;
        u_xlat9.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_258.xy, v_258.z);
        u_xlat8.x = ((u_xlat8.y * u_xlat9.x) + u_xlat8.x);
        let v_259 = u_xlat63;
        txVec57 = vec3<f32>(v_259.x, v_259.y, u_xlat2.z);
        let v_260 = txVec57;
        u_xlat34.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_260.xy, v_260.z);
        u_xlat8.x = ((u_xlat8.z * u_xlat34.x) + u_xlat8.x);
        let v_261 = u_xlat59;
        txVec58 = vec3<f32>(v_261.x, v_261.y, u_xlat2.z);
        let v_262 = txVec58;
        u_xlat59.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_262.xy, v_262.z);
        u_xlat0.x = ((u_xlat8.w * u_xlat59.x) + u_xlat8.x);
      }
    }
  } else {
    let v_263 = u_xlat2.xy;
    txVec59 = vec3<f32>(v_263.x, v_263.y, u_xlat2.z);
    let v_264 = txVec59;
    u_xlat0.x = textureSampleCompareLevel(_MainLightShadowmapTexture, sampler_LinearClampCompare, v_264.xy, v_264.z);
  }
  u_xlat0.x = ((u_xlat0.x * v_3._MainLightShadowParams.x) + u_xlat80);
  if (u_xlatb3.x) {
    v_10 = 1.0f;
  } else {
    v_10 = u_xlat0.x;
  }
  u_xlat0.x = v_10;
  let v_265 = (vs_INTERP8 + -(v_1._WorldSpaceCameraPos));
  u_xlat2 = vec4<f32>(v_265.xyz, u_xlat2.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat28.x = ((u_xlat2.x * v_3._MainLightShadowParams.z) + v_3._MainLightShadowParams.w);
  u_xlat28.x = clamp(u_xlat28.x, 0.0f, 1.0f);
  u_xlat54 = (-(u_xlat0.x) + 1.0f);
  u_xlat0.x = ((u_xlat28.x * u_xlat54) + u_xlat0.x);
  u_xlatb28.x = !((v_4._MainLightCookieTextureFormat == -1.0f));
  if (u_xlatb28.x) {
    let v_266 = (vs_INTERP8.yy * v_4._MainLightWorldToLight[1i].xy);
    u_xlat28 = vec3<f32>(v_266.xy, u_xlat28.z);
    let v_267 = ((v_4._MainLightWorldToLight[0i].xy * vs_INTERP8.xx) + u_xlat28.xy);
    u_xlat28 = vec3<f32>(v_267.xy, u_xlat28.z);
    let v_268 = ((v_4._MainLightWorldToLight[2i].xy * vs_INTERP8.zz) + u_xlat28.xy);
    u_xlat28 = vec3<f32>(v_268.xy, u_xlat28.z);
    let v_269 = (u_xlat28.xy + v_4._MainLightWorldToLight[3i].xy);
    u_xlat28 = vec3<f32>(v_269.xy, u_xlat28.z);
    let v_270 = ((u_xlat28.xy * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
    u_xlat28 = vec3<f32>(v_270.xy, u_xlat28.z);
    u_xlat8 = textureSampleBias(_MainLightCookieTexture, sampler_MainLightCookieTexture, u_xlat28.xy, v_1._GlobalMipBias.x);
    let v_271 = vec4<f32>(v_4._MainLightCookieTextureFormat, v_4._MainLightCookieTextureFormat, v_4._MainLightCookieTextureFormat, v_4._MainLightCookieTextureFormat);
    u_xlatb28 = ((vec4<f32>(v_271.x, v_271.y, v_271.z, v_271.w) == vec4<f32>(0.0f, 1.0f, 0.0f, 0.0f))).xy;
    if (u_xlatb28.y) {
      v_11 = u_xlat8.w;
    } else {
      v_11 = u_xlat8.x;
    }
    u_xlat54 = v_11;
    if (u_xlatb28.x) {
      v_12 = u_xlat8.xyz;
    } else {
      let v_272 = u_xlat54;
      v_12 = vec3<f32>(v_272, v_272, v_272);
    }
    u_xlat28 = v_12;
  } else {
    u_xlat28.x = 1.0f;
    u_xlat28.y = 1.0f;
    u_xlat28.z = 1.0f;
  }
  u_xlat28 = (u_xlat28 * v_1._MainLightColor.xyz);
  u_xlat3.x = dot(-(u_xlat4), u_xlat26);
  u_xlat3.x = (u_xlat3.x + u_xlat3.x);
  let v_273 = ((u_xlat26 * -(u_xlat3.xxx)) + -(u_xlat4));
  u_xlat8 = vec4<f32>(v_273.xyz, u_xlat8.w);
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
  u_xlat81 = ((v.unity_SpecCube0_HDR.w * u_xlat81) + 1.0f);
  u_xlat81 = max(u_xlat81, 0.0f);
  u_xlat81 = log2(u_xlat81);
  u_xlat81 = (u_xlat81 * v.unity_SpecCube0_HDR.y);
  u_xlat81 = exp2(u_xlat81);
  u_xlat81 = (u_xlat81 * v.unity_SpecCube0_HDR.x);
  let v_274 = u_xlat8.xyz;
  let v_275 = u_xlat81;
  u_xlat8 = vec4<f32>(((v_274 * vec3<f32>(v_275, v_275, v_275))).xyz, u_xlat8.w);
  let v_276 = u_xlat82;
  let v_277 = u_xlat82;
  u_xlat59 = ((vec2<f32>(v_276, v_276) * vec2<f32>(v_277, v_277)) + vec2<f32>(-1.0f, 1.0f));
  u_xlat81 = (1.0f / u_xlat59.y);
  u_xlat82 = (u_xlat84 + -0.03999999910593032837f);
  u_xlat3.x = ((u_xlat3.x * u_xlat82) + 0.03999999910593032837f);
  u_xlat3.x = (u_xlat3.x * u_xlat81);
  let v_278 = (u_xlat3.xxx * u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_278.xyz, u_xlat8.w);
  let v_279 = ((u_xlat5.xyz * u_xlat6.xyz) + u_xlat8.xyz);
  u_xlat5 = vec4<f32>(v_279.xyz, u_xlat5.w);
  u_xlat0.x = (u_xlat0.x * v.unity_LightData.z);
  u_xlat3.x = dot(u_xlat26, v_1._MainLightPosition.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat28 = (u_xlat0.xxx * u_xlat28);
  let v_280 = (u_xlat4 + v_1._MainLightPosition.xyz);
  u_xlat8 = vec4<f32>(v_280.xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat8.xyz, u_xlat8.xyz);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  let v_281 = (u_xlat0.xxx * u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_281.xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat26, u_xlat8.xyz);
  u_xlat0.x = clamp(u_xlat0.x, 0.0f, 1.0f);
  u_xlat3.x = dot(v_1._MainLightPosition.xyz, u_xlat8.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat0.x = ((u_xlat0.x * u_xlat59.x) + 1.00001001358032226562f);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat3.x = max(u_xlat3.x, 0.10000000149011611938f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat0.x = (u_xlat7.x * u_xlat0.x);
  u_xlat0.x = (u_xlat83 / u_xlat0.x);
  let v_282 = ((u_xlat0.xxx * vec3<f32>(0.03999999910593032837f)) + u_xlat6.xyz);
  u_xlat8 = vec4<f32>(v_282.xyz, u_xlat8.w);
  u_xlat28 = (u_xlat28 * u_xlat8.xyz);
  u_xlat0.x = min(v_1._AdditionalLightsCount.x, v.unity_LightData.y);
  u_xlatu0 = bitcast<u32>(i32(u_xlat0.x));
  u_xlat2.x = ((u_xlat2.x * v_3._AdditionalShadowFadeParams.x) + v_3._AdditionalShadowFadeParams.y);
  u_xlat2.x = clamp(u_xlat2.x, 0.0f, 1.0f);
  let v_283 = vec4<f32>(v_4._AdditionalLightsCookieAtlasTextureFormat, v_4._AdditionalLightsCookieAtlasTextureFormat, v_4._AdditionalLightsCookieAtlasTextureFormat, v_4._AdditionalLightsCookieAtlasTextureFormat);
  let v_284 = ((vec4<f32>(v_283.x, v_283.y, v_283.z, v_283.w) == vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f))).xw;
  u_xlatb3 = vec4<bool>(v_284.x, u_xlatb3.yz, v_284.y);
  u_xlat8.x = 0.0f;
  u_xlat8.y = 0.0f;
  u_xlat8.z = 0.0f;
  u_xlatu_loop_1 = 0u;
  loop {
    if ((u_xlatu_loop_1 < u_xlatu0)) {
      u_xlatu84 = (u_xlatu_loop_1 >> 2u);
      u_xlati85 = bitcast<i32>((u_xlatu_loop_1 & 3u));
      let v_285 = v.unity_LightIndices[bitcast<i32>(u_xlatu84)];
      let v_286 = u_xlati85;
      indexable = array<vec4<u32>, 4u>(vec4<u32>(1065353216u, 0u, 0u, 0u), vec4<u32>(0u, 1065353216u, 0u, 0u), vec4<u32>(0u, 0u, 1065353216u, 0u), vec4<u32>(0u, 0u, 0u, 1065353216u));
      u_xlat84 = dot(v_285, bitcast<vec4<f32>>(indexable[v_286]));
      u_xlati84 = i32(u_xlat84);
      let v_287 = ((-(vs_INTERP8) * v_5._AdditionalLightsPosition[u_xlati84].www) + v_5._AdditionalLightsPosition[u_xlati84].xyz);
      u_xlat9 = vec4<f32>(v_287.xyz, u_xlat9.w);
      u_xlat85 = dot(u_xlat9.xyz, u_xlat9.xyz);
      u_xlat85 = max(u_xlat85, 0.00006103515625f);
      u_xlat86 = inverseSqrt(u_xlat85);
      let v_288 = u_xlat86;
      let v_289 = (vec3<f32>(v_288, v_288, v_288) * u_xlat9.xyz);
      u_xlat10 = vec4<f32>(v_289.xyz, u_xlat10.w);
      u_xlat87 = (1.0f / u_xlat85);
      u_xlat85 = (u_xlat85 * v_5._AdditionalLightsAttenuation[u_xlati84].x);
      u_xlat85 = ((-(u_xlat85) * u_xlat85) + 1.0f);
      u_xlat85 = max(u_xlat85, 0.0f);
      u_xlat85 = (u_xlat85 * u_xlat85);
      u_xlat85 = (u_xlat85 * u_xlat87);
      u_xlat87 = dot(v_5._AdditionalLightsSpotDir[u_xlati84].xyz, u_xlat10.xyz);
      u_xlat87 = ((u_xlat87 * v_5._AdditionalLightsAttenuation[u_xlati84].z) + v_5._AdditionalLightsAttenuation[u_xlati84].w);
      u_xlat87 = clamp(u_xlat87, 0.0f, 1.0f);
      u_xlat87 = (u_xlat87 * u_xlat87);
      u_xlat85 = (u_xlat85 * u_xlat87);
      u_xlati87 = i32(v_3._AdditionalShadowParams[u_xlati84].w);
      u_xlatb88 = (u_xlati87 >= 0i);
      if (u_xlatb88) {
        let v_290 = v_3._AdditionalShadowParams[u_xlati84].z;
        u_xlatb88 = any(!((vec4<f32>() == vec4<f32>(v_290, v_290, v_290, v_290))));
        if (u_xlatb88) {
          let v_291 = ((abs(u_xlat10.zzyz) >= abs(u_xlat10.xyxx))).xyz;
          u_xlatb11 = vec4<bool>(v_291.xyz, u_xlatb11.w);
          u_xlatb88 = (u_xlatb11.y & u_xlatb11.x);
          let v_292 = ((-(u_xlat10.zyzx) < vec4<f32>())).xyw;
          u_xlatb11 = vec4<bool>(v_292.xy, u_xlatb11.z, v_292.z);
          u_xlat11.x = select(4.0f, 5.0f, u_xlatb11.x);
          u_xlat11.y = select(2.0f, 3.0f, u_xlatb11.y);
          u_xlat89 = select(0.0f, 1.0f, u_xlatb11.w);
          if (u_xlatb11.z) {
            v_13 = u_xlat11.y;
          } else {
            v_13 = u_xlat89;
          }
          u_xlat37.x = v_13;
          if (u_xlatb88) {
            v_14 = u_xlat11.x;
          } else {
            v_14 = u_xlat37.x;
          }
          u_xlat88 = v_14;
          u_xlat11.x = trunc(v_3._AdditionalShadowParams[u_xlati84].w);
          u_xlat88 = (u_xlat88 + u_xlat11.x);
          u_xlati87 = i32(u_xlat88);
        }
        u_xlati87 = (u_xlati87 << bitcast<u32>(2i));
        u_xlat11 = (vs_INTERP8.yyyy * v_3._AdditionalLightsWorldToShadow[((u_xlati87 + 1i) / 4i)][((u_xlati87 + 1i) % 4i)]);
        u_xlat11 = ((v_3._AdditionalLightsWorldToShadow[(u_xlati87 / 4i)][(u_xlati87 % 4i)] * vs_INTERP8.xxxx) + u_xlat11);
        u_xlat11 = ((v_3._AdditionalLightsWorldToShadow[((u_xlati87 + 2i) / 4i)][((u_xlati87 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat11);
        u_xlat11 = (u_xlat11 + v_3._AdditionalLightsWorldToShadow[((u_xlati87 + 3i) / 4i)][((u_xlati87 + 3i) % 4i)]);
        let v_293 = (u_xlat11.xyz / u_xlat11.www);
        u_xlat11 = vec4<f32>(v_293.xyz, u_xlat11.w);
        u_xlatb87 = (0.0f < v_3._AdditionalShadowParams[u_xlati84].y);
        if (u_xlatb87) {
          u_xlatb87 = (1.0f == v_3._AdditionalShadowParams[u_xlati84].y);
          if (u_xlatb87) {
            u_xlat12 = (u_xlat11.xyxy + v_3._AdditionalShadowOffset0);
            let v_294 = u_xlat12.xy;
            txVec60 = vec3<f32>(v_294.x, v_294.y, u_xlat11.z);
            let v_295 = txVec60;
            u_xlat13.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_295.xy, v_295.z);
            let v_296 = u_xlat12.zw;
            txVec61 = vec3<f32>(v_296.x, v_296.y, u_xlat11.z);
            let v_297 = txVec61;
            u_xlat13.y = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_297.xy, v_297.z);
            u_xlat12 = (u_xlat11.xyxy + v_3._AdditionalShadowOffset1);
            let v_298 = u_xlat12.xy;
            txVec62 = vec3<f32>(v_298.x, v_298.y, u_xlat11.z);
            let v_299 = txVec62;
            u_xlat13.z = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_299.xy, v_299.z);
            let v_300 = u_xlat12.zw;
            txVec63 = vec3<f32>(v_300.x, v_300.y, u_xlat11.z);
            let v_301 = txVec63;
            u_xlat13.w = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_301.xy, v_301.z);
            u_xlat87 = dot(u_xlat13, vec4<f32>(0.25f));
          } else {
            u_xlatb88 = (2.0f == v_3._AdditionalShadowParams[u_xlati84].y);
            if (u_xlatb88) {
              let v_302 = ((u_xlat11.xy * v_3._AdditionalShadowmapSize.zw) + vec2<f32>(0.5f));
              u_xlat12 = vec4<f32>(v_302.xy, u_xlat12.zw);
              let v_303 = floor(u_xlat12.xy);
              u_xlat12 = vec4<f32>(v_303.xy, u_xlat12.zw);
              u_xlat64 = ((u_xlat11.xy * v_3._AdditionalShadowmapSize.zw) + -(u_xlat12.xy));
              u_xlat13 = (u_xlat64.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
              u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
              let v_304 = (u_xlat14.yw * vec2<f32>(0.07999999821186065674f));
              let v_305 = u_xlat13;
              u_xlat13 = vec4<f32>(v_304.x, v_305.y, v_304.y, v_305.w);
              let v_306 = ((u_xlat14.xz * vec2<f32>(0.5f)) + -(u_xlat64));
              u_xlat14 = vec4<f32>(v_306.xy, u_xlat14.zw);
              u_xlat66 = (-(u_xlat64) + vec2<f32>(1.0f));
              let v_307 = min(u_xlat64, vec2<f32>());
              u_xlat15 = vec4<f32>(v_307.xy, u_xlat15.zw);
              let v_308 = ((-(u_xlat15.xy) * u_xlat15.xy) + u_xlat66);
              u_xlat15 = vec4<f32>(v_308.xy, u_xlat15.zw);
              u_xlat64 = max(u_xlat64, vec2<f32>());
              u_xlat64 = ((-(u_xlat64) * u_xlat64) + u_xlat13.yw);
              let v_309 = (u_xlat15.xy + vec2<f32>(1.0f));
              u_xlat15 = vec4<f32>(v_309.xy, u_xlat15.zw);
              u_xlat64 = (u_xlat64 + vec2<f32>(1.0f));
              let v_310 = (u_xlat14.xy * vec2<f32>(0.15999999642372131348f));
              u_xlat16 = vec4<f32>(v_310.xy, u_xlat16.zw);
              let v_311 = (u_xlat66 * vec2<f32>(0.15999999642372131348f));
              u_xlat14 = vec4<f32>(v_311.xy, u_xlat14.zw);
              let v_312 = (u_xlat15.xy * vec2<f32>(0.15999999642372131348f));
              u_xlat15 = vec4<f32>(v_312.xy, u_xlat15.zw);
              let v_313 = (u_xlat64 * vec2<f32>(0.15999999642372131348f));
              u_xlat17 = vec4<f32>(v_313.xy, u_xlat17.zw);
              u_xlat64 = (u_xlat13.yw * vec2<f32>(0.15999999642372131348f));
              u_xlat16.z = u_xlat15.x;
              u_xlat16.w = u_xlat64.x;
              u_xlat14.z = u_xlat17.x;
              u_xlat14.w = u_xlat13.x;
              u_xlat18 = (u_xlat14.zwxz + u_xlat16.zwxz);
              u_xlat15.z = u_xlat16.y;
              u_xlat15.w = u_xlat64.y;
              u_xlat17.z = u_xlat14.y;
              u_xlat17.w = u_xlat13.z;
              let v_314 = (u_xlat15.zyw + u_xlat17.zyw);
              u_xlat13 = vec4<f32>(v_314.xyz, u_xlat13.w);
              let v_315 = (u_xlat14.xzw / u_xlat18.zwy);
              u_xlat14 = vec4<f32>(v_315.xyz, u_xlat14.w);
              let v_316 = (u_xlat14.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
              u_xlat14 = vec4<f32>(v_316.xyz, u_xlat14.w);
              let v_317 = (u_xlat17.zyw / u_xlat13.xyz);
              u_xlat15 = vec4<f32>(v_317.xyz, u_xlat15.w);
              let v_318 = (u_xlat15.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
              u_xlat15 = vec4<f32>(v_318.xyz, u_xlat15.w);
              let v_319 = (u_xlat14.yxz * v_3._AdditionalShadowmapSize.xxx);
              u_xlat14 = vec4<f32>(v_319.xyz, u_xlat14.w);
              let v_320 = (u_xlat15.xyz * v_3._AdditionalShadowmapSize.yyy);
              u_xlat15 = vec4<f32>(v_320.xyz, u_xlat15.w);
              u_xlat14.w = u_xlat15.x;
              u_xlat16 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat14.ywxw);
              u_xlat64 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat14.zw);
              u_xlat15.w = u_xlat14.y;
              let v_321 = u_xlat15.yz;
              let v_322 = u_xlat14;
              u_xlat14 = vec4<f32>(v_322.x, v_321.x, v_322.z, v_321.y);
              u_xlat17 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat14.xyzy);
              u_xlat15 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat15.wywz);
              u_xlat14 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat14.xwzw);
              u_xlat19 = (u_xlat13.xxxy * u_xlat18.zwyz);
              u_xlat20 = (u_xlat13.yyzz * u_xlat18);
              u_xlat88 = (u_xlat13.z * u_xlat18.y);
              let v_323 = u_xlat16.xy;
              txVec64 = vec3<f32>(v_323.x, v_323.y, u_xlat11.z);
              let v_324 = txVec64;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_324.xy, v_324.z);
              let v_325 = u_xlat16.zw;
              txVec65 = vec3<f32>(v_325.x, v_325.y, u_xlat11.z);
              let v_326 = txVec65;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_326.xy, v_326.z);
              u_xlat12.x = (u_xlat12.x * u_xlat19.y);
              u_xlat89 = ((u_xlat19.x * u_xlat89) + u_xlat12.x);
              let v_327 = u_xlat64;
              txVec66 = vec3<f32>(v_327.x, v_327.y, u_xlat11.z);
              let v_328 = txVec66;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_328.xy, v_328.z);
              u_xlat89 = ((u_xlat19.z * u_xlat12.x) + u_xlat89);
              let v_329 = u_xlat15.xy;
              txVec67 = vec3<f32>(v_329.x, v_329.y, u_xlat11.z);
              let v_330 = txVec67;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_330.xy, v_330.z);
              u_xlat89 = ((u_xlat19.w * u_xlat12.x) + u_xlat89);
              let v_331 = u_xlat17.xy;
              txVec68 = vec3<f32>(v_331.x, v_331.y, u_xlat11.z);
              let v_332 = txVec68;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_332.xy, v_332.z);
              u_xlat89 = ((u_xlat20.x * u_xlat12.x) + u_xlat89);
              let v_333 = u_xlat17.zw;
              txVec69 = vec3<f32>(v_333.x, v_333.y, u_xlat11.z);
              let v_334 = txVec69;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_334.xy, v_334.z);
              u_xlat89 = ((u_xlat20.y * u_xlat12.x) + u_xlat89);
              let v_335 = u_xlat15.zw;
              txVec70 = vec3<f32>(v_335.x, v_335.y, u_xlat11.z);
              let v_336 = txVec70;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_336.xy, v_336.z);
              u_xlat89 = ((u_xlat20.z * u_xlat12.x) + u_xlat89);
              let v_337 = u_xlat14.xy;
              txVec71 = vec3<f32>(v_337.x, v_337.y, u_xlat11.z);
              let v_338 = txVec71;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_338.xy, v_338.z);
              u_xlat89 = ((u_xlat20.w * u_xlat12.x) + u_xlat89);
              let v_339 = u_xlat14.zw;
              txVec72 = vec3<f32>(v_339.x, v_339.y, u_xlat11.z);
              let v_340 = txVec72;
              u_xlat12.x = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_340.xy, v_340.z);
              u_xlat87 = ((u_xlat88 * u_xlat12.x) + u_xlat89);
            } else {
              let v_341 = ((u_xlat11.xy * v_3._AdditionalShadowmapSize.zw) + vec2<f32>(0.5f));
              u_xlat12 = vec4<f32>(v_341.xy, u_xlat12.zw);
              let v_342 = floor(u_xlat12.xy);
              u_xlat12 = vec4<f32>(v_342.xy, u_xlat12.zw);
              u_xlat64 = ((u_xlat11.xy * v_3._AdditionalShadowmapSize.zw) + -(u_xlat12.xy));
              u_xlat13 = (u_xlat64.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
              u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
              let v_343 = (u_xlat14.yw * vec2<f32>(0.04081600159406661987f));
              let v_344 = u_xlat15;
              u_xlat15 = vec4<f32>(v_344.x, v_343.x, v_344.z, v_343.y);
              let v_345 = ((u_xlat14.xz * vec2<f32>(0.5f)) + -(u_xlat64));
              let v_346 = u_xlat13;
              u_xlat13 = vec4<f32>(v_345.x, v_346.y, v_345.y, v_346.w);
              let v_347 = (-(u_xlat64) + vec2<f32>(1.0f));
              u_xlat14 = vec4<f32>(v_347.xy, u_xlat14.zw);
              u_xlat66 = min(u_xlat64, vec2<f32>());
              let v_348 = ((-(u_xlat66) * u_xlat66) + u_xlat14.xy);
              u_xlat14 = vec4<f32>(v_348.xy, u_xlat14.zw);
              u_xlat66 = max(u_xlat64, vec2<f32>());
              let v_349 = ((-(u_xlat66) * u_xlat66) + u_xlat13.yw);
              u_xlat39 = vec3<f32>(v_349.x, u_xlat39.y, v_349.y);
              let v_350 = (u_xlat14.xy + vec2<f32>(2.0f));
              u_xlat14 = vec4<f32>(v_350.xy, u_xlat14.zw);
              let v_351 = (u_xlat39.xz + vec2<f32>(2.0f));
              let v_352 = u_xlat13;
              u_xlat13 = vec4<f32>(v_352.x, v_351.x, v_352.z, v_351.y);
              u_xlat16.z = (u_xlat13.y * 0.08163200318813323975f);
              let v_353 = (u_xlat13.zxw * vec3<f32>(0.08163200318813323975f));
              u_xlat17 = vec4<f32>(v_353.xyz, u_xlat17.w);
              let v_354 = (u_xlat14.xy * vec2<f32>(0.08163200318813323975f));
              u_xlat13 = vec4<f32>(v_354.xy, u_xlat13.zw);
              u_xlat16.x = u_xlat17.y;
              let v_355 = ((u_xlat64.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
              let v_356 = u_xlat16;
              u_xlat16 = vec4<f32>(v_356.x, v_355.x, v_356.z, v_355.y);
              let v_357 = ((u_xlat64.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
              let v_358 = u_xlat14;
              u_xlat14 = vec4<f32>(v_357.x, v_358.y, v_357.y, v_358.w);
              u_xlat14.y = u_xlat13.x;
              u_xlat14.w = u_xlat15.y;
              u_xlat16 = (u_xlat14 + u_xlat16);
              let v_359 = ((u_xlat64.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
              let v_360 = u_xlat17;
              u_xlat17 = vec4<f32>(v_360.x, v_359.x, v_360.z, v_359.y);
              let v_361 = ((u_xlat64.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
              let v_362 = u_xlat15;
              u_xlat15 = vec4<f32>(v_361.x, v_362.y, v_361.y, v_362.w);
              u_xlat15.y = u_xlat13.y;
              u_xlat13 = (u_xlat15 + u_xlat17);
              u_xlat14 = (u_xlat14 / u_xlat16);
              u_xlat14 = (u_xlat14 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
              u_xlat15 = (u_xlat15 / u_xlat13);
              u_xlat15 = (u_xlat15 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
              u_xlat14 = (u_xlat14.wxyz * v_3._AdditionalShadowmapSize.xxxx);
              u_xlat15 = (u_xlat15.xwyz * v_3._AdditionalShadowmapSize.yyyy);
              let v_363 = u_xlat14.yzw;
              u_xlat17 = vec4<f32>(v_363.x, u_xlat17.y, v_363.yz);
              u_xlat17.y = u_xlat15.x;
              u_xlat18 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
              u_xlat64 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat17.wy);
              u_xlat14.y = u_xlat17.y;
              u_xlat17.y = u_xlat15.z;
              u_xlat19 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
              let v_364 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat17.wy);
              u_xlat20 = vec4<f32>(v_364.xy, u_xlat20.zw);
              u_xlat14.z = u_xlat17.y;
              u_xlat21 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat14.xyxz);
              u_xlat17.y = u_xlat15.w;
              u_xlat22 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat17.xyzy);
              u_xlat40 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat17.wy);
              u_xlat14.w = u_xlat17.y;
              u_xlat72 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat14.xw);
              let v_365 = u_xlat17.xzw;
              u_xlat15 = vec4<f32>(v_365.x, u_xlat15.y, v_365.yz);
              u_xlat17 = ((u_xlat12.xyxy * v_3._AdditionalShadowmapSize.xyxy) + u_xlat15.xyzy);
              u_xlat67 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat15.wy);
              u_xlat15.x = u_xlat14.x;
              let v_366 = ((u_xlat12.xy * v_3._AdditionalShadowmapSize.xy) + u_xlat15.xy);
              u_xlat12 = vec4<f32>(v_366.xy, u_xlat12.zw);
              u_xlat23 = (u_xlat13.xxxx * u_xlat16);
              u_xlat24 = (u_xlat13.yyyy * u_xlat16);
              u_xlat25 = (u_xlat13.zzzz * u_xlat16);
              u_xlat13 = (u_xlat13.wwww * u_xlat16);
              let v_367 = u_xlat18.xy;
              txVec73 = vec3<f32>(v_367.x, v_367.y, u_xlat11.z);
              let v_368 = txVec73;
              u_xlat88 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_368.xy, v_368.z);
              let v_369 = u_xlat18.zw;
              txVec74 = vec3<f32>(v_369.x, v_369.y, u_xlat11.z);
              let v_370 = txVec74;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_370.xy, v_370.z);
              u_xlat89 = (u_xlat89 * u_xlat23.y);
              u_xlat88 = ((u_xlat23.x * u_xlat88) + u_xlat89);
              let v_371 = u_xlat64;
              txVec75 = vec3<f32>(v_371.x, v_371.y, u_xlat11.z);
              let v_372 = txVec75;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_372.xy, v_372.z);
              u_xlat88 = ((u_xlat23.z * u_xlat89) + u_xlat88);
              let v_373 = u_xlat21.xy;
              txVec76 = vec3<f32>(v_373.x, v_373.y, u_xlat11.z);
              let v_374 = txVec76;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_374.xy, v_374.z);
              u_xlat88 = ((u_xlat23.w * u_xlat89) + u_xlat88);
              let v_375 = u_xlat19.xy;
              txVec77 = vec3<f32>(v_375.x, v_375.y, u_xlat11.z);
              let v_376 = txVec77;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_376.xy, v_376.z);
              u_xlat88 = ((u_xlat24.x * u_xlat89) + u_xlat88);
              let v_377 = u_xlat19.zw;
              txVec78 = vec3<f32>(v_377.x, v_377.y, u_xlat11.z);
              let v_378 = txVec78;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_378.xy, v_378.z);
              u_xlat88 = ((u_xlat24.y * u_xlat89) + u_xlat88);
              let v_379 = u_xlat20.xy;
              txVec79 = vec3<f32>(v_379.x, v_379.y, u_xlat11.z);
              let v_380 = txVec79;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_380.xy, v_380.z);
              u_xlat88 = ((u_xlat24.z * u_xlat89) + u_xlat88);
              let v_381 = u_xlat21.zw;
              txVec80 = vec3<f32>(v_381.x, v_381.y, u_xlat11.z);
              let v_382 = txVec80;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_382.xy, v_382.z);
              u_xlat88 = ((u_xlat24.w * u_xlat89) + u_xlat88);
              let v_383 = u_xlat22.xy;
              txVec81 = vec3<f32>(v_383.x, v_383.y, u_xlat11.z);
              let v_384 = txVec81;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_384.xy, v_384.z);
              u_xlat88 = ((u_xlat25.x * u_xlat89) + u_xlat88);
              let v_385 = u_xlat22.zw;
              txVec82 = vec3<f32>(v_385.x, v_385.y, u_xlat11.z);
              let v_386 = txVec82;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_386.xy, v_386.z);
              u_xlat88 = ((u_xlat25.y * u_xlat89) + u_xlat88);
              let v_387 = u_xlat40;
              txVec83 = vec3<f32>(v_387.x, v_387.y, u_xlat11.z);
              let v_388 = txVec83;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_388.xy, v_388.z);
              u_xlat88 = ((u_xlat25.z * u_xlat89) + u_xlat88);
              let v_389 = u_xlat72;
              txVec84 = vec3<f32>(v_389.x, v_389.y, u_xlat11.z);
              let v_390 = txVec84;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_390.xy, v_390.z);
              u_xlat88 = ((u_xlat25.w * u_xlat89) + u_xlat88);
              let v_391 = u_xlat17.xy;
              txVec85 = vec3<f32>(v_391.x, v_391.y, u_xlat11.z);
              let v_392 = txVec85;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_392.xy, v_392.z);
              u_xlat88 = ((u_xlat13.x * u_xlat89) + u_xlat88);
              let v_393 = u_xlat17.zw;
              txVec86 = vec3<f32>(v_393.x, v_393.y, u_xlat11.z);
              let v_394 = txVec86;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_394.xy, v_394.z);
              u_xlat88 = ((u_xlat13.y * u_xlat89) + u_xlat88);
              let v_395 = u_xlat67;
              txVec87 = vec3<f32>(v_395.x, v_395.y, u_xlat11.z);
              let v_396 = txVec87;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_396.xy, v_396.z);
              u_xlat88 = ((u_xlat13.z * u_xlat89) + u_xlat88);
              let v_397 = u_xlat12.xy;
              txVec88 = vec3<f32>(v_397.x, v_397.y, u_xlat11.z);
              let v_398 = txVec88;
              u_xlat89 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_398.xy, v_398.z);
              u_xlat87 = ((u_xlat13.w * u_xlat89) + u_xlat88);
            }
          }
        } else {
          let v_399 = u_xlat11.xy;
          txVec89 = vec3<f32>(v_399.x, v_399.y, u_xlat11.z);
          let v_400 = txVec89;
          u_xlat87 = textureSampleCompareLevel(_AdditionalLightsShadowmapTexture, sampler_LinearClampCompare, v_400.xy, v_400.z);
        }
        u_xlat88 = (1.0f + -(v_3._AdditionalShadowParams[u_xlati84].x));
        u_xlat87 = ((u_xlat87 * v_3._AdditionalShadowParams[u_xlati84].x) + u_xlat88);
        u_xlatb88 = (0.0f >= u_xlat11.z);
        u_xlatb11.x = (u_xlat11.z >= 1.0f);
        u_xlatb88 = (u_xlatb88 | u_xlatb11.x);
        let v_401 = u_xlatb88;
        u_xlat87 = select(u_xlat87, 1.0f, v_401);
      } else {
        u_xlat87 = 1.0f;
      }
      u_xlat88 = (-(u_xlat87) + 1.0f);
      u_xlat87 = ((u_xlat2.x * u_xlat88) + u_xlat87);
      u_xlati88 = (1i << bitcast<u32>((u_xlati84 & 31i)));
      u_xlati88 = bitcast<i32>((bitcast<u32>(u_xlati88) & bitcast<u32>(v_4._AdditionalLightsCookieEnableBits)));
      if ((u_xlati88 != 0i)) {
        u_xlati88 = i32(v_4._AdditionalLightsLightTypes[u_xlati84].tint_element);
        u_xlati11 = select(1i, 0i, (u_xlati88 != 0i));
        u_xlati37 = (u_xlati84 << bitcast<u32>(2i));
        if ((u_xlati11 != 0i)) {
          let v_402 = (vs_INTERP8.yyy * v_4._AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xyw);
          u_xlat11 = vec4<f32>(v_402.x, u_xlat11.y, v_402.yz);
          let v_403 = ((v_4._AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xyw * vs_INTERP8.xxx) + u_xlat11.xzw);
          u_xlat11 = vec4<f32>(v_403.x, u_xlat11.y, v_403.yz);
          let v_404 = ((v_4._AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xyw * vs_INTERP8.zzz) + u_xlat11.xzw);
          u_xlat11 = vec4<f32>(v_404.x, u_xlat11.y, v_404.yz);
          let v_405 = (u_xlat11.xzw + v_4._AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xyw);
          u_xlat11 = vec4<f32>(v_405.x, u_xlat11.y, v_405.yz);
          let v_406 = (u_xlat11.xz / u_xlat11.ww);
          let v_407 = u_xlat11;
          u_xlat11 = vec4<f32>(v_406.x, v_407.y, v_406.y, v_407.w);
          let v_408 = ((u_xlat11.xz * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
          let v_409 = u_xlat11;
          u_xlat11 = vec4<f32>(v_408.x, v_409.y, v_408.y, v_409.w);
          let v_410 = clamp(u_xlat11.xz, vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
          let v_411 = u_xlat11;
          u_xlat11 = vec4<f32>(v_410.x, v_411.y, v_410.y, v_411.w);
          let v_412 = ((v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat11.xz) + v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
          let v_413 = u_xlat11;
          u_xlat11 = vec4<f32>(v_412.x, v_413.y, v_412.y, v_413.w);
        } else {
          u_xlatb88 = (u_xlati88 == 1i);
          u_xlati88 = select(0i, 1i, u_xlatb88);
          if ((u_xlati88 != 0i)) {
            let v_414 = (vs_INTERP8.yy * v_4._AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xy);
            u_xlat12 = vec4<f32>(v_414.xy, u_xlat12.zw);
            let v_415 = ((v_4._AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xy * vs_INTERP8.xx) + u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_415.xy, u_xlat12.zw);
            let v_416 = ((v_4._AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xy * vs_INTERP8.zz) + u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_416.xy, u_xlat12.zw);
            let v_417 = (u_xlat12.xy + v_4._AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xy);
            u_xlat12 = vec4<f32>(v_417.xy, u_xlat12.zw);
            let v_418 = ((u_xlat12.xy * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
            u_xlat12 = vec4<f32>(v_418.xy, u_xlat12.zw);
            let v_419 = fract(u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_419.xy, u_xlat12.zw);
            let v_420 = ((v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat12.xy) + v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
            let v_421 = u_xlat11;
            u_xlat11 = vec4<f32>(v_420.x, v_421.y, v_420.y, v_421.w);
          } else {
            u_xlat12 = (vs_INTERP8.yyyy * v_4._AdditionalLightsWorldToLights[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)]);
            u_xlat12 = ((v_4._AdditionalLightsWorldToLights[(u_xlati37 / 4i)][(u_xlati37 % 4i)] * vs_INTERP8.xxxx) + u_xlat12);
            u_xlat12 = ((v_4._AdditionalLightsWorldToLights[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat12);
            u_xlat12 = (u_xlat12 + v_4._AdditionalLightsWorldToLights[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)]);
            let v_422 = (u_xlat12.xyz / u_xlat12.www);
            u_xlat12 = vec4<f32>(v_422.xyz, u_xlat12.w);
            u_xlat88 = dot(u_xlat12.xyz, u_xlat12.xyz);
            u_xlat88 = inverseSqrt(u_xlat88);
            let v_423 = u_xlat88;
            let v_424 = (vec3<f32>(v_423, v_423, v_423) * u_xlat12.xyz);
            u_xlat12 = vec4<f32>(v_424.xyz, u_xlat12.w);
            u_xlat88 = dot(abs(u_xlat12.xyz), vec3<f32>(1.0f));
            u_xlat88 = max(u_xlat88, 0.00000099999999747524f);
            u_xlat88 = (1.0f / u_xlat88);
            let v_425 = u_xlat88;
            let v_426 = (vec3<f32>(v_425, v_425, v_425) * u_xlat12.zxy);
            u_xlat13 = vec4<f32>(v_426.xyz, u_xlat13.w);
            u_xlat13.x = -(u_xlat13.x);
            u_xlat13.x = clamp(u_xlat13.x, 0.0f, 1.0f);
            let v_427 = ((u_xlat13.yyzz >= vec4<f32>())).xz;
            u_xlatb37 = vec3<bool>(v_427.x, u_xlatb37.y, v_427.y);
            if (u_xlatb37.x) {
              v_15 = u_xlat13.x;
            } else {
              v_15 = -(u_xlat13.x);
            }
            u_xlat37.x = v_15;
            if (u_xlatb37.z) {
              v_16 = u_xlat13.x;
            } else {
              v_16 = -(u_xlat13.x);
            }
            u_xlat37.z = v_16;
            let v_428 = u_xlat12.xy;
            let v_429 = u_xlat88;
            let v_430 = ((v_428 * vec2<f32>(v_429, v_429)) + u_xlat37.xz);
            u_xlat37 = vec3<f32>(v_430.x, u_xlat37.y, v_430.y);
            let v_431 = ((u_xlat37.xz * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
            u_xlat37 = vec3<f32>(v_431.x, u_xlat37.y, v_431.y);
            let v_432 = clamp(u_xlat37.xz, vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
            u_xlat37 = vec3<f32>(v_432.x, u_xlat37.y, v_432.y);
            let v_433 = ((v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].xy * u_xlat37.xz) + v_4._AdditionalLightsCookieAtlasUVRects[u_xlati84].zw);
            let v_434 = u_xlat11;
            u_xlat11 = vec4<f32>(v_433.x, v_434.y, v_433.y, v_434.w);
          }
        }
        u_xlat11 = textureSampleLevel(_AdditionalLightsCookieAtlasTexture, sampler_LinearClamp, u_xlat11.xz, 0.0f);
        if (u_xlatb3.w) {
          v_17 = u_xlat11.w;
        } else {
          v_17 = u_xlat11.x;
        }
        u_xlat88 = v_17;
        if (u_xlatb3.x) {
          v_18 = u_xlat11.xyz;
        } else {
          let v_435 = u_xlat88;
          v_18 = vec3<f32>(v_435, v_435, v_435);
        }
        let v_436 = v_18;
        u_xlat11 = vec4<f32>(v_436.xyz, u_xlat11.w);
      } else {
        u_xlat11.x = 1.0f;
        u_xlat11.y = 1.0f;
        u_xlat11.z = 1.0f;
      }
      let v_437 = (u_xlat11.xyz * v_5._AdditionalLightsColor[u_xlati84].xyz);
      u_xlat11 = vec4<f32>(v_437.xyz, u_xlat11.w);
      u_xlat84 = (u_xlat85 * u_xlat87);
      u_xlat85 = dot(u_xlat26, u_xlat10.xyz);
      u_xlat85 = clamp(u_xlat85, 0.0f, 1.0f);
      u_xlat84 = (u_xlat84 * u_xlat85);
      let v_438 = u_xlat84;
      let v_439 = (vec3<f32>(v_438, v_438, v_438) * u_xlat11.xyz);
      u_xlat11 = vec4<f32>(v_439.xyz, u_xlat11.w);
      let v_440 = u_xlat9.xyz;
      let v_441 = u_xlat86;
      let v_442 = ((v_440 * vec3<f32>(v_441, v_441, v_441)) + u_xlat4);
      u_xlat9 = vec4<f32>(v_442.xyz, u_xlat9.w);
      u_xlat84 = dot(u_xlat9.xyz, u_xlat9.xyz);
      u_xlat84 = max(u_xlat84, 1.17549435e-38f);
      u_xlat84 = inverseSqrt(u_xlat84);
      let v_443 = u_xlat84;
      let v_444 = (vec3<f32>(v_443, v_443, v_443) * u_xlat9.xyz);
      u_xlat9 = vec4<f32>(v_444.xyz, u_xlat9.w);
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
      let v_445 = u_xlat84;
      let v_446 = ((vec3<f32>(v_445, v_445, v_445) * vec3<f32>(0.03999999910593032837f)) + u_xlat6.xyz);
      u_xlat9 = vec4<f32>(v_446.xyz, u_xlat9.w);
      let v_447 = ((u_xlat9.xyz * u_xlat11.xyz) + u_xlat8.xyz);
      u_xlat8 = vec4<f32>(v_447.xyz, u_xlat8.w);
      continue;
    } else {
      break;
    }

    continuing {
      u_xlatu_loop_1 = (u_xlatu_loop_1 + bitcast<u32>(1i));
    }
  }
  let v_448 = u_xlat5.xyz;
  let v_449 = u_xlat33;
  u_xlat0 = ((v_448 * vec3<f32>(v_449, v_449, v_449)) + u_xlat28);
  u_xlat0 = (u_xlat8.xyz + u_xlat0);
  u_xlat0 = ((vs_INTERP6.www * u_xlat1) + u_xlat0);
  u_xlat78 = (u_xlat55.x * -(u_xlat55.x));
  u_xlat78 = exp2(u_xlat78);
  u_xlat0 = (u_xlat0 + -(v_1.unity_FogColor.xyz));
  let v_450 = u_xlat78;
  let v_451 = ((vec3<f32>(v_450, v_450, v_450) * u_xlat0) + v_1.unity_FogColor.xyz);
  SV_Target0 = vec4<f32>(v_451.xyz, SV_Target0.w);
  let v_452 = u_xlatb29;
  SV_Target0.w = select(1.0f, u_xlat79, v_452);
}

fn v_149(base : ptr<function, i32>, insert : ptr<function, i32>, offset : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  mask = (~((4294967295u << bitcast<u32>(*(bits)))) << bitcast<u32>(*(offset)));
  return bitcast<i32>(((bitcast<u32>(*(base)) & ~(mask)) | ((bitcast<u32>(*(insert)) << bitcast<u32>(*(offset))) & mask)));
}

@fragment
fn main(@location(5u) vs_INTERP9 : vec3<f32>, @location(1u) vs_INTERP4 : vec4<f32>, @location(4u) vs_INTERP8 : vec3<f32>, @location(2u) vs_INTERP5 : vec4<f32>, @location(3u) vs_INTERP6 : vec4<f32>, @location(0u) vs_INTERP0 : vec2<f32>, @builtin(position) gl_FragCoord : vec4<f32>) -> @location(0u) vec4<f32> {
  main_inner(vs_INTERP9, vs_INTERP4, vs_INTERP8, vs_INTERP5, vs_INTERP6, vs_INTERP0, gl_FragCoord);
  return SV_Target0;
}
