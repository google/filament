diagnostic(off, derivative_uniformity);

alias Arr = array<u32, 4u>;

struct x_positionNormalBuffer_type {
  /* @offset(0) */
  value : Arr,
}

alias RTArr = array<x_positionNormalBuffer_type>;

struct x_positionNormalBuffer {
  /* @offset(0) */
  x_positionNormalBuffer_buf : RTArr,
}

struct VGlobals {
  /* @offset(0) */
  x_WorldSpaceCameraPos : vec3f,
  /* @offset(16) */
  unity_MatrixInvV : mat4x4f,
  /* @offset(80) */
  unity_MatrixVP : mat4x4f,
  /* @offset(144) */
  x_ProbeSize : f32,
  /* @offset(148) */
  x_ForceDebugNormalViewBias : i32,
}

struct ShaderVariablesProbeVolumes {
  /* @offset(0) */
  x_PoolDim_CellInMeters : vec4f,
  /* @offset(16) */
  x_RcpPoolDim_Padding : vec4f,
  /* @offset(32) */
  x_MinEntryPos_Noise : vec4f,
  /* @offset(48) */
  x_IndicesDim_IndexChunkSize : vec4f,
  /* @offset(64) */
  x_Biases_CellInMinBrick_MinBrickSize : vec4f,
  /* @offset(80) */
  x_LeakReductionParams : vec4f,
  /* @offset(96) */
  x_Weight_MinLoadedCellInEntries : vec4f,
  /* @offset(112) */
  x_MaxLoadedCellInEntries_FrameIndex : vec4f,
  /* @offset(128) */
  x_NormalizationClamp_IndirectionEntryDim_Padding : vec4f,
}

alias Arr_1 = array<u32, 3u>;

struct x_APVResCellIndices_type {
  /* @offset(0) */
  value : Arr_1,
}

alias RTArr_1 = array<x_APVResCellIndices_type>;

struct x_APVResCellIndices {
  /* @offset(0) */
  x_APVResCellIndices_buf : RTArr_1,
}

alias Arr_2 = array<u32, 1u>;

struct x_APVResIndex_type {
  /* @offset(0) */
  value : Arr_2,
}

alias RTArr_2 = array<x_APVResIndex_type>;

struct x_APVResIndex {
  /* @offset(0) */
  x_APVResIndex_buf : RTArr_2,
}

alias Arr_3 = array<vec4f, 2u>;

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
  unity_LightIndices : Arr_3,
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

var<private> u_xlat0 : vec4f;

@group(0) @binding(5) var<storage, read> x_38 : x_positionNormalBuffer;

var<private> u_xlat1 : vec4f;

var<private> u_xlat2 : vec4f;

@group(1) @binding(1) var<uniform> x_76 : VGlobals;

var<private> u_xlat42 : f32;

var<private> u_xlat3 : vec3f;

@group(1) @binding(3) var<uniform> x_105 : ShaderVariablesProbeVolumes;

var<private> u_xlatb4 : vec3<bool>;

var<private> u_xlatb5 : vec3<bool>;

var<private> u_xlatb42 : bool;

var<private> u_xlat4 : vec3f;

var<private> u_xlati4 : vec3i;

var<private> u_xlati5 : vec3i;

var<private> u_xlati43 : i32;

var<private> u_xlati45 : i32;

var<private> u_xlatu4 : vec4u;

@group(0) @binding(3) var<storage, read> x_237 : x_APVResCellIndices;

var<private> u_xlatb43 : bool;

var<private> u_xlatu45 : u32;

var<private> u_xlat45 : f32;

var<private> u_xlatu6 : vec4u;

var<private> u_xlatu7 : vec4u;

var<private> u_xlatu5 : vec4u;

var<private> u_xlat43 : f32;

var<private> u_xlati3 : vec3i;

var<private> u_xlatb20 : vec3<bool>;

var<private> u_xlatb7 : vec3<bool>;

var<private> u_xlati7 : vec2i;

var<private> u_xlatu42 : u32;

@group(0) @binding(2) var<storage, read> x_508 : x_APVResIndex;

var<private> u_xlatu43 : u32;

var<private> u_xlat5 : vec4f;

var<private> u_xlat6 : vec4f;

var<private> u_xlat8 : vec4f;

@group(0) @binding(4) var x_APVResValidity : texture_3d<f32>;

var<private> u_xlat46 : f32;

var<private> u_xlat9 : vec4f;

var<private> u_xlat47 : f32;

var<private> u_xlat10 : vec4f;

var<private> u_xlat7 : vec4f;

var<private> u_xlat11 : vec4f;

var<private> u_xlat50 : f32;

var<private> u_xlat36 : f32;

var<private> u_xlatu10 : vec4u;

var<private> u_xlat12 : vec3f;

var<private> u_xlat13 : vec3f;

var<private> in_COLOR0 : vec4f;

var<private> u_xlatb8 : vec4<bool>;

var<private> u_xlat18 : vec3f;

var<private> u_xlatb45 : bool;

var<private> u_xlat32 : f32;

var<private> u_xlatb32 : bool;

@group(1) @binding(2) var<uniform> x_1804 : UnityPerDraw;

var<private> in_POSITION0 : vec4f;

var<private> u_xlatb0 : bool;

var<private> vs_COLOR0 : vec4f;

var<private> phase0_Output0_3 : vec4f;

var<private> vs_TEXCOORD1 : vec3f;

var<private> vs_TEXCOORD0 : vec2f;

var<private> vs_TEXCOORD2 : vec2f;

var<private> u_xlat28 : f32;

var<private> in_NORMAL0 : vec3f;

var<private> in_TEXCOORD0 : vec2f;

var<private> gl_Position : vec4f;

fn uint_bitfieldExtract_u1_i1_i1_(value : ptr<function, u32>, offset_1 : ptr<function, i32>, bits : ptr<function, i32>) -> u32 {
  let x_16 = *(value);
  let x_17 = *(offset_1);
  let x_21 = *(bits);
  return ((x_16 >> bitcast<u32>(x_17)) & ~((4294967295u << bitcast<u32>(x_21))));
}

const x_329 = vec3u(4294967295u);

const x_586 = vec3f(0.5f);

const x_590 = vec3f(3.0f);

const x_607 = vec3f(0.3333333432674407959f, 0.3333333432674407959f, 0.0f);

const x_657 = vec3f(1.0f);

const x_1164 = vec3f(1.0f, 0.0f, 0.0f);

const x_1191 = vec3f(0.0f, 0.0f, 1.0f);

const x_1494 = vec2f(1.0f);

fn main_1() {
  var param : u32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : u32;
  var param_4 : i32;
  var param_5 : i32;
  var param_6 : u32;
  var param_7 : i32;
  var param_8 : i32;
  var param_9 : u32;
  var param_10 : i32;
  var param_11 : i32;
  var x_323 : vec3u;
  var x_337 : vec3u;
  var x_348 : vec2u;
  var x_1253 : vec3f;
  var x_1743 : f32;
  var x_1751 : f32;
  var x_1760 : f32;
  var x_1769 : f32;
  var u_xlat_precise_vec4 : vec4f;
  var u_xlat_precise_ivec4 : vec4i;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4u;
  u_xlat0 = vec4f(vec3f(bitcast<f32>(x_38.x_positionNormalBuffer_buf[0i].value[0i]), bitcast<f32>(x_38.x_positionNormalBuffer_buf[0i].value[1i]), bitcast<f32>(x_38.x_positionNormalBuffer_buf[0i].value[2i])).xyz, u_xlat0.w);
  u_xlat1 = vec4f(vec3f(bitcast<f32>(x_38.x_positionNormalBuffer_buf[1i].value[0i]), bitcast<f32>(x_38.x_positionNormalBuffer_buf[1i].value[1i]), bitcast<f32>(x_38.x_positionNormalBuffer_buf[1i].value[2i])).xyz, u_xlat1.w);
  u_xlat2 = vec4f(((-(u_xlat0.xyz) + x_76.x_WorldSpaceCameraPos)).xyz, u_xlat2.w);
  u_xlat42 = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat42 = inverseSqrt(u_xlat42);
  u_xlat2 = vec4f(((vec3f(u_xlat42) * u_xlat2.xyz)).xyz, u_xlat2.w);
  u_xlat3 = ((u_xlat1.xyz * x_105.x_Biases_CellInMinBrick_MinBrickSize.xxx) + u_xlat0.xyz);
  u_xlat2 = vec4f((((u_xlat2.xyz * x_105.x_Biases_CellInMinBrick_MinBrickSize.yyy) + u_xlat3)).xyz, u_xlat2.w);
  u_xlat3 = (u_xlat2.xyz / x_105.x_NormalizationClamp_IndirectionEntryDim_Padding.zzz);
  u_xlat3 = floor(u_xlat3);
  u_xlatb4 = ((u_xlat3.xyzx >= x_105.x_Weight_MinLoadedCellInEntries.yzwy)).xyz;
  u_xlatb5 = ((x_105.x_MaxLoadedCellInEntries_FrameIndex.xyzx >= u_xlat3.xyzx)).xyz;
  u_xlatb4.x = (u_xlatb4.x & u_xlatb5.x);
  u_xlatb4.y = (u_xlatb4.y & u_xlatb5.y);
  u_xlatb4.z = (u_xlatb4.z & u_xlatb5.z);
  u_xlatb42 = (u_xlatb4.y & u_xlatb4.x);
  u_xlatb42 = (u_xlatb4.z & u_xlatb42);
  u_xlat4 = (u_xlat3 + -(x_105.x_MinEntryPos_Noise.xyz));
  u_xlati4 = vec3i(u_xlat4);
  u_xlati5 = vec3i(x_105.x_IndicesDim_IndexChunkSize.xyw);
  u_xlati43 = (u_xlati5.y * u_xlati5.x);
  u_xlati45 = ((u_xlati4.y * u_xlati5.x) + u_xlati4.x);
  u_xlati43 = ((u_xlati4.z * u_xlati43) + u_xlati45);
  u_xlatu4 = vec4u(vec3u(x_237.x_APVResCellIndices_buf[u_xlati43].value[0i], x_237.x_APVResCellIndices_buf[u_xlati43].value[1i], x_237.x_APVResCellIndices_buf[u_xlati43].value[2i]).xyz, u_xlatu4.w);
  u_xlatb43 = (bitcast<i32>(u_xlatu4.x) != -1i);
  u_xlatu45 = (u_xlatu4.x >> 29u);
  u_xlat45 = f32(u_xlatu45);
  u_xlat45 = (u_xlat45 * 1.58496248722076416016f);
  u_xlat45 = exp2(u_xlat45);
  u_xlatu6.w = bitcast<u32>(i32(u_xlat45));
  u_xlatu6 = vec4u(((u_xlatu4.xyz & vec3u(536870911u, 1023u, 1023u))).xyz, u_xlatu6.w);
  param = u_xlatu4.y;
  param_1 = 10i;
  param_2 = 10i;
  let x_290 = uint_bitfieldExtract_u1_i1_i1_(&(param), &(param_1), &(param_2));
  param_3 = u_xlatu4.z;
  param_4 = 10i;
  param_5 = 10i;
  let x_296 = uint_bitfieldExtract_u1_i1_i1_(&(param_3), &(param_4), &(param_5));
  param_6 = u_xlatu4.z;
  param_7 = 20i;
  param_8 = 10i;
  let x_303 = uint_bitfieldExtract_u1_i1_i1_(&(param_6), &(param_7), &(param_8));
  param_9 = u_xlatu4.y;
  param_10 = 20i;
  param_11 = 10i;
  let x_309 = uint_bitfieldExtract_u1_i1_i1_(&(param_9), &(param_10), &(param_11));
  u_xlatu4 = vec4u(x_290, x_296, x_303, x_309);
  u_xlatu7.x = u_xlatu6.y;
  u_xlatu7 = vec4u(u_xlatu7.x, u_xlatu4.xw.xy, u_xlatu7.w);
  if (u_xlatb43) {
    x_323 = u_xlatu7.xyz;
  } else {
    x_323 = x_329;
  }
  u_xlatu5 = vec4u(x_323.xy, u_xlatu5.z, x_323.z);
  u_xlatu4.x = u_xlatu6.z;
  if (u_xlatb43) {
    x_337 = u_xlatu4.xyz;
  } else {
    x_337 = x_329;
  }
  u_xlatu4 = vec4u(x_337.xyz, u_xlatu4.w);
  if (u_xlatb43) {
    x_348 = u_xlatu6.xw;
  } else {
    x_348 = vec2u(4294967295u);
  }
  u_xlatu6 = vec4u(x_348.xy, u_xlatu6.zw);
  u_xlatb42 = (u_xlatb42 & u_xlatb43);
  u_xlat3 = ((-(u_xlat3) * x_105.x_NormalizationClamp_IndirectionEntryDim_Padding.zzz) + u_xlat2.xyz);
  u_xlat43 = f32(bitcast<i32>(u_xlatu6.y));
  u_xlat43 = (u_xlat43 * x_105.x_Biases_CellInMinBrick_MinBrickSize.w);
  u_xlat3 = (u_xlat3 / vec3f(u_xlat43));
  u_xlat3 = floor(u_xlat3);
  u_xlati3 = vec3i(u_xlat3);
  u_xlatb20 = ((u_xlati3.xyzz >= bitcast<vec4i>(u_xlatu5.xyww))).xyz;
  u_xlatb7 = ((u_xlati3.xyzx < bitcast<vec4i>(u_xlatu4.xyzx))).xyz;
  u_xlatb20.x = (u_xlatb20.x & u_xlatb7.x);
  u_xlatb20.y = (u_xlatb20.y & u_xlatb7.y);
  u_xlatb20.z = (u_xlatb20.z & u_xlatb7.z);
  u_xlatb43 = (u_xlatb20.y & u_xlatb20.x);
  u_xlati7.x = bitcast<i32>(((select(0u, 1u, u_xlatb20.z) * 4294967295u) & (select(0u, 1u, u_xlatb43) * 4294967295u)));
  u_xlati4 = vec3i(((-(bitcast<vec2i>(u_xlatu5.xy)) + bitcast<vec2i>(u_xlatu4.xy))).xy, u_xlati4.z);
  u_xlati3 = (-(bitcast<vec3i>(u_xlatu5.xyw)) + u_xlati3);
  u_xlati43 = (u_xlati4.y * u_xlati4.x);
  u_xlati3.x = ((u_xlati3.x * u_xlati4.y) + u_xlati3.y);
  u_xlati43 = ((u_xlati3.z * u_xlati43) + u_xlati3.x);
  u_xlati7.y = ((bitcast<i32>(u_xlatu6.x) * u_xlati5.z) + u_xlati43);
  u_xlati3 = vec3i(bitcast<vec2i>(((select(vec2u(), vec2u(1u), vec2<bool>(u_xlatb42)) * vec2u(4294967295u)) & bitcast<vec2u>(u_xlati7))).xy, u_xlati3.z);
  u_xlatu42 = x_508.x_APVResIndex_buf[u_xlati3.y].value[0i];
  u_xlatu42 = select(4294967295u, u_xlatu42, (u_xlati3.x != 0i));
  u_xlatu43 = (u_xlatu42 >> 28u);
  u_xlat43 = f32(u_xlatu43);
  u_xlat43 = (u_xlat43 * 1.58496248722076416016f);
  u_xlat43 = exp2(u_xlat43);
  u_xlatu42 = (u_xlatu42 & 268435455u);
  u_xlat42 = f32(u_xlatu42);
  u_xlat3.x = (u_xlat42 * x_105.x_RcpPoolDim_Padding.w);
  u_xlat3.z = floor(u_xlat3.x);
  u_xlat45 = (x_105.x_PoolDim_CellInMeters.y * x_105.x_PoolDim_CellInMeters.x);
  u_xlat42 = ((-(u_xlat3.z) * u_xlat45) + u_xlat42);
  u_xlat45 = (u_xlat42 * x_105.x_RcpPoolDim_Padding.x);
  u_xlat3.y = floor(u_xlat45);
  u_xlat42 = ((-(u_xlat3.y) * x_105.x_PoolDim_CellInMeters.x) + u_xlat42);
  u_xlat3.x = floor(u_xlat42);
  u_xlat4 = (u_xlat2.xyz / x_105.x_Biases_CellInMinBrick_MinBrickSize.www);
  u_xlat4 = (u_xlat4 / vec3f(u_xlat43));
  u_xlat4 = fract(u_xlat4);
  u_xlat3 = (u_xlat3 + x_586);
  u_xlat3 = ((u_xlat4 * x_590) + u_xlat3);
  u_xlat3 = (u_xlat3 * x_105.x_RcpPoolDim_Padding.xyz);
  u_xlat42 = (u_xlat43 * x_105.x_Biases_CellInMinBrick_MinBrickSize.w);
  u_xlat4 = (vec3f(u_xlat42) * x_607);
  u_xlat5 = vec4f(((u_xlat2.xyz / u_xlat4.xxx)).xyz, u_xlat5.w);
  u_xlat6 = vec4f(fract(u_xlat5.xyz).xyz, u_xlat6.w);
  u_xlat5 = vec4f(((u_xlat5.xyz + -(u_xlat6.xyz))).xyz, u_xlat5.w);
  u_xlat6 = vec4f(u_xlat6.x, ((u_xlat4.xxx * u_xlat5.xyz)).xyz);
  u_xlat3 = ((u_xlat3 * x_105.x_PoolDim_CellInMeters.xyz) + vec3f(-0.5f));
  u_xlatu7 = vec4u(bitcast<vec3u>(vec3i(u_xlat3)).xyz, u_xlatu7.w);
  u_xlat3 = fract(u_xlat3);
  u_xlat8 = vec4f(((-(u_xlat3) + x_657)).xyz, u_xlat8.w);
  u_xlatu7.w = 0u;
  u_xlat43 = textureLoad(x_APVResValidity, bitcast<vec3i>(u_xlatu7.xyz), bitcast<i32>(u_xlatu7.w)).x;
  u_xlat43 = (u_xlat43 * 255.0f);
  u_xlatu43 = u32(u_xlat43);
  u_xlat45 = (u_xlat8.y * u_xlat8.x);
  u_xlat46 = (u_xlat8.z * u_xlat45);
  u_xlatu7 = (vec4u(u_xlatu43) & vec4u(1u, 2u, 4u, 8u));
  u_xlat6.x = f32(bitcast<i32>(u_xlatu7.x));
  u_xlat9 = vec4f((((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat0.xyz))).xyz, u_xlat9.w);
  u_xlat47 = dot(u_xlat9.xyz, u_xlat9.xyz);
  u_xlat47 = inverseSqrt(u_xlat47);
  u_xlat10 = vec4f(((vec3f(u_xlat47) * u_xlat9.xyz)).xyz, u_xlat10.w);
  u_xlat47 = dot(u_xlat10.xyz, u_xlat1.xyz);
  u_xlat47 = (u_xlat47 + -(x_105.x_LeakReductionParams.z));
  u_xlat47 = clamp(u_xlat47, 0.0f, 1.0f);
  u_xlat47 = (u_xlat6.x * u_xlat47);
  u_xlat8 = vec4f(((u_xlat3.xy * u_xlat8.yx)).xy, u_xlat8.zw);
  u_xlat10 = vec4f(((u_xlat8.zz * u_xlat8.xy)).xy, u_xlat10.zw);
  u_xlatu7 = vec4u(min(u_xlatu7.yzw, vec3u(1u)).xyz, u_xlatu7.w);
  u_xlat7 = vec4f(vec3f(bitcast<vec3i>(u_xlatu7.xyz)).xyz, u_xlat7.w);
  u_xlat11 = vec4f((((vec3f(u_xlat42) * vec3f(0.3333333432674407959f, 0.0f, 0.0f)) + u_xlat9.xyz)).xyz, u_xlat11.w);
  u_xlat50 = dot(u_xlat11.xyz, u_xlat11.xyz);
  u_xlat50 = inverseSqrt(u_xlat50);
  u_xlat11 = vec4f(((vec3f(u_xlat50) * u_xlat11.xyz)).xyz, u_xlat11.w);
  u_xlat50 = dot(u_xlat11.xyz, u_xlat1.xyz);
  u_xlat50 = (u_xlat50 + -(x_105.x_LeakReductionParams.z));
  u_xlat50 = clamp(u_xlat50, 0.0f, 1.0f);
  u_xlat50 = (u_xlat7.x * u_xlat50);
  u_xlat11.x = (u_xlat50 * u_xlat10.x);
  u_xlat46 = ((u_xlat46 * u_xlat47) + u_xlat11.x);
  let x_830 = ((vec3f(u_xlat42) * vec3f(0.0f, 0.3333333432674407959f, 0.0f)) + u_xlat9.xyz);
  u_xlat10 = vec4f(x_830.x, u_xlat10.y, x_830.yz);
  u_xlat47 = dot(u_xlat10.xzw, u_xlat10.xzw);
  u_xlat47 = inverseSqrt(u_xlat47);
  let x_844 = (vec3f(u_xlat47) * u_xlat10.xzw);
  u_xlat10 = vec4f(x_844.x, u_xlat10.y, x_844.yz);
  u_xlat47 = dot(u_xlat10.xzw, u_xlat1.xyz);
  u_xlat47 = (u_xlat47 + -(x_105.x_LeakReductionParams.z));
  u_xlat47 = clamp(u_xlat47, 0.0f, 1.0f);
  u_xlat47 = (u_xlat7.y * u_xlat47);
  u_xlat11.y = (u_xlat47 * u_xlat10.y);
  u_xlat46 = ((u_xlat10.y * u_xlat47) + u_xlat46);
  u_xlat47 = (u_xlat3.y * u_xlat3.x);
  u_xlat36 = (u_xlat8.z * u_xlat47);
  u_xlat10 = vec4f((((vec3f(u_xlat42) * x_607) + u_xlat9.xyz)).xyz, u_xlat10.w);
  u_xlat50 = dot(u_xlat10.xyz, u_xlat10.xyz);
  u_xlat50 = inverseSqrt(u_xlat50);
  u_xlat10 = vec4f(((vec3f(u_xlat50) * u_xlat10.xyz)).xyz, u_xlat10.w);
  u_xlat50 = dot(u_xlat10.xyz, u_xlat1.xyz);
  u_xlat50 = (u_xlat50 + -(x_105.x_LeakReductionParams.z));
  u_xlat50 = clamp(u_xlat50, 0.0f, 1.0f);
  u_xlat50 = (u_xlat7.z * u_xlat50);
  u_xlat11.z = (u_xlat50 * u_xlat36);
  u_xlat46 = ((u_xlat36 * u_xlat50) + u_xlat46);
  u_xlat45 = (u_xlat3.z * u_xlat45);
  u_xlatu10 = (vec4u(u_xlatu43) & vec4u(16u, 32u, 64u, 128u));
  u_xlatu10 = min(u_xlatu10, vec4u(1u));
  u_xlat10 = vec4f(bitcast<vec4i>(u_xlatu10.yxzw));
  u_xlat12 = ((vec3f(u_xlat42) * vec3f(0.0f, 0.0f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat43 = dot(u_xlat12, u_xlat12);
  u_xlat43 = inverseSqrt(u_xlat43);
  u_xlat12 = (vec3f(u_xlat43) * u_xlat12);
  u_xlat43 = dot(u_xlat12, u_xlat1.xyz);
  u_xlat43 = (u_xlat43 + -(x_105.x_LeakReductionParams.z));
  u_xlat43 = clamp(u_xlat43, 0.0f, 1.0f);
  u_xlat43 = (u_xlat10.y * u_xlat43);
  u_xlat12.x = (u_xlat43 * u_xlat45);
  u_xlat43 = ((u_xlat45 * u_xlat43) + u_xlat46);
  u_xlat8 = vec4f(((u_xlat3.zz * u_xlat8.xy)).xy, u_xlat8.zw);
  u_xlat13 = ((vec3f(u_xlat42) * vec3f(0.3333333432674407959f, 0.0f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat45 = dot(u_xlat13, u_xlat13);
  u_xlat45 = inverseSqrt(u_xlat45);
  u_xlat13 = (vec3f(u_xlat45) * u_xlat13);
  u_xlat45 = dot(u_xlat13, u_xlat1.xyz);
  u_xlat45 = (u_xlat45 + -(x_105.x_LeakReductionParams.z));
  u_xlat45 = clamp(u_xlat45, 0.0f, 1.0f);
  u_xlat45 = (u_xlat10.x * u_xlat45);
  u_xlat12.y = (u_xlat45 * u_xlat8.x);
  u_xlat43 = ((u_xlat8.x * u_xlat45) + u_xlat43);
  let x_1048 = ((vec3f(u_xlat42) * vec3f(0.0f, 0.3333333432674407959f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat8 = vec4f(x_1048.x, u_xlat8.y, x_1048.yz);
  u_xlat45 = dot(u_xlat8.xzw, u_xlat8.xzw);
  u_xlat45 = inverseSqrt(u_xlat45);
  let x_1062 = (vec3f(u_xlat45) * u_xlat8.xzw);
  u_xlat8 = vec4f(x_1062.x, u_xlat8.y, x_1062.yz);
  u_xlat45 = dot(u_xlat8.xzw, u_xlat1.xyz);
  u_xlat45 = (u_xlat45 + -(x_105.x_LeakReductionParams.z));
  u_xlat45 = clamp(u_xlat45, 0.0f, 1.0f);
  u_xlat45 = (u_xlat10.z * u_xlat45);
  u_xlat12.z = (u_xlat45 * u_xlat8.y);
  u_xlat43 = ((u_xlat8.y * u_xlat45) + u_xlat43);
  u_xlat45 = (u_xlat3.z * u_xlat47);
  u_xlat8 = vec4f((((vec3f(u_xlat42) * vec3f(0.3333333432674407959f)) + u_xlat9.xyz)).xyz, u_xlat8.w);
  u_xlat46 = dot(u_xlat8.xyz, u_xlat8.xyz);
  u_xlat46 = inverseSqrt(u_xlat46);
  u_xlat8 = vec4f(((vec3f(u_xlat46) * u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat46 = dot(u_xlat8.xyz, u_xlat1.xyz);
  u_xlat46 = (u_xlat46 + -(x_105.x_LeakReductionParams.z));
  u_xlat46 = clamp(u_xlat46, 0.0f, 1.0f);
  u_xlat46 = (u_xlat10.w * u_xlat46);
  u_xlat5.w = (u_xlat45 * u_xlat46);
  u_xlat43 = ((u_xlat45 * u_xlat46) + u_xlat43);
  u_xlat43 = max(u_xlat43, 0.00009999999747378752f);
  u_xlat43 = (1.0f / u_xlat43);
  u_xlat8 = vec4f(((vec3f(u_xlat43) * u_xlat11.xyz)).xyz, u_xlat8.w);
  u_xlat9 = vec4f(((vec3f(u_xlat43) * u_xlat12)).xyz, u_xlat9.w);
  u_xlat11 = vec4f((((u_xlat8.xxx * x_1164) + -(u_xlat3))).xyz, u_xlat11.w);
  let x_1177 = ((u_xlat8.yyy * vec3f(0.0f, 1.0f, 0.0f)) + u_xlat11.xyz);
  u_xlat8 = vec4f(x_1177.xy, u_xlat8.z, x_1177.z);
  u_xlat8 = vec4f((((u_xlat8.zzz * vec3f(1.0f, 1.0f, 0.0f)) + u_xlat8.xyw)).xyz, u_xlat8.w);
  u_xlat8 = vec4f((((u_xlat9.xxx * x_1191) + u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat8 = vec4f((((u_xlat9.yyy * vec3f(1.0f, 0.0f, 1.0f)) + u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat8 = vec4f((((u_xlat9.zzz * vec3f(0.0f, 1.0f, 1.0f)) + u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat8 = vec4f((((u_xlat5.www * vec3f(u_xlat43)) + u_xlat8.xyz)).xyz, u_xlat8.w);
  u_xlat3 = (u_xlat3 + u_xlat8.xyz);
  u_xlatb43 = !((x_105.x_LeakReductionParams.x == 0.0f));
  u_xlat8 = vec4f((((-(u_xlat5.xyz) * u_xlat4.xxx) + u_xlat2.xyz)).xyz, u_xlat8.w);
  u_xlat8 = vec4f(((u_xlat8.xyz / u_xlat4.xxx)).xyz, u_xlat8.w);
  if (u_xlatb43) {
    x_1253 = u_xlat3;
  } else {
    x_1253 = u_xlat8.xyz;
  }
  u_xlat3 = x_1253;
  u_xlatb43 = any(!((vec4f() == vec4f(in_COLOR0.z))));
  if (u_xlatb43) {
    u_xlat8 = (in_COLOR0.zzzz + vec4f(-0.20000000298023223877f, -0.30000001192092895508f, -0.40000000596046447754f, -0.5f));
    u_xlatb8 = (abs(u_xlat8) < vec4f(0.01999999955296516418f));
    u_xlat9 = vec4f(u_xlat9.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zyz)).xyz);
    u_xlat9.x = u_xlat7.y;
    u_xlat9 = select(u_xlat6, u_xlat9, vec4<bool>(u_xlatb8.x));
    u_xlat11 = vec4f(u_xlat11.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yyz)).xyz);
    u_xlat11.x = u_xlat7.z;
    u_xlat9 = select(u_xlat9, u_xlat11, vec4<bool>(u_xlatb8.y));
    u_xlat7 = vec4f(u_xlat7.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yzz)).xyz);
    u_xlat7 = select(u_xlat9, u_xlat7, vec4<bool>(u_xlatb8.z));
    u_xlat9 = vec4f(u_xlat9.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zzy)).xyz);
    u_xlat9.x = u_xlat10.y;
    u_xlat7 = select(u_xlat7, u_xlat9, vec4<bool>(u_xlatb8.w));
    u_xlat8 = vec4f(((in_COLOR0.zzz + vec3f(-0.60000002384185791016f, -0.69999998807907104492f, -0.80000001192092895508f))).xyz, u_xlat8.w);
    u_xlatb8 = vec4<bool>(((abs(u_xlat8.xyzx) < vec4f(0.01999999955296516418f, 0.01999999955296516418f, 0.01999999955296516418f, 0.0f))).xyz.xyz, u_xlatb8.w);
    u_xlat9 = vec4f(u_xlat9.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zyy)).xyz);
    u_xlat9.x = u_xlat10.z;
    u_xlat7 = select(u_xlat7, u_xlat9, vec4<bool>(u_xlatb8.x));
    u_xlat9 = vec4f(u_xlat9.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.xxx)).xyz);
    u_xlat9.x = u_xlat10.w;
    u_xlat7 = select(u_xlat7, u_xlat9, vec4<bool>(u_xlatb8.y));
    u_xlat10 = vec4f(u_xlat10.x, (((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yzy)).xyz);
    u_xlat7 = select(u_xlat7, u_xlat10, vec4<bool>(u_xlatb8.z));
    u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat7.yzw));
    u_xlat43 = dot(u_xlat18, u_xlat18);
    u_xlat43 = sqrt(u_xlat43);
    u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
    if (u_xlatb43) {
      u_xlat18 = (-(u_xlat3) + x_657);
      u_xlat43 = (u_xlat18.y * u_xlat18.x);
      u_xlat8.x = (u_xlat18.z * u_xlat43);
    } else {
      u_xlat18 = ((vec3f(u_xlat42) * vec3f(-0.3333333432674407959f, 0.0f, 0.0f)) + u_xlat7.yzw);
      u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
      u_xlat43 = dot(u_xlat18, u_xlat18);
      u_xlat43 = sqrt(u_xlat43);
      u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
      if (u_xlatb43) {
        u_xlat18 = vec3f(((-(u_xlat3.yz) + x_1494)).xy, u_xlat18.z);
        u_xlat43 = (u_xlat3.x * u_xlat18.x);
        u_xlat8.x = (u_xlat18.y * u_xlat43);
      } else {
        u_xlat18 = ((vec3f(u_xlat42) * vec3f(-0.3333333432674407959f, -0.3333333432674407959f, 0.0f)) + u_xlat7.yzw);
        u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
        u_xlat43 = dot(u_xlat18, u_xlat18);
        u_xlat43 = sqrt(u_xlat43);
        u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
        if (u_xlatb43) {
          u_xlat43 = (u_xlat3.y * u_xlat3.x);
          u_xlat45 = (-(u_xlat3.z) + 1.0f);
          u_xlat8.x = (u_xlat43 * u_xlat45);
        } else {
          u_xlat18 = ((vec3f(u_xlat42) * vec3f(0.0f, -0.3333333432674407959f, 0.0f)) + u_xlat7.yzw);
          u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
          u_xlat43 = dot(u_xlat18, u_xlat18);
          u_xlat43 = sqrt(u_xlat43);
          u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
          if (u_xlatb43) {
            u_xlat18 = vec3f(((-(u_xlat3.xz) + x_1494)).xy, u_xlat18.z);
            u_xlat43 = (u_xlat3.y * u_xlat18.x);
            u_xlat8.x = (u_xlat18.y * u_xlat43);
          } else {
            u_xlat18 = ((vec3f(u_xlat42) * vec3f(-0.3333333432674407959f, 0.0f, -0.3333333432674407959f)) + u_xlat7.yzw);
            u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
            u_xlat43 = dot(u_xlat18, u_xlat18);
            u_xlat43 = sqrt(u_xlat43);
            u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
            u_xlat18 = vec3f(((-(u_xlat3.yx) + x_1494)).xy, u_xlat18.z);
            u_xlat9 = vec4f(((u_xlat3.xy * u_xlat18.xy)).xy, u_xlat9.zw);
            u_xlat10 = vec4f((((vec3f(u_xlat42) * vec3f(0.0f, 0.0f, -0.3333333432674407959f)) + u_xlat7.yzw)).xyz, u_xlat10.w);
            u_xlat10 = vec4f((((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz))).xyz, u_xlat10.w);
            u_xlat45 = dot(u_xlat10.xyz, u_xlat10.xyz);
            u_xlat45 = sqrt(u_xlat45);
            u_xlatb45 = (u_xlat45 < 0.00009999999747378752f);
            u_xlat18.x = (u_xlat18.x * u_xlat18.y);
            u_xlat10 = vec4f((((vec3f(u_xlat42) * vec3f(-0.3333333432674407959f)) + u_xlat7.yzw)).xyz, u_xlat10.w);
            u_xlat10 = vec4f((((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz))).xyz, u_xlat10.w);
            u_xlat32 = dot(u_xlat10.xyz, u_xlat10.xyz);
            u_xlat32 = sqrt(u_xlat32);
            u_xlatb32 = (u_xlat32 < 0.00009999999747378752f);
            u_xlat18.z = (u_xlat3.y * u_xlat3.x);
            let x_1702 = (u_xlat3.zz * u_xlat18.xz);
            u_xlat18 = vec3f(x_1702.x, u_xlat18.y, x_1702.y);
            u_xlat10 = vec4f((((vec3f(u_xlat42) * vec3f(0.0f, -0.3333333432674407959f, -0.3333333432674407959f)) + u_xlat7.yzw)).xyz, u_xlat10.w);
            u_xlat5 = vec4f((((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz))).xyz, u_xlat5.w);
            u_xlat42 = dot(u_xlat5.xyz, u_xlat5.xyz);
            u_xlat42 = sqrt(u_xlat42);
            u_xlatb42 = (u_xlat42 < 0.00009999999747378752f);
            u_xlat5 = vec4f(((u_xlat3.zz * u_xlat9.xy)).xy, u_xlat5.zw);
            if (u_xlatb42) {
              x_1743 = u_xlat5.y;
            } else {
              x_1743 = 0.0f;
            }
            u_xlat42 = x_1743;
            if (u_xlatb32) {
              x_1751 = u_xlat18.z;
            } else {
              x_1751 = u_xlat42;
            }
            u_xlat42 = x_1751;
            if (u_xlatb45) {
              x_1760 = u_xlat18.x;
            } else {
              x_1760 = u_xlat42;
            }
            u_xlat42 = x_1760;
            if (u_xlatb43) {
              x_1769 = u_xlat5.x;
            } else {
              x_1769 = u_xlat42;
            }
            u_xlat8.x = x_1769;
          }
        }
      }
    }
    let x_1787 = vec3f(x_76.x_ProbeSize, x_76.x_ProbeSize, x_76.x_ProbeSize);
    u_xlat18 = (x_76.unity_MatrixInvV[1i].xyz * vec3f(x_1787.x, x_1787.y, x_1787.z));
    u_xlat18 = ((u_xlat18 * vec3f(0.6666666865348815918f)) + u_xlat7.yzw);
    u_xlat5 = vec4f(((u_xlat18 + x_1804.unity_ObjectToWorld[3i].xyz)).xyz, u_xlat5.w);
    u_xlat18 = (in_POSITION0.yyy * x_76.unity_MatrixInvV[1i].xyz);
    u_xlat18 = (u_xlat18 * x_586);
    u_xlat18 = ((in_POSITION0.xxx * -(x_76.unity_MatrixInvV[0i].xyz)) + u_xlat18);
    let x_1836 = vec3f(x_76.x_ProbeSize, x_76.x_ProbeSize, x_76.x_ProbeSize);
    u_xlat18 = (u_xlat18 * vec3f(x_1836.x, x_1836.y, x_1836.z));
    u_xlat9 = vec4f(((u_xlat18 * vec3f(20.0f))).xyz, u_xlat9.w);
    u_xlat5.w = x_1804.unity_ObjectToWorld[3i].w;
    u_xlat9.w = 0.0f;
    u_xlat5 = (u_xlat5 + u_xlat9);
    u_xlat8.y = u_xlat7.x;
  } else {
    u_xlatb42 = any(!((vec4f() == vec4f(in_COLOR0.y))));
    if (u_xlatb42) {
      u_xlat42 = dot(u_xlat1.xyz, u_xlat1.xyz);
      u_xlat42 = inverseSqrt(u_xlat42);
      u_xlat1 = vec4f(((vec3f(u_xlat42) * u_xlat1.xyz)).xyz, u_xlat1.w);
      u_xlatb42 = (0.89999997615814208984f < u_xlat1.y);
      u_xlat18 = select(x_1164, x_1191, vec3<bool>(u_xlatb42));
      u_xlat7 = vec4f(((u_xlat1.zxy * u_xlat18)).xyz, u_xlat7.w);
      u_xlat18 = ((u_xlat1.yzx * u_xlat18.yzx) + -(u_xlat7.xyz));
      u_xlat42 = dot(u_xlat18, u_xlat18);
      u_xlat42 = inverseSqrt(u_xlat42);
      u_xlat18 = (vec3f(u_xlat42) * u_xlat18);
      u_xlat7 = vec4f(((u_xlat1.yzx * u_xlat18.zxy)).xyz, u_xlat7.w);
      u_xlat7 = vec4f((((u_xlat18.yzx * u_xlat1.zxy) + -(u_xlat7.xyz))).xyz, u_xlat7.w);
      let x_1938 = vec3f(x_76.x_ProbeSize, x_76.x_ProbeSize, x_76.x_ProbeSize);
      u_xlat9 = vec4f(((in_POSITION0.xyz * vec3f(x_1938.x, x_1938.y, x_1938.z))).xyz, u_xlat9.w);
      u_xlat9 = vec4f(((u_xlat9.xyz * vec3f(5.0f))).xyz, u_xlat9.w);
      u_xlat7 = vec4f(((u_xlat7.xyz * u_xlat9.yyy)).xyz, u_xlat7.w);
      u_xlat18 = ((u_xlat18 * u_xlat9.xxx) + u_xlat7.xyz);
      u_xlat1 = vec4f((((u_xlat1.xyz * u_xlat9.zzz) + u_xlat18)).xyz, u_xlat1.w);
      u_xlat7 = (u_xlat1.yyyy * x_1804.unity_ObjectToWorld[1i]);
      u_xlat7 = ((x_1804.unity_ObjectToWorld[0i] * u_xlat1.xxxx) + u_xlat7);
      u_xlat1 = ((x_1804.unity_ObjectToWorld[2i] * u_xlat1.zzzz) + u_xlat7);
      u_xlat5 = (u_xlat1 + x_1804.unity_ObjectToWorld[3i]);
      u_xlat5 = vec4f(((u_xlat0.xyz + u_xlat5.xyz)).xyz, u_xlat5.w);
    } else {
      u_xlatb0 = any(!((vec4f() == vec4f(in_COLOR0.x))));
      if (u_xlatb0) {
        if ((x_76.x_ForceDebugNormalViewBias != 0i)) {
          let x_2030 = vec3f(x_76.x_ProbeSize, x_76.x_ProbeSize, x_76.x_ProbeSize);
          u_xlat0 = vec4f(((in_POSITION0.xyz * vec3f(x_2030.x, x_2030.y, x_2030.z))).xyz, u_xlat0.w);
          u_xlat0 = vec4f(((u_xlat0.xyz * vec3f(1.5f))).xyz, u_xlat0.w);
          u_xlat1 = (u_xlat0.yyyy * x_1804.unity_ObjectToWorld[1i]);
          u_xlat1 = ((x_1804.unity_ObjectToWorld[0i] * u_xlat0.xxxx) + u_xlat1);
          u_xlat0 = ((x_1804.unity_ObjectToWorld[2i] * u_xlat0.zzzz) + u_xlat1);
          u_xlat0 = (u_xlat0 + x_1804.unity_ObjectToWorld[3i]);
          u_xlat2.w = 0.0f;
          u_xlat5 = (u_xlat0 + u_xlat2);
        } else {
          gl_Position = vec4f();
          vs_COLOR0 = vec4f();
          phase0_Output0_3 = vec4f(0.0f, 0.0f, 0.0f, 1.0f);
          vs_TEXCOORD1 = vec3f();
          vs_TEXCOORD0 = phase0_Output0_3.xy;
          vs_TEXCOORD2 = phase0_Output0_3.zw;
          return;
        }
      } else {
        let x_2101 = vec3f(x_76.x_ProbeSize, x_76.x_ProbeSize, x_76.x_ProbeSize);
        u_xlat0 = vec4f(((in_POSITION0.xyz * vec3f(x_2101.x, x_2101.y, x_2101.z))).xyz, u_xlat0.w);
        u_xlat0 = vec4f(((u_xlat0.xyz * x_590)).xyz, u_xlat0.w);
        u_xlat1 = (u_xlat0.yyyy * x_1804.unity_ObjectToWorld[1i]);
        u_xlat1 = ((x_1804.unity_ObjectToWorld[0i] * u_xlat0.xxxx) + u_xlat1);
        u_xlat0 = ((x_1804.unity_ObjectToWorld[2i] * u_xlat0.zzzz) + u_xlat1);
        u_xlat0 = (u_xlat0 + x_1804.unity_ObjectToWorld[3i]);
        u_xlat1 = vec4f((((u_xlat3 * u_xlat4.xxx) + u_xlat6.yzw)).xyz, u_xlat1.w);
        u_xlat1.w = 0.0f;
        u_xlat5 = (u_xlat0 + u_xlat1);
      }
    }
    u_xlat8.x = 0.0f;
    u_xlat8.y = 1.0f;
  }
  u_xlat0 = (u_xlat5.yyyy * x_76.unity_MatrixVP[1i]);
  u_xlat0 = ((x_76.unity_MatrixVP[0i] * u_xlat5.xxxx) + u_xlat0);
  u_xlat0 = ((x_76.unity_MatrixVP[2i] * u_xlat5.zzzz) + u_xlat0);
  u_xlat0 = ((x_76.unity_MatrixVP[3i] * u_xlat5.wwww) + u_xlat0);
  u_xlat28 = (u_xlat0.z + 1.0f);
  u_xlat28 = ((u_xlat28 * 0.19999998807907104492f) + 0.60000002384185791016f);
  gl_Position.z = (u_xlat0.w * u_xlat28);
  u_xlat1.x = dot(in_NORMAL0, x_1804.unity_ObjectToWorld[0i].xyz);
  u_xlat1.y = dot(in_NORMAL0, x_1804.unity_ObjectToWorld[1i].xyz);
  u_xlat1.z = dot(in_NORMAL0, x_1804.unity_ObjectToWorld[2i].xyz);
  u_xlat28 = dot(u_xlat1.xyz, u_xlat1.xyz);
  u_xlat28 = inverseSqrt(u_xlat28);
  vs_TEXCOORD1 = (vec3f(u_xlat28) * u_xlat1.xyz);
  let x_2226 = u_xlat0.xyw;
  gl_Position = vec4f(x_2226.xy, gl_Position.z, x_2226.z);
  vs_COLOR0 = in_COLOR0;
  u_xlat8 = vec4f(u_xlat8.xy, in_TEXCOORD0.xy);
  phase0_Output0_3 = u_xlat8.zwxy;
  vs_TEXCOORD0 = phase0_Output0_3.xy;
  vs_TEXCOORD2 = phase0_Output0_3.zw;
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
  @location(0)
  vs_COLOR0_1 : vec4f,
  @location(1)
  vs_TEXCOORD0_1 : vec2f,
  @location(2)
  vs_TEXCOORD2_1 : vec2f,
}

@vertex
fn main(@location(2) in_COLOR0_param : vec4f, @location(0) in_POSITION0_param : vec4f, @location(1) in_NORMAL0_param : vec3f, @location(3) in_TEXCOORD0_param : vec2f) -> main_out {
  in_COLOR0 = in_COLOR0_param;
  in_POSITION0 = in_POSITION0_param;
  in_NORMAL0 = in_NORMAL0_param;
  in_TEXCOORD0 = in_TEXCOORD0_param;
  main_1();
  return main_out(gl_Position, vs_COLOR0, vs_TEXCOORD0, vs_TEXCOORD2);
}
