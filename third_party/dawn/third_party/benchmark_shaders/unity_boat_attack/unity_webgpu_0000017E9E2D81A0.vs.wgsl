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
  x_WorldSpaceCameraPos : vec3<f32>,
  /* @offset(16) */
  unity_MatrixInvV : mat4x4<f32>,
  /* @offset(80) */
  unity_MatrixVP : mat4x4<f32>,
  /* @offset(144) */
  x_ProbeSize : f32,
  /* @offset(148) */
  x_ForceDebugNormalViewBias : i32,
}

struct ShaderVariablesProbeVolumes {
  /* @offset(0) */
  x_PoolDim_CellInMeters : vec4<f32>,
  /* @offset(16) */
  x_RcpPoolDim_Padding : vec4<f32>,
  /* @offset(32) */
  x_MinEntryPos_Noise : vec4<f32>,
  /* @offset(48) */
  x_IndicesDim_IndexChunkSize : vec4<f32>,
  /* @offset(64) */
  x_Biases_CellInMinBrick_MinBrickSize : vec4<f32>,
  /* @offset(80) */
  x_LeakReductionParams : vec4<f32>,
  /* @offset(96) */
  x_Weight_MinLoadedCellInEntries : vec4<f32>,
  /* @offset(112) */
  x_MaxLoadedCellInEntries_FrameIndex : vec4<f32>,
  /* @offset(128) */
  x_NormalizationClamp_IndirectionEntryDim_Padding : vec4<f32>,
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

alias Arr_3 = array<vec4<f32>, 2u>;

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
  unity_LightIndices : Arr_3,
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

var<private> u_xlat0 : vec4<f32>;

@group(0) @binding(5) var<storage, read> x_38 : x_positionNormalBuffer;

var<private> u_xlat1 : vec4<f32>;

var<private> u_xlat2 : vec4<f32>;

@group(1) @binding(1) var<uniform> x_76 : VGlobals;

var<private> u_xlat42 : f32;

var<private> u_xlat3 : vec3<f32>;

@group(1) @binding(3) var<uniform> x_105 : ShaderVariablesProbeVolumes;

var<private> u_xlatb4 : vec3<bool>;

var<private> u_xlatb5 : vec3<bool>;

var<private> u_xlatb42 : bool;

var<private> u_xlat4 : vec3<f32>;

var<private> u_xlati4 : vec3<i32>;

var<private> u_xlati5 : vec3<i32>;

var<private> u_xlati43 : i32;

var<private> u_xlati45 : i32;

var<private> u_xlatu4 : vec4<u32>;

@group(0) @binding(3) var<storage, read> x_237 : x_APVResCellIndices;

var<private> u_xlatb43 : bool;

var<private> u_xlatu45 : u32;

var<private> u_xlat45 : f32;

var<private> u_xlatu6 : vec4<u32>;

var<private> u_xlatu7 : vec4<u32>;

var<private> u_xlatu5 : vec4<u32>;

var<private> u_xlat43 : f32;

var<private> u_xlati3 : vec3<i32>;

var<private> u_xlatb20 : vec3<bool>;

var<private> u_xlatb7 : vec3<bool>;

var<private> u_xlati7 : vec2<i32>;

var<private> u_xlatu42 : u32;

@group(0) @binding(2) var<storage, read> x_508 : x_APVResIndex;

var<private> u_xlatu43 : u32;

var<private> u_xlat5 : vec4<f32>;

var<private> u_xlat6 : vec4<f32>;

var<private> u_xlat8 : vec4<f32>;

@group(0) @binding(4) var x_APVResValidity : texture_3d<f32>;

var<private> u_xlat46 : f32;

var<private> u_xlat9 : vec4<f32>;

var<private> u_xlat47 : f32;

var<private> u_xlat10 : vec4<f32>;

var<private> u_xlat7 : vec4<f32>;

var<private> u_xlat11 : vec4<f32>;

var<private> u_xlat50 : f32;

var<private> u_xlat36 : f32;

var<private> u_xlatu10 : vec4<u32>;

var<private> u_xlat12 : vec3<f32>;

var<private> u_xlat13 : vec3<f32>;

var<private> in_COLOR0 : vec4<f32>;

var<private> u_xlatb8 : vec4<bool>;

var<private> u_xlat18 : vec3<f32>;

var<private> u_xlatb45 : bool;

var<private> u_xlat32 : f32;

var<private> u_xlatb32 : bool;

@group(1) @binding(2) var<uniform> x_1804 : UnityPerDraw;

var<private> in_POSITION0 : vec4<f32>;

var<private> u_xlatb0 : bool;

var<private> vs_COLOR0 : vec4<f32>;

var<private> phase0_Output0_3 : vec4<f32>;

var<private> vs_TEXCOORD1 : vec3<f32>;

var<private> vs_TEXCOORD0 : vec2<f32>;

var<private> vs_TEXCOORD2 : vec2<f32>;

var<private> u_xlat28 : f32;

var<private> in_NORMAL0 : vec3<f32>;

var<private> in_TEXCOORD0 : vec2<f32>;

var<private> gl_Position : vec4<f32>;

fn uint_bitfieldExtract_u1_i1_i1_(value : ptr<function, u32>, offset_1 : ptr<function, i32>, bits : ptr<function, i32>) -> u32 {
  let x_16 : u32 = *(value);
  let x_17 : i32 = *(offset_1);
  let x_21 : i32 = *(bits);
  return ((x_16 >> bitcast<u32>(x_17)) & ~((4294967295u << bitcast<u32>(x_21))));
}

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
  var x_323 : vec3<u32>;
  var x_337 : vec3<u32>;
  var x_348 : vec2<u32>;
  var x_1253 : vec3<f32>;
  var x_1743 : f32;
  var x_1751 : f32;
  var x_1760 : f32;
  var x_1769 : f32;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  let x_42 : u32 = x_38.x_positionNormalBuffer_buf[0i].value[0i];
  let x_46 : u32 = x_38.x_positionNormalBuffer_buf[0i].value[1i];
  let x_50 : u32 = x_38.x_positionNormalBuffer_buf[0i].value[2i];
  let x_53 : vec3<f32> = vec3<f32>(bitcast<f32>(x_42), bitcast<f32>(x_46), bitcast<f32>(x_50));
  let x_54 : vec4<f32> = u_xlat0;
  u_xlat0 = vec4<f32>(x_53.x, x_53.y, x_53.z, x_54.w);
  let x_58 : u32 = x_38.x_positionNormalBuffer_buf[1i].value[0i];
  let x_61 : u32 = x_38.x_positionNormalBuffer_buf[1i].value[1i];
  let x_64 : u32 = x_38.x_positionNormalBuffer_buf[1i].value[2i];
  let x_66 : vec3<f32> = vec3<f32>(bitcast<f32>(x_58), bitcast<f32>(x_61), bitcast<f32>(x_64));
  let x_67 : vec4<f32> = u_xlat1;
  u_xlat1 = vec4<f32>(x_66.x, x_66.y, x_66.z, x_67.w);
  let x_70 : vec4<f32> = u_xlat0;
  let x_79 : vec3<f32> = x_76.x_WorldSpaceCameraPos;
  let x_80 : vec3<f32> = (-(vec3<f32>(x_70.x, x_70.y, x_70.z)) + x_79);
  let x_81 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_80.x, x_80.y, x_80.z, x_81.w);
  let x_85 : vec4<f32> = u_xlat2;
  let x_87 : vec4<f32> = u_xlat2;
  u_xlat42 = dot(vec3<f32>(x_85.x, x_85.y, x_85.z), vec3<f32>(x_87.x, x_87.y, x_87.z));
  let x_90 : f32 = u_xlat42;
  u_xlat42 = inverseSqrt(x_90);
  let x_92 : f32 = u_xlat42;
  let x_94 : vec4<f32> = u_xlat2;
  let x_96 : vec3<f32> = (vec3<f32>(x_92, x_92, x_92) * vec3<f32>(x_94.x, x_94.y, x_94.z));
  let x_97 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_96.x, x_96.y, x_96.z, x_97.w);
  let x_101 : vec4<f32> = u_xlat1;
  let x_109 : vec4<f32> = x_105.x_Biases_CellInMinBrick_MinBrickSize;
  let x_112 : vec4<f32> = u_xlat0;
  u_xlat3 = ((vec3<f32>(x_101.x, x_101.y, x_101.z) * vec3<f32>(x_109.x, x_109.x, x_109.x)) + vec3<f32>(x_112.x, x_112.y, x_112.z));
  let x_115 : vec4<f32> = u_xlat2;
  let x_118 : vec4<f32> = x_105.x_Biases_CellInMinBrick_MinBrickSize;
  let x_121 : vec3<f32> = u_xlat3;
  let x_122 : vec3<f32> = ((vec3<f32>(x_115.x, x_115.y, x_115.z) * vec3<f32>(x_118.y, x_118.y, x_118.y)) + x_121);
  let x_123 : vec4<f32> = u_xlat2;
  u_xlat2 = vec4<f32>(x_122.x, x_122.y, x_122.z, x_123.w);
  let x_125 : vec4<f32> = u_xlat2;
  let x_129 : vec4<f32> = x_105.x_NormalizationClamp_IndirectionEntryDim_Padding;
  u_xlat3 = (vec3<f32>(x_125.x, x_125.y, x_125.z) / vec3<f32>(x_129.z, x_129.z, x_129.z));
  let x_132 : vec3<f32> = u_xlat3;
  u_xlat3 = floor(x_132);
  let x_138 : vec3<f32> = u_xlat3;
  let x_142 : vec4<f32> = x_105.x_Weight_MinLoadedCellInEntries;
  let x_145 : vec4<bool> = (vec4<f32>(x_138.x, x_138.y, x_138.z, x_138.x) >= vec4<f32>(x_142.y, x_142.z, x_142.w, x_142.y));
  u_xlatb4 = vec3<bool>(x_145.x, x_145.y, x_145.z);
  let x_150 : vec4<f32> = x_105.x_MaxLoadedCellInEntries_FrameIndex;
  let x_152 : vec3<f32> = u_xlat3;
  let x_154 : vec4<bool> = (vec4<f32>(x_150.x, x_150.y, x_150.z, x_150.x) >= vec4<f32>(x_152.x, x_152.y, x_152.z, x_152.x));
  u_xlatb5 = vec3<bool>(x_154.x, x_154.y, x_154.z);
  let x_159 : bool = u_xlatb4.x;
  let x_161 : bool = u_xlatb5.x;
  u_xlatb4.x = (x_159 & x_161);
  let x_166 : bool = u_xlatb4.y;
  let x_168 : bool = u_xlatb5.y;
  u_xlatb4.y = (x_166 & x_168);
  let x_173 : bool = u_xlatb4.z;
  let x_175 : bool = u_xlatb5.z;
  u_xlatb4.z = (x_173 & x_175);
  let x_180 : bool = u_xlatb4.y;
  let x_182 : bool = u_xlatb4.x;
  u_xlatb42 = (x_180 & x_182);
  let x_185 : bool = u_xlatb4.z;
  let x_186 : bool = u_xlatb42;
  u_xlatb42 = (x_185 & x_186);
  let x_189 : vec3<f32> = u_xlat3;
  let x_191 : vec4<f32> = x_105.x_MinEntryPos_Noise;
  u_xlat4 = (x_189 + -(vec3<f32>(x_191.x, x_191.y, x_191.z)));
  let x_198 : vec3<f32> = u_xlat4;
  u_xlati4 = vec3<i32>(x_198);
  let x_203 : vec4<f32> = x_105.x_IndicesDim_IndexChunkSize;
  u_xlati5 = vec3<i32>(vec3<f32>(x_203.x, x_203.y, x_203.w));
  let x_209 : i32 = u_xlati5.y;
  let x_211 : i32 = u_xlati5.x;
  u_xlati43 = (x_209 * x_211);
  let x_215 : i32 = u_xlati4.y;
  let x_217 : i32 = u_xlati5.x;
  let x_220 : i32 = u_xlati4.x;
  u_xlati45 = ((x_215 * x_217) + x_220);
  let x_223 : i32 = u_xlati4.z;
  let x_224 : i32 = u_xlati43;
  let x_226 : i32 = u_xlati45;
  u_xlati43 = ((x_223 * x_224) + x_226);
  let x_238 : i32 = u_xlati43;
  let x_240 : u32 = x_237.x_APVResCellIndices_buf[x_238].value[0i];
  let x_241 : i32 = u_xlati43;
  let x_243 : u32 = x_237.x_APVResCellIndices_buf[x_241].value[1i];
  let x_244 : i32 = u_xlati43;
  let x_246 : u32 = x_237.x_APVResCellIndices_buf[x_244].value[2i];
  let x_248 : vec3<u32> = vec3<u32>(x_240, x_243, x_246);
  let x_249 : vec4<u32> = u_xlatu4;
  u_xlatu4 = vec4<u32>(x_248.x, x_248.y, x_248.z, x_249.w);
  let x_254 : u32 = u_xlatu4.x;
  u_xlatb43 = (bitcast<i32>(x_254) != -1i);
  let x_260 : u32 = u_xlatu4.x;
  u_xlatu45 = (x_260 >> 29u);
  let x_264 : u32 = u_xlatu45;
  u_xlat45 = f32(x_264);
  let x_266 : f32 = u_xlat45;
  u_xlat45 = (x_266 * 1.58496248722076416016f);
  let x_269 : f32 = u_xlat45;
  u_xlat45 = exp2(x_269);
  let x_272 : f32 = u_xlat45;
  u_xlatu6.w = bitcast<u32>(i32(x_272));
  let x_276 : vec4<u32> = u_xlatu4;
  let x_281 : vec3<u32> = (vec3<u32>(x_276.x, x_276.y, x_276.z) & vec3<u32>(536870911u, 1023u, 1023u));
  let x_282 : vec4<u32> = u_xlatu6;
  u_xlatu6 = vec4<u32>(x_281.x, x_281.y, x_281.z, x_282.w);
  let x_287 : u32 = u_xlatu4.y;
  param = x_287;
  param_1 = 10i;
  param_2 = 10i;
  let x_290 : u32 = uint_bitfieldExtract_u1_i1_i1_(&(param), &(param_1), &(param_2));
  let x_293 : u32 = u_xlatu4.z;
  param_3 = x_293;
  param_4 = 10i;
  param_5 = 10i;
  let x_296 : u32 = uint_bitfieldExtract_u1_i1_i1_(&(param_3), &(param_4), &(param_5));
  let x_300 : u32 = u_xlatu4.z;
  param_6 = x_300;
  param_7 = 20i;
  param_8 = 10i;
  let x_303 : u32 = uint_bitfieldExtract_u1_i1_i1_(&(param_6), &(param_7), &(param_8));
  let x_306 : u32 = u_xlatu4.y;
  param_9 = x_306;
  param_10 = 20i;
  param_11 = 10i;
  let x_309 : u32 = uint_bitfieldExtract_u1_i1_i1_(&(param_9), &(param_10), &(param_11));
  u_xlatu4 = vec4<u32>(x_290, x_296, x_303, x_309);
  let x_313 : u32 = u_xlatu6.y;
  u_xlatu7.x = x_313;
  let x_316 : vec4<u32> = u_xlatu4;
  let x_317 : vec2<u32> = vec2<u32>(x_316.x, x_316.w);
  let x_318 : vec4<u32> = u_xlatu7;
  u_xlatu7 = vec4<u32>(x_318.x, x_317.x, x_317.y, x_318.w);
  let x_321 : bool = u_xlatb43;
  if (x_321) {
    let x_326 : vec4<u32> = u_xlatu7;
    x_323 = vec3<u32>(x_326.x, x_326.y, x_326.z);
  } else {
    x_323 = vec3<u32>(4294967295u, 4294967295u, 4294967295u);
  }
  let x_330 : vec3<u32> = x_323;
  let x_331 : vec4<u32> = u_xlatu5;
  u_xlatu5 = vec4<u32>(x_330.x, x_330.y, x_331.z, x_330.z);
  let x_334 : u32 = u_xlatu6.z;
  u_xlatu4.x = x_334;
  let x_336 : bool = u_xlatb43;
  if (x_336) {
    let x_340 : vec4<u32> = u_xlatu4;
    x_337 = vec3<u32>(x_340.x, x_340.y, x_340.z);
  } else {
    x_337 = vec3<u32>(4294967295u, 4294967295u, 4294967295u);
  }
  let x_343 : vec3<u32> = x_337;
  let x_344 : vec4<u32> = u_xlatu4;
  u_xlatu4 = vec4<u32>(x_343.x, x_343.y, x_343.z, x_344.w);
  let x_346 : bool = u_xlatb43;
  if (x_346) {
    let x_351 : vec4<u32> = u_xlatu6;
    x_348 = vec2<u32>(x_351.x, x_351.w);
  } else {
    x_348 = vec2<u32>(4294967295u, 4294967295u);
  }
  let x_355 : vec2<u32> = x_348;
  let x_356 : vec4<u32> = u_xlatu6;
  u_xlatu6 = vec4<u32>(x_355.x, x_355.y, x_356.z, x_356.w);
  let x_358 : bool = u_xlatb42;
  let x_359 : bool = u_xlatb43;
  u_xlatb42 = (x_358 & x_359);
  let x_361 : vec3<f32> = u_xlat3;
  let x_364 : vec4<f32> = x_105.x_NormalizationClamp_IndirectionEntryDim_Padding;
  let x_367 : vec4<f32> = u_xlat2;
  u_xlat3 = ((-(x_361) * vec3<f32>(x_364.z, x_364.z, x_364.z)) + vec3<f32>(x_367.x, x_367.y, x_367.z));
  let x_372 : u32 = u_xlatu6.y;
  u_xlat43 = f32(bitcast<i32>(x_372));
  let x_375 : f32 = u_xlat43;
  let x_378 : f32 = x_105.x_Biases_CellInMinBrick_MinBrickSize.w;
  u_xlat43 = (x_375 * x_378);
  let x_380 : vec3<f32> = u_xlat3;
  let x_381 : f32 = u_xlat43;
  u_xlat3 = (x_380 / vec3<f32>(x_381, x_381, x_381));
  let x_384 : vec3<f32> = u_xlat3;
  u_xlat3 = floor(x_384);
  let x_387 : vec3<f32> = u_xlat3;
  u_xlati3 = vec3<i32>(x_387);
  let x_391 : vec3<i32> = u_xlati3;
  let x_393 : vec4<u32> = u_xlatu5;
  let x_396 : vec4<bool> = (vec4<i32>(x_391.x, x_391.y, x_391.z, x_391.z) >= bitcast<vec4<i32>>(vec4<u32>(x_393.x, x_393.y, x_393.w, x_393.w)));
  u_xlatb20 = vec3<bool>(x_396.x, x_396.y, x_396.z);
  let x_399 : vec3<i32> = u_xlati3;
  let x_401 : vec4<u32> = u_xlatu4;
  let x_404 : vec4<bool> = (vec4<i32>(x_399.x, x_399.y, x_399.z, x_399.x) < bitcast<vec4<i32>>(vec4<u32>(x_401.x, x_401.y, x_401.z, x_401.x)));
  u_xlatb7 = vec3<bool>(x_404.x, x_404.y, x_404.z);
  let x_407 : bool = u_xlatb20.x;
  let x_409 : bool = u_xlatb7.x;
  u_xlatb20.x = (x_407 & x_409);
  let x_413 : bool = u_xlatb20.y;
  let x_415 : bool = u_xlatb7.y;
  u_xlatb20.y = (x_413 & x_415);
  let x_419 : bool = u_xlatb20.z;
  let x_421 : bool = u_xlatb7.z;
  u_xlatb20.z = (x_419 & x_421);
  let x_425 : bool = u_xlatb20.y;
  let x_427 : bool = u_xlatb20.x;
  u_xlatb43 = (x_425 & x_427);
  let x_433 : bool = u_xlatb20.z;
  let x_436 : bool = u_xlatb43;
  u_xlati7.x = bitcast<i32>(((select(0u, 1u, x_433) * 4294967295u) & (select(0u, 1u, x_436) * 4294967295u)));
  let x_442 : vec4<u32> = u_xlatu5;
  let x_446 : vec4<u32> = u_xlatu4;
  let x_449 : vec2<i32> = (-(bitcast<vec2<i32>>(vec2<u32>(x_442.x, x_442.y))) + bitcast<vec2<i32>>(vec2<u32>(x_446.x, x_446.y)));
  let x_450 : vec3<i32> = u_xlati4;
  u_xlati4 = vec3<i32>(x_449.x, x_449.y, x_450.z);
  let x_452 : vec4<u32> = u_xlatu5;
  let x_456 : vec3<i32> = u_xlati3;
  u_xlati3 = (-(bitcast<vec3<i32>>(vec3<u32>(x_452.x, x_452.y, x_452.w))) + x_456);
  let x_459 : i32 = u_xlati4.y;
  let x_461 : i32 = u_xlati4.x;
  u_xlati43 = (x_459 * x_461);
  let x_464 : i32 = u_xlati3.x;
  let x_466 : i32 = u_xlati4.y;
  let x_469 : i32 = u_xlati3.y;
  u_xlati3.x = ((x_464 * x_466) + x_469);
  let x_473 : i32 = u_xlati3.z;
  let x_474 : i32 = u_xlati43;
  let x_477 : i32 = u_xlati3.x;
  u_xlati43 = ((x_473 * x_474) + x_477);
  let x_480 : u32 = u_xlatu6.x;
  let x_483 : i32 = u_xlati5.z;
  let x_485 : i32 = u_xlati43;
  u_xlati7.y = ((bitcast<i32>(x_480) * x_483) + x_485);
  let x_488 : bool = u_xlatb42;
  let x_496 : vec2<i32> = u_xlati7;
  let x_499 : vec2<i32> = bitcast<vec2<i32>>(((select(vec2<u32>(0u, 0u), vec2<u32>(1u, 1u), vec2<bool>(x_488, x_488)) * vec2<u32>(4294967295u, 4294967295u)) & bitcast<vec2<u32>>(x_496)));
  let x_500 : vec3<i32> = u_xlati3;
  u_xlati3 = vec3<i32>(x_499.x, x_499.y, x_500.z);
  let x_510 : i32 = u_xlati3.y;
  let x_512 : u32 = x_508.x_APVResIndex_buf[x_510].value[0i];
  u_xlatu42 = x_512;
  let x_514 : i32 = u_xlati3.x;
  let x_516 : u32 = u_xlatu42;
  u_xlatu42 = select(4294967295u, x_516, (x_514 != 0i));
  let x_519 : u32 = u_xlatu42;
  u_xlatu43 = (x_519 >> 28u);
  let x_522 : u32 = u_xlatu43;
  u_xlat43 = f32(x_522);
  let x_524 : f32 = u_xlat43;
  u_xlat43 = (x_524 * 1.58496248722076416016f);
  let x_526 : f32 = u_xlat43;
  u_xlat43 = exp2(x_526);
  let x_528 : u32 = u_xlatu42;
  u_xlatu42 = (x_528 & 268435455u);
  let x_531 : u32 = u_xlatu42;
  u_xlat42 = f32(x_531);
  let x_533 : f32 = u_xlat42;
  let x_535 : f32 = x_105.x_RcpPoolDim_Padding.w;
  u_xlat3.x = (x_533 * x_535);
  let x_539 : f32 = u_xlat3.x;
  u_xlat3.z = floor(x_539);
  let x_543 : f32 = x_105.x_PoolDim_CellInMeters.y;
  let x_545 : f32 = x_105.x_PoolDim_CellInMeters.x;
  u_xlat45 = (x_543 * x_545);
  let x_548 : f32 = u_xlat3.z;
  let x_550 : f32 = u_xlat45;
  let x_552 : f32 = u_xlat42;
  u_xlat42 = ((-(x_548) * x_550) + x_552);
  let x_554 : f32 = u_xlat42;
  let x_556 : f32 = x_105.x_RcpPoolDim_Padding.x;
  u_xlat45 = (x_554 * x_556);
  let x_558 : f32 = u_xlat45;
  u_xlat3.y = floor(x_558);
  let x_562 : f32 = u_xlat3.y;
  let x_565 : f32 = x_105.x_PoolDim_CellInMeters.x;
  let x_567 : f32 = u_xlat42;
  u_xlat42 = ((-(x_562) * x_565) + x_567);
  let x_569 : f32 = u_xlat42;
  u_xlat3.x = floor(x_569);
  let x_572 : vec4<f32> = u_xlat2;
  let x_575 : vec4<f32> = x_105.x_Biases_CellInMinBrick_MinBrickSize;
  u_xlat4 = (vec3<f32>(x_572.x, x_572.y, x_572.z) / vec3<f32>(x_575.w, x_575.w, x_575.w));
  let x_578 : vec3<f32> = u_xlat4;
  let x_579 : f32 = u_xlat43;
  u_xlat4 = (x_578 / vec3<f32>(x_579, x_579, x_579));
  let x_582 : vec3<f32> = u_xlat4;
  u_xlat4 = fract(x_582);
  let x_584 : vec3<f32> = u_xlat3;
  u_xlat3 = (x_584 + vec3<f32>(0.5f, 0.5f, 0.5f));
  let x_588 : vec3<f32> = u_xlat4;
  let x_592 : vec3<f32> = u_xlat3;
  u_xlat3 = ((x_588 * vec3<f32>(3.0f, 3.0f, 3.0f)) + x_592);
  let x_594 : vec3<f32> = u_xlat3;
  let x_596 : vec4<f32> = x_105.x_RcpPoolDim_Padding;
  u_xlat3 = (x_594 * vec3<f32>(x_596.x, x_596.y, x_596.z));
  let x_599 : f32 = u_xlat43;
  let x_601 : f32 = x_105.x_Biases_CellInMinBrick_MinBrickSize.w;
  u_xlat42 = (x_599 * x_601);
  let x_603 : f32 = u_xlat42;
  u_xlat4 = (vec3<f32>(x_603, x_603, x_603) * vec3<f32>(0.3333333432674407959f, 0.3333333432674407959f, 0.0f));
  let x_610 : vec4<f32> = u_xlat2;
  let x_612 : vec3<f32> = u_xlat4;
  let x_614 : vec3<f32> = (vec3<f32>(x_610.x, x_610.y, x_610.z) / vec3<f32>(x_612.x, x_612.x, x_612.x));
  let x_615 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_614.x, x_614.y, x_614.z, x_615.w);
  let x_618 : vec4<f32> = u_xlat5;
  let x_620 : vec3<f32> = fract(vec3<f32>(x_618.x, x_618.y, x_618.z));
  let x_621 : vec4<f32> = u_xlat6;
  u_xlat6 = vec4<f32>(x_620.x, x_620.y, x_620.z, x_621.w);
  let x_623 : vec4<f32> = u_xlat5;
  let x_625 : vec4<f32> = u_xlat6;
  let x_628 : vec3<f32> = (vec3<f32>(x_623.x, x_623.y, x_623.z) + -(vec3<f32>(x_625.x, x_625.y, x_625.z)));
  let x_629 : vec4<f32> = u_xlat5;
  u_xlat5 = vec4<f32>(x_628.x, x_628.y, x_628.z, x_629.w);
  let x_631 : vec3<f32> = u_xlat4;
  let x_633 : vec4<f32> = u_xlat5;
  let x_635 : vec3<f32> = (vec3<f32>(x_631.x, x_631.x, x_631.x) * vec3<f32>(x_633.x, x_633.y, x_633.z));
  let x_636 : vec4<f32> = u_xlat6;
  u_xlat6 = vec4<f32>(x_636.x, x_635.x, x_635.y, x_635.z);
  let x_638 : vec3<f32> = u_xlat3;
  let x_640 : vec4<f32> = x_105.x_PoolDim_CellInMeters;
  u_xlat3 = ((x_638 * vec3<f32>(x_640.x, x_640.y, x_640.z)) + vec3<f32>(-0.5f, -0.5f, -0.5f));
  let x_646 : vec3<f32> = u_xlat3;
  let x_648 : vec3<u32> = bitcast<vec3<u32>>(vec3<i32>(x_646));
  let x_649 : vec4<u32> = u_xlatu7;
  u_xlatu7 = vec4<u32>(x_648.x, x_648.y, x_648.z, x_649.w);
  let x_651 : vec3<f32> = u_xlat3;
  u_xlat3 = fract(x_651);
  let x_654 : vec3<f32> = u_xlat3;
  let x_658 : vec3<f32> = (-(x_654) + vec3<f32>(1.0f, 1.0f, 1.0f));
  let x_659 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_658.x, x_658.y, x_658.z, x_659.w);
  u_xlatu7.w = 0u;
  let x_666 : vec4<u32> = u_xlatu7;
  let x_670 : u32 = u_xlatu7.w;
  let x_672 : vec4<f32> = textureLoad(x_APVResValidity, bitcast<vec3<i32>>(vec3<u32>(x_666.x, x_666.y, x_666.z)), bitcast<i32>(x_670));
  u_xlat43 = x_672.x;
  let x_674 : f32 = u_xlat43;
  u_xlat43 = (x_674 * 255.0f);
  let x_677 : f32 = u_xlat43;
  u_xlatu43 = u32(x_677);
  let x_680 : f32 = u_xlat8.y;
  let x_682 : f32 = u_xlat8.x;
  u_xlat45 = (x_680 * x_682);
  let x_686 : f32 = u_xlat8.z;
  let x_687 : f32 = u_xlat45;
  u_xlat46 = (x_686 * x_687);
  let x_689 : u32 = u_xlatu43;
  u_xlatu7 = (vec4<u32>(x_689, x_689, x_689, x_689) & vec4<u32>(1u, 2u, 4u, 8u));
  let x_695 : u32 = u_xlatu7.x;
  u_xlat6.x = f32(bitcast<i32>(x_695));
  let x_700 : vec4<f32> = u_xlat5;
  let x_702 : vec3<f32> = u_xlat4;
  let x_705 : vec4<f32> = u_xlat0;
  let x_708 : vec3<f32> = ((vec3<f32>(x_700.x, x_700.y, x_700.z) * vec3<f32>(x_702.x, x_702.x, x_702.x)) + -(vec3<f32>(x_705.x, x_705.y, x_705.z)));
  let x_709 : vec4<f32> = u_xlat9;
  u_xlat9 = vec4<f32>(x_708.x, x_708.y, x_708.z, x_709.w);
  let x_712 : vec4<f32> = u_xlat9;
  let x_714 : vec4<f32> = u_xlat9;
  u_xlat47 = dot(vec3<f32>(x_712.x, x_712.y, x_712.z), vec3<f32>(x_714.x, x_714.y, x_714.z));
  let x_717 : f32 = u_xlat47;
  u_xlat47 = inverseSqrt(x_717);
  let x_720 : f32 = u_xlat47;
  let x_722 : vec4<f32> = u_xlat9;
  let x_724 : vec3<f32> = (vec3<f32>(x_720, x_720, x_720) * vec3<f32>(x_722.x, x_722.y, x_722.z));
  let x_725 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_724.x, x_724.y, x_724.z, x_725.w);
  let x_727 : vec4<f32> = u_xlat10;
  let x_729 : vec4<f32> = u_xlat1;
  u_xlat47 = dot(vec3<f32>(x_727.x, x_727.y, x_727.z), vec3<f32>(x_729.x, x_729.y, x_729.z));
  let x_732 : f32 = u_xlat47;
  let x_735 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat47 = (x_732 + -(x_735));
  let x_738 : f32 = u_xlat47;
  u_xlat47 = clamp(x_738, 0.0f, 1.0f);
  let x_741 : f32 = u_xlat6.x;
  let x_742 : f32 = u_xlat47;
  u_xlat47 = (x_741 * x_742);
  let x_745 : vec3<f32> = u_xlat3;
  let x_747 : vec4<f32> = u_xlat8;
  let x_749 : vec2<f32> = (vec2<f32>(x_745.x, x_745.y) * vec2<f32>(x_747.y, x_747.x));
  let x_750 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_749.x, x_749.y, x_750.z, x_750.w);
  let x_752 : vec4<f32> = u_xlat8;
  let x_754 : vec4<f32> = u_xlat8;
  let x_756 : vec2<f32> = (vec2<f32>(x_752.z, x_752.z) * vec2<f32>(x_754.x, x_754.y));
  let x_757 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_756.x, x_756.y, x_757.z, x_757.w);
  let x_759 : vec4<u32> = u_xlatu7;
  let x_762 : vec3<u32> = min(vec3<u32>(x_759.y, x_759.z, x_759.w), vec3<u32>(1u, 1u, 1u));
  let x_763 : vec4<u32> = u_xlatu7;
  u_xlatu7 = vec4<u32>(x_762.x, x_762.y, x_762.z, x_763.w);
  let x_766 : vec4<u32> = u_xlatu7;
  let x_769 : vec3<f32> = vec3<f32>(bitcast<vec3<i32>>(vec3<u32>(x_766.x, x_766.y, x_766.z)));
  let x_770 : vec4<f32> = u_xlat7;
  u_xlat7 = vec4<f32>(x_769.x, x_769.y, x_769.z, x_770.w);
  let x_773 : f32 = u_xlat42;
  let x_777 : vec4<f32> = u_xlat9;
  let x_779 : vec3<f32> = ((vec3<f32>(x_773, x_773, x_773) * vec3<f32>(0.3333333432674407959f, 0.0f, 0.0f)) + vec3<f32>(x_777.x, x_777.y, x_777.z));
  let x_780 : vec4<f32> = u_xlat11;
  u_xlat11 = vec4<f32>(x_779.x, x_779.y, x_779.z, x_780.w);
  let x_783 : vec4<f32> = u_xlat11;
  let x_785 : vec4<f32> = u_xlat11;
  u_xlat50 = dot(vec3<f32>(x_783.x, x_783.y, x_783.z), vec3<f32>(x_785.x, x_785.y, x_785.z));
  let x_788 : f32 = u_xlat50;
  u_xlat50 = inverseSqrt(x_788);
  let x_790 : f32 = u_xlat50;
  let x_792 : vec4<f32> = u_xlat11;
  let x_794 : vec3<f32> = (vec3<f32>(x_790, x_790, x_790) * vec3<f32>(x_792.x, x_792.y, x_792.z));
  let x_795 : vec4<f32> = u_xlat11;
  u_xlat11 = vec4<f32>(x_794.x, x_794.y, x_794.z, x_795.w);
  let x_797 : vec4<f32> = u_xlat11;
  let x_799 : vec4<f32> = u_xlat1;
  u_xlat50 = dot(vec3<f32>(x_797.x, x_797.y, x_797.z), vec3<f32>(x_799.x, x_799.y, x_799.z));
  let x_802 : f32 = u_xlat50;
  let x_804 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat50 = (x_802 + -(x_804));
  let x_807 : f32 = u_xlat50;
  u_xlat50 = clamp(x_807, 0.0f, 1.0f);
  let x_810 : f32 = u_xlat7.x;
  let x_811 : f32 = u_xlat50;
  u_xlat50 = (x_810 * x_811);
  let x_813 : f32 = u_xlat50;
  let x_815 : f32 = u_xlat10.x;
  u_xlat11.x = (x_813 * x_815);
  let x_818 : f32 = u_xlat46;
  let x_819 : f32 = u_xlat47;
  let x_822 : f32 = u_xlat11.x;
  u_xlat46 = ((x_818 * x_819) + x_822);
  let x_824 : f32 = u_xlat42;
  let x_828 : vec4<f32> = u_xlat9;
  let x_830 : vec3<f32> = ((vec3<f32>(x_824, x_824, x_824) * vec3<f32>(0.0f, 0.3333333432674407959f, 0.0f)) + vec3<f32>(x_828.x, x_828.y, x_828.z));
  let x_831 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_830.x, x_831.y, x_830.y, x_830.z);
  let x_833 : vec4<f32> = u_xlat10;
  let x_835 : vec4<f32> = u_xlat10;
  u_xlat47 = dot(vec3<f32>(x_833.x, x_833.z, x_833.w), vec3<f32>(x_835.x, x_835.z, x_835.w));
  let x_838 : f32 = u_xlat47;
  u_xlat47 = inverseSqrt(x_838);
  let x_840 : f32 = u_xlat47;
  let x_842 : vec4<f32> = u_xlat10;
  let x_844 : vec3<f32> = (vec3<f32>(x_840, x_840, x_840) * vec3<f32>(x_842.x, x_842.z, x_842.w));
  let x_845 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_844.x, x_845.y, x_844.y, x_844.z);
  let x_847 : vec4<f32> = u_xlat10;
  let x_849 : vec4<f32> = u_xlat1;
  u_xlat47 = dot(vec3<f32>(x_847.x, x_847.z, x_847.w), vec3<f32>(x_849.x, x_849.y, x_849.z));
  let x_852 : f32 = u_xlat47;
  let x_854 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat47 = (x_852 + -(x_854));
  let x_857 : f32 = u_xlat47;
  u_xlat47 = clamp(x_857, 0.0f, 1.0f);
  let x_860 : f32 = u_xlat7.y;
  let x_861 : f32 = u_xlat47;
  u_xlat47 = (x_860 * x_861);
  let x_863 : f32 = u_xlat47;
  let x_865 : f32 = u_xlat10.y;
  u_xlat11.y = (x_863 * x_865);
  let x_869 : f32 = u_xlat10.y;
  let x_870 : f32 = u_xlat47;
  let x_872 : f32 = u_xlat46;
  u_xlat46 = ((x_869 * x_870) + x_872);
  let x_875 : f32 = u_xlat3.y;
  let x_877 : f32 = u_xlat3.x;
  u_xlat47 = (x_875 * x_877);
  let x_881 : f32 = u_xlat8.z;
  let x_882 : f32 = u_xlat47;
  u_xlat36 = (x_881 * x_882);
  let x_884 : f32 = u_xlat42;
  let x_887 : vec4<f32> = u_xlat9;
  let x_889 : vec3<f32> = ((vec3<f32>(x_884, x_884, x_884) * vec3<f32>(0.3333333432674407959f, 0.3333333432674407959f, 0.0f)) + vec3<f32>(x_887.x, x_887.y, x_887.z));
  let x_890 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_889.x, x_889.y, x_889.z, x_890.w);
  let x_892 : vec4<f32> = u_xlat10;
  let x_894 : vec4<f32> = u_xlat10;
  u_xlat50 = dot(vec3<f32>(x_892.x, x_892.y, x_892.z), vec3<f32>(x_894.x, x_894.y, x_894.z));
  let x_897 : f32 = u_xlat50;
  u_xlat50 = inverseSqrt(x_897);
  let x_899 : f32 = u_xlat50;
  let x_901 : vec4<f32> = u_xlat10;
  let x_903 : vec3<f32> = (vec3<f32>(x_899, x_899, x_899) * vec3<f32>(x_901.x, x_901.y, x_901.z));
  let x_904 : vec4<f32> = u_xlat10;
  u_xlat10 = vec4<f32>(x_903.x, x_903.y, x_903.z, x_904.w);
  let x_906 : vec4<f32> = u_xlat10;
  let x_908 : vec4<f32> = u_xlat1;
  u_xlat50 = dot(vec3<f32>(x_906.x, x_906.y, x_906.z), vec3<f32>(x_908.x, x_908.y, x_908.z));
  let x_911 : f32 = u_xlat50;
  let x_913 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat50 = (x_911 + -(x_913));
  let x_916 : f32 = u_xlat50;
  u_xlat50 = clamp(x_916, 0.0f, 1.0f);
  let x_919 : f32 = u_xlat7.z;
  let x_920 : f32 = u_xlat50;
  u_xlat50 = (x_919 * x_920);
  let x_922 : f32 = u_xlat50;
  let x_923 : f32 = u_xlat36;
  u_xlat11.z = (x_922 * x_923);
  let x_926 : f32 = u_xlat36;
  let x_927 : f32 = u_xlat50;
  let x_929 : f32 = u_xlat46;
  u_xlat46 = ((x_926 * x_927) + x_929);
  let x_932 : f32 = u_xlat3.z;
  let x_933 : f32 = u_xlat45;
  u_xlat45 = (x_932 * x_933);
  let x_936 : u32 = u_xlatu43;
  u_xlatu10 = (vec4<u32>(x_936, x_936, x_936, x_936) & vec4<u32>(16u, 32u, 64u, 128u));
  let x_944 : vec4<u32> = u_xlatu10;
  u_xlatu10 = min(x_944, vec4<u32>(1u, 1u, 1u, 1u));
  let x_947 : vec4<u32> = u_xlatu10;
  u_xlat10 = vec4<f32>(bitcast<vec4<i32>>(vec4<u32>(x_947.y, x_947.x, x_947.z, x_947.w)));
  let x_952 : f32 = u_xlat42;
  let x_956 : vec4<f32> = u_xlat9;
  u_xlat12 = ((vec3<f32>(x_952, x_952, x_952) * vec3<f32>(0.0f, 0.0f, 0.3333333432674407959f)) + vec3<f32>(x_956.x, x_956.y, x_956.z));
  let x_959 : vec3<f32> = u_xlat12;
  let x_960 : vec3<f32> = u_xlat12;
  u_xlat43 = dot(x_959, x_960);
  let x_962 : f32 = u_xlat43;
  u_xlat43 = inverseSqrt(x_962);
  let x_964 : f32 = u_xlat43;
  let x_966 : vec3<f32> = u_xlat12;
  u_xlat12 = (vec3<f32>(x_964, x_964, x_964) * x_966);
  let x_968 : vec3<f32> = u_xlat12;
  let x_969 : vec4<f32> = u_xlat1;
  u_xlat43 = dot(x_968, vec3<f32>(x_969.x, x_969.y, x_969.z));
  let x_972 : f32 = u_xlat43;
  let x_974 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat43 = (x_972 + -(x_974));
  let x_977 : f32 = u_xlat43;
  u_xlat43 = clamp(x_977, 0.0f, 1.0f);
  let x_980 : f32 = u_xlat10.y;
  let x_981 : f32 = u_xlat43;
  u_xlat43 = (x_980 * x_981);
  let x_983 : f32 = u_xlat43;
  let x_984 : f32 = u_xlat45;
  u_xlat12.x = (x_983 * x_984);
  let x_987 : f32 = u_xlat45;
  let x_988 : f32 = u_xlat43;
  let x_990 : f32 = u_xlat46;
  u_xlat43 = ((x_987 * x_988) + x_990);
  let x_992 : vec3<f32> = u_xlat3;
  let x_994 : vec4<f32> = u_xlat8;
  let x_996 : vec2<f32> = (vec2<f32>(x_992.z, x_992.z) * vec2<f32>(x_994.x, x_994.y));
  let x_997 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_996.x, x_996.y, x_997.z, x_997.w);
  let x_1000 : f32 = u_xlat42;
  let x_1004 : vec4<f32> = u_xlat9;
  u_xlat13 = ((vec3<f32>(x_1000, x_1000, x_1000) * vec3<f32>(0.3333333432674407959f, 0.0f, 0.3333333432674407959f)) + vec3<f32>(x_1004.x, x_1004.y, x_1004.z));
  let x_1007 : vec3<f32> = u_xlat13;
  let x_1008 : vec3<f32> = u_xlat13;
  u_xlat45 = dot(x_1007, x_1008);
  let x_1010 : f32 = u_xlat45;
  u_xlat45 = inverseSqrt(x_1010);
  let x_1012 : f32 = u_xlat45;
  let x_1014 : vec3<f32> = u_xlat13;
  u_xlat13 = (vec3<f32>(x_1012, x_1012, x_1012) * x_1014);
  let x_1016 : vec3<f32> = u_xlat13;
  let x_1017 : vec4<f32> = u_xlat1;
  u_xlat45 = dot(x_1016, vec3<f32>(x_1017.x, x_1017.y, x_1017.z));
  let x_1020 : f32 = u_xlat45;
  let x_1022 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat45 = (x_1020 + -(x_1022));
  let x_1025 : f32 = u_xlat45;
  u_xlat45 = clamp(x_1025, 0.0f, 1.0f);
  let x_1028 : f32 = u_xlat10.x;
  let x_1029 : f32 = u_xlat45;
  u_xlat45 = (x_1028 * x_1029);
  let x_1031 : f32 = u_xlat45;
  let x_1033 : f32 = u_xlat8.x;
  u_xlat12.y = (x_1031 * x_1033);
  let x_1037 : f32 = u_xlat8.x;
  let x_1038 : f32 = u_xlat45;
  let x_1040 : f32 = u_xlat43;
  u_xlat43 = ((x_1037 * x_1038) + x_1040);
  let x_1042 : f32 = u_xlat42;
  let x_1046 : vec4<f32> = u_xlat9;
  let x_1048 : vec3<f32> = ((vec3<f32>(x_1042, x_1042, x_1042) * vec3<f32>(0.0f, 0.3333333432674407959f, 0.3333333432674407959f)) + vec3<f32>(x_1046.x, x_1046.y, x_1046.z));
  let x_1049 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1048.x, x_1049.y, x_1048.y, x_1048.z);
  let x_1051 : vec4<f32> = u_xlat8;
  let x_1053 : vec4<f32> = u_xlat8;
  u_xlat45 = dot(vec3<f32>(x_1051.x, x_1051.z, x_1051.w), vec3<f32>(x_1053.x, x_1053.z, x_1053.w));
  let x_1056 : f32 = u_xlat45;
  u_xlat45 = inverseSqrt(x_1056);
  let x_1058 : f32 = u_xlat45;
  let x_1060 : vec4<f32> = u_xlat8;
  let x_1062 : vec3<f32> = (vec3<f32>(x_1058, x_1058, x_1058) * vec3<f32>(x_1060.x, x_1060.z, x_1060.w));
  let x_1063 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1062.x, x_1063.y, x_1062.y, x_1062.z);
  let x_1065 : vec4<f32> = u_xlat8;
  let x_1067 : vec4<f32> = u_xlat1;
  u_xlat45 = dot(vec3<f32>(x_1065.x, x_1065.z, x_1065.w), vec3<f32>(x_1067.x, x_1067.y, x_1067.z));
  let x_1070 : f32 = u_xlat45;
  let x_1072 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat45 = (x_1070 + -(x_1072));
  let x_1075 : f32 = u_xlat45;
  u_xlat45 = clamp(x_1075, 0.0f, 1.0f);
  let x_1078 : f32 = u_xlat10.z;
  let x_1079 : f32 = u_xlat45;
  u_xlat45 = (x_1078 * x_1079);
  let x_1081 : f32 = u_xlat45;
  let x_1083 : f32 = u_xlat8.y;
  u_xlat12.z = (x_1081 * x_1083);
  let x_1087 : f32 = u_xlat8.y;
  let x_1088 : f32 = u_xlat45;
  let x_1090 : f32 = u_xlat43;
  u_xlat43 = ((x_1087 * x_1088) + x_1090);
  let x_1093 : f32 = u_xlat3.z;
  let x_1094 : f32 = u_xlat47;
  u_xlat45 = (x_1093 * x_1094);
  let x_1096 : f32 = u_xlat42;
  let x_1100 : vec4<f32> = u_xlat9;
  let x_1102 : vec3<f32> = ((vec3<f32>(x_1096, x_1096, x_1096) * vec3<f32>(0.3333333432674407959f, 0.3333333432674407959f, 0.3333333432674407959f)) + vec3<f32>(x_1100.x, x_1100.y, x_1100.z));
  let x_1103 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1102.x, x_1102.y, x_1102.z, x_1103.w);
  let x_1105 : vec4<f32> = u_xlat8;
  let x_1107 : vec4<f32> = u_xlat8;
  u_xlat46 = dot(vec3<f32>(x_1105.x, x_1105.y, x_1105.z), vec3<f32>(x_1107.x, x_1107.y, x_1107.z));
  let x_1110 : f32 = u_xlat46;
  u_xlat46 = inverseSqrt(x_1110);
  let x_1112 : f32 = u_xlat46;
  let x_1114 : vec4<f32> = u_xlat8;
  let x_1116 : vec3<f32> = (vec3<f32>(x_1112, x_1112, x_1112) * vec3<f32>(x_1114.x, x_1114.y, x_1114.z));
  let x_1117 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1116.x, x_1116.y, x_1116.z, x_1117.w);
  let x_1119 : vec4<f32> = u_xlat8;
  let x_1121 : vec4<f32> = u_xlat1;
  u_xlat46 = dot(vec3<f32>(x_1119.x, x_1119.y, x_1119.z), vec3<f32>(x_1121.x, x_1121.y, x_1121.z));
  let x_1124 : f32 = u_xlat46;
  let x_1126 : f32 = x_105.x_LeakReductionParams.z;
  u_xlat46 = (x_1124 + -(x_1126));
  let x_1129 : f32 = u_xlat46;
  u_xlat46 = clamp(x_1129, 0.0f, 1.0f);
  let x_1132 : f32 = u_xlat10.w;
  let x_1133 : f32 = u_xlat46;
  u_xlat46 = (x_1132 * x_1133);
  let x_1135 : f32 = u_xlat45;
  let x_1136 : f32 = u_xlat46;
  u_xlat5.w = (x_1135 * x_1136);
  let x_1139 : f32 = u_xlat45;
  let x_1140 : f32 = u_xlat46;
  let x_1142 : f32 = u_xlat43;
  u_xlat43 = ((x_1139 * x_1140) + x_1142);
  let x_1144 : f32 = u_xlat43;
  u_xlat43 = max(x_1144, 0.00009999999747378752f);
  let x_1147 : f32 = u_xlat43;
  u_xlat43 = (1.0f / x_1147);
  let x_1149 : f32 = u_xlat43;
  let x_1151 : vec4<f32> = u_xlat11;
  let x_1153 : vec3<f32> = (vec3<f32>(x_1149, x_1149, x_1149) * vec3<f32>(x_1151.x, x_1151.y, x_1151.z));
  let x_1154 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1153.x, x_1153.y, x_1153.z, x_1154.w);
  let x_1156 : f32 = u_xlat43;
  let x_1158 : vec3<f32> = u_xlat12;
  let x_1159 : vec3<f32> = (vec3<f32>(x_1156, x_1156, x_1156) * x_1158);
  let x_1160 : vec4<f32> = u_xlat9;
  u_xlat9 = vec4<f32>(x_1159.x, x_1159.y, x_1159.z, x_1160.w);
  let x_1162 : vec4<f32> = u_xlat8;
  let x_1166 : vec3<f32> = u_xlat3;
  let x_1168 : vec3<f32> = ((vec3<f32>(x_1162.x, x_1162.x, x_1162.x) * vec3<f32>(1.0f, 0.0f, 0.0f)) + -(x_1166));
  let x_1169 : vec4<f32> = u_xlat11;
  u_xlat11 = vec4<f32>(x_1168.x, x_1168.y, x_1168.z, x_1169.w);
  let x_1171 : vec4<f32> = u_xlat8;
  let x_1175 : vec4<f32> = u_xlat11;
  let x_1177 : vec3<f32> = ((vec3<f32>(x_1171.y, x_1171.y, x_1171.y) * vec3<f32>(0.0f, 1.0f, 0.0f)) + vec3<f32>(x_1175.x, x_1175.y, x_1175.z));
  let x_1178 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1177.x, x_1177.y, x_1178.z, x_1177.z);
  let x_1180 : vec4<f32> = u_xlat8;
  let x_1184 : vec4<f32> = u_xlat8;
  let x_1186 : vec3<f32> = ((vec3<f32>(x_1180.z, x_1180.z, x_1180.z) * vec3<f32>(1.0f, 1.0f, 0.0f)) + vec3<f32>(x_1184.x, x_1184.y, x_1184.w));
  let x_1187 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1186.x, x_1186.y, x_1186.z, x_1187.w);
  let x_1189 : vec4<f32> = u_xlat9;
  let x_1193 : vec4<f32> = u_xlat8;
  let x_1195 : vec3<f32> = ((vec3<f32>(x_1189.x, x_1189.x, x_1189.x) * vec3<f32>(0.0f, 0.0f, 1.0f)) + vec3<f32>(x_1193.x, x_1193.y, x_1193.z));
  let x_1196 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1195.x, x_1195.y, x_1195.z, x_1196.w);
  let x_1198 : vec4<f32> = u_xlat9;
  let x_1202 : vec4<f32> = u_xlat8;
  let x_1204 : vec3<f32> = ((vec3<f32>(x_1198.y, x_1198.y, x_1198.y) * vec3<f32>(1.0f, 0.0f, 1.0f)) + vec3<f32>(x_1202.x, x_1202.y, x_1202.z));
  let x_1205 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1204.x, x_1204.y, x_1204.z, x_1205.w);
  let x_1207 : vec4<f32> = u_xlat9;
  let x_1211 : vec4<f32> = u_xlat8;
  let x_1213 : vec3<f32> = ((vec3<f32>(x_1207.z, x_1207.z, x_1207.z) * vec3<f32>(0.0f, 1.0f, 1.0f)) + vec3<f32>(x_1211.x, x_1211.y, x_1211.z));
  let x_1214 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1213.x, x_1213.y, x_1213.z, x_1214.w);
  let x_1216 : vec4<f32> = u_xlat5;
  let x_1218 : f32 = u_xlat43;
  let x_1221 : vec4<f32> = u_xlat8;
  let x_1223 : vec3<f32> = ((vec3<f32>(x_1216.w, x_1216.w, x_1216.w) * vec3<f32>(x_1218, x_1218, x_1218)) + vec3<f32>(x_1221.x, x_1221.y, x_1221.z));
  let x_1224 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1223.x, x_1223.y, x_1223.z, x_1224.w);
  let x_1226 : vec3<f32> = u_xlat3;
  let x_1227 : vec4<f32> = u_xlat8;
  u_xlat3 = (x_1226 + vec3<f32>(x_1227.x, x_1227.y, x_1227.z));
  let x_1231 : f32 = x_105.x_LeakReductionParams.x;
  u_xlatb43 = !((x_1231 == 0.0f));
  let x_1233 : vec4<f32> = u_xlat5;
  let x_1236 : vec3<f32> = u_xlat4;
  let x_1239 : vec4<f32> = u_xlat2;
  let x_1241 : vec3<f32> = ((-(vec3<f32>(x_1233.x, x_1233.y, x_1233.z)) * vec3<f32>(x_1236.x, x_1236.x, x_1236.x)) + vec3<f32>(x_1239.x, x_1239.y, x_1239.z));
  let x_1242 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1241.x, x_1241.y, x_1241.z, x_1242.w);
  let x_1244 : vec4<f32> = u_xlat8;
  let x_1246 : vec3<f32> = u_xlat4;
  let x_1248 : vec3<f32> = (vec3<f32>(x_1244.x, x_1244.y, x_1244.z) / vec3<f32>(x_1246.x, x_1246.x, x_1246.x));
  let x_1249 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_1248.x, x_1248.y, x_1248.z, x_1249.w);
  let x_1251 : bool = u_xlatb43;
  if (x_1251) {
    let x_1256 : vec3<f32> = u_xlat3;
    x_1253 = x_1256;
  } else {
    let x_1258 : vec4<f32> = u_xlat8;
    x_1253 = vec3<f32>(x_1258.x, x_1258.y, x_1258.z);
  }
  let x_1260 : vec3<f32> = x_1253;
  u_xlat3 = x_1260;
  let x_1266 : f32 = in_COLOR0.z;
  u_xlatb43 = any(!((vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) == vec4<f32>(x_1266, x_1266, x_1266, x_1266))));
  let x_1270 : bool = u_xlatb43;
  if (x_1270) {
    let x_1273 : vec4<f32> = in_COLOR0;
    u_xlat8 = (vec4<f32>(x_1273.z, x_1273.z, x_1273.z, x_1273.z) + vec4<f32>(-0.20000000298023223877f, -0.30000001192092895508f, -0.40000000596046447754f, -0.5f));
    let x_1282 : vec4<f32> = u_xlat8;
    u_xlatb8 = (abs(x_1282) < vec4<f32>(0.01999999955296516418f, 0.01999999955296516418f, 0.01999999955296516418f, 0.01999999955296516418f));
    let x_1287 : vec4<f32> = u_xlat5;
    let x_1289 : vec3<f32> = u_xlat4;
    let x_1292 : vec3<f32> = u_xlat4;
    let x_1294 : vec3<f32> = ((vec3<f32>(x_1287.x, x_1287.y, x_1287.z) * vec3<f32>(x_1289.x, x_1289.x, x_1289.x)) + vec3<f32>(x_1292.z, x_1292.y, x_1292.z));
    let x_1295 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_1295.x, x_1294.x, x_1294.y, x_1294.z);
    let x_1298 : f32 = u_xlat7.y;
    u_xlat9.x = x_1298;
    let x_1301 : bool = u_xlatb8.x;
    let x_1302 : vec4<f32> = u_xlat9;
    let x_1303 : vec4<f32> = u_xlat6;
    u_xlat9 = select(x_1303, x_1302, vec4<bool>(x_1301, x_1301, x_1301, x_1301));
    let x_1306 : vec4<f32> = u_xlat5;
    let x_1308 : vec3<f32> = u_xlat4;
    let x_1311 : vec3<f32> = u_xlat4;
    let x_1313 : vec3<f32> = ((vec3<f32>(x_1306.x, x_1306.y, x_1306.z) * vec3<f32>(x_1308.x, x_1308.x, x_1308.x)) + vec3<f32>(x_1311.y, x_1311.y, x_1311.z));
    let x_1314 : vec4<f32> = u_xlat11;
    u_xlat11 = vec4<f32>(x_1314.x, x_1313.x, x_1313.y, x_1313.z);
    let x_1317 : f32 = u_xlat7.z;
    u_xlat11.x = x_1317;
    let x_1320 : bool = u_xlatb8.y;
    let x_1321 : vec4<f32> = u_xlat11;
    let x_1322 : vec4<f32> = u_xlat9;
    u_xlat9 = select(x_1322, x_1321, vec4<bool>(x_1320, x_1320, x_1320, x_1320));
    let x_1325 : vec4<f32> = u_xlat5;
    let x_1327 : vec3<f32> = u_xlat4;
    let x_1330 : vec3<f32> = u_xlat4;
    let x_1332 : vec3<f32> = ((vec3<f32>(x_1325.x, x_1325.y, x_1325.z) * vec3<f32>(x_1327.x, x_1327.x, x_1327.x)) + vec3<f32>(x_1330.y, x_1330.z, x_1330.z));
    let x_1333 : vec4<f32> = u_xlat7;
    u_xlat7 = vec4<f32>(x_1333.x, x_1332.x, x_1332.y, x_1332.z);
    let x_1336 : bool = u_xlatb8.z;
    let x_1337 : vec4<f32> = u_xlat7;
    let x_1338 : vec4<f32> = u_xlat9;
    u_xlat7 = select(x_1338, x_1337, vec4<bool>(x_1336, x_1336, x_1336, x_1336));
    let x_1341 : vec4<f32> = u_xlat5;
    let x_1343 : vec3<f32> = u_xlat4;
    let x_1346 : vec3<f32> = u_xlat4;
    let x_1348 : vec3<f32> = ((vec3<f32>(x_1341.x, x_1341.y, x_1341.z) * vec3<f32>(x_1343.x, x_1343.x, x_1343.x)) + vec3<f32>(x_1346.z, x_1346.z, x_1346.y));
    let x_1349 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_1349.x, x_1348.x, x_1348.y, x_1348.z);
    let x_1352 : f32 = u_xlat10.y;
    u_xlat9.x = x_1352;
    let x_1355 : bool = u_xlatb8.w;
    let x_1356 : vec4<f32> = u_xlat9;
    let x_1357 : vec4<f32> = u_xlat7;
    u_xlat7 = select(x_1357, x_1356, vec4<bool>(x_1355, x_1355, x_1355, x_1355));
    let x_1360 : vec4<f32> = in_COLOR0;
    let x_1366 : vec3<f32> = (vec3<f32>(x_1360.z, x_1360.z, x_1360.z) + vec3<f32>(-0.60000002384185791016f, -0.69999998807907104492f, -0.80000001192092895508f));
    let x_1367 : vec4<f32> = u_xlat8;
    u_xlat8 = vec4<f32>(x_1366.x, x_1366.y, x_1366.z, x_1367.w);
    let x_1369 : vec4<f32> = u_xlat8;
    let x_1373 : vec4<bool> = (abs(vec4<f32>(x_1369.x, x_1369.y, x_1369.z, x_1369.x)) < vec4<f32>(0.01999999955296516418f, 0.01999999955296516418f, 0.01999999955296516418f, 0.0f));
    let x_1374 : vec3<bool> = vec3<bool>(x_1373.x, x_1373.y, x_1373.z);
    let x_1375 : vec4<bool> = u_xlatb8;
    u_xlatb8 = vec4<bool>(x_1374.x, x_1374.y, x_1374.z, x_1375.w);
    let x_1377 : vec4<f32> = u_xlat5;
    let x_1379 : vec3<f32> = u_xlat4;
    let x_1382 : vec3<f32> = u_xlat4;
    let x_1384 : vec3<f32> = ((vec3<f32>(x_1377.x, x_1377.y, x_1377.z) * vec3<f32>(x_1379.x, x_1379.x, x_1379.x)) + vec3<f32>(x_1382.z, x_1382.y, x_1382.y));
    let x_1385 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_1385.x, x_1384.x, x_1384.y, x_1384.z);
    let x_1388 : f32 = u_xlat10.z;
    u_xlat9.x = x_1388;
    let x_1391 : bool = u_xlatb8.x;
    let x_1392 : vec4<f32> = u_xlat9;
    let x_1393 : vec4<f32> = u_xlat7;
    u_xlat7 = select(x_1393, x_1392, vec4<bool>(x_1391, x_1391, x_1391, x_1391));
    let x_1396 : vec4<f32> = u_xlat5;
    let x_1398 : vec3<f32> = u_xlat4;
    let x_1401 : vec3<f32> = u_xlat4;
    let x_1403 : vec3<f32> = ((vec3<f32>(x_1396.x, x_1396.y, x_1396.z) * vec3<f32>(x_1398.x, x_1398.x, x_1398.x)) + vec3<f32>(x_1401.x, x_1401.x, x_1401.x));
    let x_1404 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_1404.x, x_1403.x, x_1403.y, x_1403.z);
    let x_1407 : f32 = u_xlat10.w;
    u_xlat9.x = x_1407;
    let x_1410 : bool = u_xlatb8.y;
    let x_1411 : vec4<f32> = u_xlat9;
    let x_1412 : vec4<f32> = u_xlat7;
    u_xlat7 = select(x_1412, x_1411, vec4<bool>(x_1410, x_1410, x_1410, x_1410));
    let x_1415 : vec4<f32> = u_xlat5;
    let x_1417 : vec3<f32> = u_xlat4;
    let x_1420 : vec3<f32> = u_xlat4;
    let x_1422 : vec3<f32> = ((vec3<f32>(x_1415.x, x_1415.y, x_1415.z) * vec3<f32>(x_1417.x, x_1417.x, x_1417.x)) + vec3<f32>(x_1420.y, x_1420.z, x_1420.y));
    let x_1423 : vec4<f32> = u_xlat10;
    u_xlat10 = vec4<f32>(x_1423.x, x_1422.x, x_1422.y, x_1422.z);
    let x_1426 : bool = u_xlatb8.z;
    let x_1427 : vec4<f32> = u_xlat10;
    let x_1428 : vec4<f32> = u_xlat7;
    u_xlat7 = select(x_1428, x_1427, vec4<bool>(x_1426, x_1426, x_1426, x_1426));
    let x_1432 : vec4<f32> = u_xlat5;
    let x_1434 : vec3<f32> = u_xlat4;
    let x_1437 : vec4<f32> = u_xlat7;
    u_xlat18 = ((vec3<f32>(x_1432.x, x_1432.y, x_1432.z) * vec3<f32>(x_1434.x, x_1434.x, x_1434.x)) + -(vec3<f32>(x_1437.y, x_1437.z, x_1437.w)));
    let x_1441 : vec3<f32> = u_xlat18;
    let x_1442 : vec3<f32> = u_xlat18;
    u_xlat43 = dot(x_1441, x_1442);
    let x_1444 : f32 = u_xlat43;
    u_xlat43 = sqrt(x_1444);
    let x_1446 : f32 = u_xlat43;
    u_xlatb43 = (x_1446 < 0.00009999999747378752f);
    let x_1448 : bool = u_xlatb43;
    if (x_1448) {
      let x_1451 : vec3<f32> = u_xlat3;
      u_xlat18 = (-(x_1451) + vec3<f32>(1.0f, 1.0f, 1.0f));
      let x_1455 : f32 = u_xlat18.y;
      let x_1457 : f32 = u_xlat18.x;
      u_xlat43 = (x_1455 * x_1457);
      let x_1460 : f32 = u_xlat18.z;
      let x_1461 : f32 = u_xlat43;
      u_xlat8.x = (x_1460 * x_1461);
    } else {
      let x_1465 : f32 = u_xlat42;
      let x_1470 : vec4<f32> = u_xlat7;
      u_xlat18 = ((vec3<f32>(x_1465, x_1465, x_1465) * vec3<f32>(-0.3333333432674407959f, 0.0f, 0.0f)) + vec3<f32>(x_1470.y, x_1470.z, x_1470.w));
      let x_1473 : vec4<f32> = u_xlat5;
      let x_1475 : vec3<f32> = u_xlat4;
      let x_1478 : vec3<f32> = u_xlat18;
      u_xlat18 = ((vec3<f32>(x_1473.x, x_1473.y, x_1473.z) * vec3<f32>(x_1475.x, x_1475.x, x_1475.x)) + -(x_1478));
      let x_1481 : vec3<f32> = u_xlat18;
      let x_1482 : vec3<f32> = u_xlat18;
      u_xlat43 = dot(x_1481, x_1482);
      let x_1484 : f32 = u_xlat43;
      u_xlat43 = sqrt(x_1484);
      let x_1486 : f32 = u_xlat43;
      u_xlatb43 = (x_1486 < 0.00009999999747378752f);
      let x_1488 : bool = u_xlatb43;
      if (x_1488) {
        let x_1491 : vec3<f32> = u_xlat3;
        let x_1495 : vec2<f32> = (-(vec2<f32>(x_1491.y, x_1491.z)) + vec2<f32>(1.0f, 1.0f));
        let x_1496 : vec3<f32> = u_xlat18;
        u_xlat18 = vec3<f32>(x_1495.x, x_1495.y, x_1496.z);
        let x_1499 : f32 = u_xlat3.x;
        let x_1501 : f32 = u_xlat18.x;
        u_xlat43 = (x_1499 * x_1501);
        let x_1504 : f32 = u_xlat18.y;
        let x_1505 : f32 = u_xlat43;
        u_xlat8.x = (x_1504 * x_1505);
      } else {
        let x_1509 : f32 = u_xlat42;
        let x_1513 : vec4<f32> = u_xlat7;
        u_xlat18 = ((vec3<f32>(x_1509, x_1509, x_1509) * vec3<f32>(-0.3333333432674407959f, -0.3333333432674407959f, 0.0f)) + vec3<f32>(x_1513.y, x_1513.z, x_1513.w));
        let x_1516 : vec4<f32> = u_xlat5;
        let x_1518 : vec3<f32> = u_xlat4;
        let x_1521 : vec3<f32> = u_xlat18;
        u_xlat18 = ((vec3<f32>(x_1516.x, x_1516.y, x_1516.z) * vec3<f32>(x_1518.x, x_1518.x, x_1518.x)) + -(x_1521));
        let x_1524 : vec3<f32> = u_xlat18;
        let x_1525 : vec3<f32> = u_xlat18;
        u_xlat43 = dot(x_1524, x_1525);
        let x_1527 : f32 = u_xlat43;
        u_xlat43 = sqrt(x_1527);
        let x_1529 : f32 = u_xlat43;
        u_xlatb43 = (x_1529 < 0.00009999999747378752f);
        let x_1531 : bool = u_xlatb43;
        if (x_1531) {
          let x_1535 : f32 = u_xlat3.y;
          let x_1537 : f32 = u_xlat3.x;
          u_xlat43 = (x_1535 * x_1537);
          let x_1540 : f32 = u_xlat3.z;
          u_xlat45 = (-(x_1540) + 1.0f);
          let x_1543 : f32 = u_xlat43;
          let x_1544 : f32 = u_xlat45;
          u_xlat8.x = (x_1543 * x_1544);
        } else {
          let x_1548 : f32 = u_xlat42;
          let x_1552 : vec4<f32> = u_xlat7;
          u_xlat18 = ((vec3<f32>(x_1548, x_1548, x_1548) * vec3<f32>(0.0f, -0.3333333432674407959f, 0.0f)) + vec3<f32>(x_1552.y, x_1552.z, x_1552.w));
          let x_1555 : vec4<f32> = u_xlat5;
          let x_1557 : vec3<f32> = u_xlat4;
          let x_1560 : vec3<f32> = u_xlat18;
          u_xlat18 = ((vec3<f32>(x_1555.x, x_1555.y, x_1555.z) * vec3<f32>(x_1557.x, x_1557.x, x_1557.x)) + -(x_1560));
          let x_1563 : vec3<f32> = u_xlat18;
          let x_1564 : vec3<f32> = u_xlat18;
          u_xlat43 = dot(x_1563, x_1564);
          let x_1566 : f32 = u_xlat43;
          u_xlat43 = sqrt(x_1566);
          let x_1568 : f32 = u_xlat43;
          u_xlatb43 = (x_1568 < 0.00009999999747378752f);
          let x_1570 : bool = u_xlatb43;
          if (x_1570) {
            let x_1573 : vec3<f32> = u_xlat3;
            let x_1576 : vec2<f32> = (-(vec2<f32>(x_1573.x, x_1573.z)) + vec2<f32>(1.0f, 1.0f));
            let x_1577 : vec3<f32> = u_xlat18;
            u_xlat18 = vec3<f32>(x_1576.x, x_1576.y, x_1577.z);
            let x_1580 : f32 = u_xlat3.y;
            let x_1582 : f32 = u_xlat18.x;
            u_xlat43 = (x_1580 * x_1582);
            let x_1585 : f32 = u_xlat18.y;
            let x_1586 : f32 = u_xlat43;
            u_xlat8.x = (x_1585 * x_1586);
          } else {
            let x_1590 : f32 = u_xlat42;
            let x_1594 : vec4<f32> = u_xlat7;
            u_xlat18 = ((vec3<f32>(x_1590, x_1590, x_1590) * vec3<f32>(-0.3333333432674407959f, 0.0f, -0.3333333432674407959f)) + vec3<f32>(x_1594.y, x_1594.z, x_1594.w));
            let x_1597 : vec4<f32> = u_xlat5;
            let x_1599 : vec3<f32> = u_xlat4;
            let x_1602 : vec3<f32> = u_xlat18;
            u_xlat18 = ((vec3<f32>(x_1597.x, x_1597.y, x_1597.z) * vec3<f32>(x_1599.x, x_1599.x, x_1599.x)) + -(x_1602));
            let x_1605 : vec3<f32> = u_xlat18;
            let x_1606 : vec3<f32> = u_xlat18;
            u_xlat43 = dot(x_1605, x_1606);
            let x_1608 : f32 = u_xlat43;
            u_xlat43 = sqrt(x_1608);
            let x_1610 : f32 = u_xlat43;
            u_xlatb43 = (x_1610 < 0.00009999999747378752f);
            let x_1612 : vec3<f32> = u_xlat3;
            let x_1615 : vec2<f32> = (-(vec2<f32>(x_1612.y, x_1612.x)) + vec2<f32>(1.0f, 1.0f));
            let x_1616 : vec3<f32> = u_xlat18;
            u_xlat18 = vec3<f32>(x_1615.x, x_1615.y, x_1616.z);
            let x_1618 : vec3<f32> = u_xlat3;
            let x_1620 : vec3<f32> = u_xlat18;
            let x_1622 : vec2<f32> = (vec2<f32>(x_1618.x, x_1618.y) * vec2<f32>(x_1620.x, x_1620.y));
            let x_1623 : vec4<f32> = u_xlat9;
            u_xlat9 = vec4<f32>(x_1622.x, x_1622.y, x_1623.z, x_1623.w);
            let x_1625 : f32 = u_xlat42;
            let x_1629 : vec4<f32> = u_xlat7;
            let x_1631 : vec3<f32> = ((vec3<f32>(x_1625, x_1625, x_1625) * vec3<f32>(0.0f, 0.0f, -0.3333333432674407959f)) + vec3<f32>(x_1629.y, x_1629.z, x_1629.w));
            let x_1632 : vec4<f32> = u_xlat10;
            u_xlat10 = vec4<f32>(x_1631.x, x_1631.y, x_1631.z, x_1632.w);
            let x_1634 : vec4<f32> = u_xlat5;
            let x_1636 : vec3<f32> = u_xlat4;
            let x_1639 : vec4<f32> = u_xlat10;
            let x_1642 : vec3<f32> = ((vec3<f32>(x_1634.x, x_1634.y, x_1634.z) * vec3<f32>(x_1636.x, x_1636.x, x_1636.x)) + -(vec3<f32>(x_1639.x, x_1639.y, x_1639.z)));
            let x_1643 : vec4<f32> = u_xlat10;
            u_xlat10 = vec4<f32>(x_1642.x, x_1642.y, x_1642.z, x_1643.w);
            let x_1645 : vec4<f32> = u_xlat10;
            let x_1647 : vec4<f32> = u_xlat10;
            u_xlat45 = dot(vec3<f32>(x_1645.x, x_1645.y, x_1645.z), vec3<f32>(x_1647.x, x_1647.y, x_1647.z));
            let x_1650 : f32 = u_xlat45;
            u_xlat45 = sqrt(x_1650);
            let x_1653 : f32 = u_xlat45;
            u_xlatb45 = (x_1653 < 0.00009999999747378752f);
            let x_1656 : f32 = u_xlat18.x;
            let x_1658 : f32 = u_xlat18.y;
            u_xlat18.x = (x_1656 * x_1658);
            let x_1661 : f32 = u_xlat42;
            let x_1665 : vec4<f32> = u_xlat7;
            let x_1667 : vec3<f32> = ((vec3<f32>(x_1661, x_1661, x_1661) * vec3<f32>(-0.3333333432674407959f, -0.3333333432674407959f, -0.3333333432674407959f)) + vec3<f32>(x_1665.y, x_1665.z, x_1665.w));
            let x_1668 : vec4<f32> = u_xlat10;
            u_xlat10 = vec4<f32>(x_1667.x, x_1667.y, x_1667.z, x_1668.w);
            let x_1670 : vec4<f32> = u_xlat5;
            let x_1672 : vec3<f32> = u_xlat4;
            let x_1675 : vec4<f32> = u_xlat10;
            let x_1678 : vec3<f32> = ((vec3<f32>(x_1670.x, x_1670.y, x_1670.z) * vec3<f32>(x_1672.x, x_1672.x, x_1672.x)) + -(vec3<f32>(x_1675.x, x_1675.y, x_1675.z)));
            let x_1679 : vec4<f32> = u_xlat10;
            u_xlat10 = vec4<f32>(x_1678.x, x_1678.y, x_1678.z, x_1679.w);
            let x_1682 : vec4<f32> = u_xlat10;
            let x_1684 : vec4<f32> = u_xlat10;
            u_xlat32 = dot(vec3<f32>(x_1682.x, x_1682.y, x_1682.z), vec3<f32>(x_1684.x, x_1684.y, x_1684.z));
            let x_1687 : f32 = u_xlat32;
            u_xlat32 = sqrt(x_1687);
            let x_1690 : f32 = u_xlat32;
            u_xlatb32 = (x_1690 < 0.00009999999747378752f);
            let x_1693 : f32 = u_xlat3.y;
            let x_1695 : f32 = u_xlat3.x;
            u_xlat18.z = (x_1693 * x_1695);
            let x_1698 : vec3<f32> = u_xlat3;
            let x_1700 : vec3<f32> = u_xlat18;
            let x_1702 : vec2<f32> = (vec2<f32>(x_1698.z, x_1698.z) * vec2<f32>(x_1700.x, x_1700.z));
            let x_1703 : vec3<f32> = u_xlat18;
            u_xlat18 = vec3<f32>(x_1702.x, x_1703.y, x_1702.y);
            let x_1705 : f32 = u_xlat42;
            let x_1709 : vec4<f32> = u_xlat7;
            let x_1711 : vec3<f32> = ((vec3<f32>(x_1705, x_1705, x_1705) * vec3<f32>(0.0f, -0.3333333432674407959f, -0.3333333432674407959f)) + vec3<f32>(x_1709.y, x_1709.z, x_1709.w));
            let x_1712 : vec4<f32> = u_xlat10;
            u_xlat10 = vec4<f32>(x_1711.x, x_1711.y, x_1711.z, x_1712.w);
            let x_1714 : vec4<f32> = u_xlat5;
            let x_1716 : vec3<f32> = u_xlat4;
            let x_1719 : vec4<f32> = u_xlat10;
            let x_1722 : vec3<f32> = ((vec3<f32>(x_1714.x, x_1714.y, x_1714.z) * vec3<f32>(x_1716.x, x_1716.x, x_1716.x)) + -(vec3<f32>(x_1719.x, x_1719.y, x_1719.z)));
            let x_1723 : vec4<f32> = u_xlat5;
            u_xlat5 = vec4<f32>(x_1722.x, x_1722.y, x_1722.z, x_1723.w);
            let x_1725 : vec4<f32> = u_xlat5;
            let x_1727 : vec4<f32> = u_xlat5;
            u_xlat42 = dot(vec3<f32>(x_1725.x, x_1725.y, x_1725.z), vec3<f32>(x_1727.x, x_1727.y, x_1727.z));
            let x_1730 : f32 = u_xlat42;
            u_xlat42 = sqrt(x_1730);
            let x_1732 : f32 = u_xlat42;
            u_xlatb42 = (x_1732 < 0.00009999999747378752f);
            let x_1734 : vec3<f32> = u_xlat3;
            let x_1736 : vec4<f32> = u_xlat9;
            let x_1738 : vec2<f32> = (vec2<f32>(x_1734.z, x_1734.z) * vec2<f32>(x_1736.x, x_1736.y));
            let x_1739 : vec4<f32> = u_xlat5;
            u_xlat5 = vec4<f32>(x_1738.x, x_1738.y, x_1739.z, x_1739.w);
            let x_1741 : bool = u_xlatb42;
            if (x_1741) {
              let x_1747 : f32 = u_xlat5.y;
              x_1743 = x_1747;
            } else {
              x_1743 = 0.0f;
            }
            let x_1749 : f32 = x_1743;
            u_xlat42 = x_1749;
            let x_1750 : bool = u_xlatb32;
            if (x_1750) {
              let x_1755 : f32 = u_xlat18.z;
              x_1751 = x_1755;
            } else {
              let x_1757 : f32 = u_xlat42;
              x_1751 = x_1757;
            }
            let x_1758 : f32 = x_1751;
            u_xlat42 = x_1758;
            let x_1759 : bool = u_xlatb45;
            if (x_1759) {
              let x_1764 : f32 = u_xlat18.x;
              x_1760 = x_1764;
            } else {
              let x_1766 : f32 = u_xlat42;
              x_1760 = x_1766;
            }
            let x_1767 : f32 = x_1760;
            u_xlat42 = x_1767;
            let x_1768 : bool = u_xlatb43;
            if (x_1768) {
              let x_1773 : f32 = u_xlat5.x;
              x_1769 = x_1773;
            } else {
              let x_1775 : f32 = u_xlat42;
              x_1769 = x_1775;
            }
            let x_1776 : f32 = x_1769;
            u_xlat8.x = x_1776;
          }
        }
      }
    }
    let x_1779 : vec4<f32> = x_76.unity_MatrixInvV[1i];
    let x_1782 : f32 = x_76.x_ProbeSize;
    let x_1784 : f32 = x_76.x_ProbeSize;
    let x_1786 : f32 = x_76.x_ProbeSize;
    let x_1787 : vec3<f32> = vec3<f32>(x_1782, x_1784, x_1786);
    u_xlat18 = (vec3<f32>(x_1779.x, x_1779.y, x_1779.z) * vec3<f32>(x_1787.x, x_1787.y, x_1787.z));
    let x_1793 : vec3<f32> = u_xlat18;
    let x_1797 : vec4<f32> = u_xlat7;
    u_xlat18 = ((x_1793 * vec3<f32>(0.6666666865348815918f, 0.6666666865348815918f, 0.6666666865348815918f)) + vec3<f32>(x_1797.y, x_1797.z, x_1797.w));
    let x_1800 : vec3<f32> = u_xlat18;
    let x_1806 : vec4<f32> = x_1804.unity_ObjectToWorld[3i];
    let x_1808 : vec3<f32> = (x_1800 + vec3<f32>(x_1806.x, x_1806.y, x_1806.z));
    let x_1809 : vec4<f32> = u_xlat5;
    u_xlat5 = vec4<f32>(x_1808.x, x_1808.y, x_1808.z, x_1809.w);
    let x_1812 : vec4<f32> = in_POSITION0;
    let x_1815 : vec4<f32> = x_76.unity_MatrixInvV[1i];
    u_xlat18 = (vec3<f32>(x_1812.y, x_1812.y, x_1812.y) * vec3<f32>(x_1815.x, x_1815.y, x_1815.z));
    let x_1818 : vec3<f32> = u_xlat18;
    u_xlat18 = (x_1818 * vec3<f32>(0.5f, 0.5f, 0.5f));
    let x_1820 : vec4<f32> = in_POSITION0;
    let x_1823 : vec4<f32> = x_76.unity_MatrixInvV[0i];
    let x_1827 : vec3<f32> = u_xlat18;
    u_xlat18 = ((vec3<f32>(x_1820.x, x_1820.x, x_1820.x) * -(vec3<f32>(x_1823.x, x_1823.y, x_1823.z))) + x_1827);
    let x_1829 : vec3<f32> = u_xlat18;
    let x_1831 : f32 = x_76.x_ProbeSize;
    let x_1833 : f32 = x_76.x_ProbeSize;
    let x_1835 : f32 = x_76.x_ProbeSize;
    let x_1836 : vec3<f32> = vec3<f32>(x_1831, x_1833, x_1835);
    u_xlat18 = (x_1829 * vec3<f32>(x_1836.x, x_1836.y, x_1836.z));
    let x_1842 : vec3<f32> = u_xlat18;
    let x_1845 : vec3<f32> = (x_1842 * vec3<f32>(20.0f, 20.0f, 20.0f));
    let x_1846 : vec4<f32> = u_xlat9;
    u_xlat9 = vec4<f32>(x_1845.x, x_1845.y, x_1845.z, x_1846.w);
    let x_1849 : f32 = x_1804.unity_ObjectToWorld[3i].w;
    u_xlat5.w = x_1849;
    u_xlat9.w = 0.0f;
    let x_1852 : vec4<f32> = u_xlat5;
    let x_1853 : vec4<f32> = u_xlat9;
    u_xlat5 = (x_1852 + x_1853);
    let x_1856 : f32 = u_xlat7.x;
    u_xlat8.y = x_1856;
  } else {
    let x_1860 : f32 = in_COLOR0.y;
    u_xlatb42 = any(!((vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) == vec4<f32>(x_1860, x_1860, x_1860, x_1860))));
    let x_1864 : bool = u_xlatb42;
    if (x_1864) {
      let x_1867 : vec4<f32> = u_xlat1;
      let x_1869 : vec4<f32> = u_xlat1;
      u_xlat42 = dot(vec3<f32>(x_1867.x, x_1867.y, x_1867.z), vec3<f32>(x_1869.x, x_1869.y, x_1869.z));
      let x_1872 : f32 = u_xlat42;
      u_xlat42 = inverseSqrt(x_1872);
      let x_1874 : f32 = u_xlat42;
      let x_1876 : vec4<f32> = u_xlat1;
      let x_1878 : vec3<f32> = (vec3<f32>(x_1874, x_1874, x_1874) * vec3<f32>(x_1876.x, x_1876.y, x_1876.z));
      let x_1879 : vec4<f32> = u_xlat1;
      u_xlat1 = vec4<f32>(x_1878.x, x_1878.y, x_1878.z, x_1879.w);
      let x_1883 : f32 = u_xlat1.y;
      u_xlatb42 = (0.89999997615814208984f < x_1883);
      let x_1885 : bool = u_xlatb42;
      u_xlat18 = select(vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 1.0f), vec3<bool>(x_1885, x_1885, x_1885));
      let x_1888 : vec4<f32> = u_xlat1;
      let x_1890 : vec3<f32> = u_xlat18;
      let x_1891 : vec3<f32> = (vec3<f32>(x_1888.z, x_1888.x, x_1888.y) * x_1890);
      let x_1892 : vec4<f32> = u_xlat7;
      u_xlat7 = vec4<f32>(x_1891.x, x_1891.y, x_1891.z, x_1892.w);
      let x_1894 : vec4<f32> = u_xlat1;
      let x_1896 : vec3<f32> = u_xlat18;
      let x_1899 : vec4<f32> = u_xlat7;
      u_xlat18 = ((vec3<f32>(x_1894.y, x_1894.z, x_1894.x) * vec3<f32>(x_1896.y, x_1896.z, x_1896.x)) + -(vec3<f32>(x_1899.x, x_1899.y, x_1899.z)));
      let x_1903 : vec3<f32> = u_xlat18;
      let x_1904 : vec3<f32> = u_xlat18;
      u_xlat42 = dot(x_1903, x_1904);
      let x_1906 : f32 = u_xlat42;
      u_xlat42 = inverseSqrt(x_1906);
      let x_1908 : f32 = u_xlat42;
      let x_1910 : vec3<f32> = u_xlat18;
      u_xlat18 = (vec3<f32>(x_1908, x_1908, x_1908) * x_1910);
      let x_1912 : vec4<f32> = u_xlat1;
      let x_1914 : vec3<f32> = u_xlat18;
      let x_1916 : vec3<f32> = (vec3<f32>(x_1912.y, x_1912.z, x_1912.x) * vec3<f32>(x_1914.z, x_1914.x, x_1914.y));
      let x_1917 : vec4<f32> = u_xlat7;
      u_xlat7 = vec4<f32>(x_1916.x, x_1916.y, x_1916.z, x_1917.w);
      let x_1919 : vec3<f32> = u_xlat18;
      let x_1921 : vec4<f32> = u_xlat1;
      let x_1924 : vec4<f32> = u_xlat7;
      let x_1927 : vec3<f32> = ((vec3<f32>(x_1919.y, x_1919.z, x_1919.x) * vec3<f32>(x_1921.z, x_1921.x, x_1921.y)) + -(vec3<f32>(x_1924.x, x_1924.y, x_1924.z)));
      let x_1928 : vec4<f32> = u_xlat7;
      u_xlat7 = vec4<f32>(x_1927.x, x_1927.y, x_1927.z, x_1928.w);
      let x_1930 : vec4<f32> = in_POSITION0;
      let x_1933 : f32 = x_76.x_ProbeSize;
      let x_1935 : f32 = x_76.x_ProbeSize;
      let x_1937 : f32 = x_76.x_ProbeSize;
      let x_1938 : vec3<f32> = vec3<f32>(x_1933, x_1935, x_1937);
      let x_1943 : vec3<f32> = (vec3<f32>(x_1930.x, x_1930.y, x_1930.z) * vec3<f32>(x_1938.x, x_1938.y, x_1938.z));
      let x_1944 : vec4<f32> = u_xlat9;
      u_xlat9 = vec4<f32>(x_1943.x, x_1943.y, x_1943.z, x_1944.w);
      let x_1946 : vec4<f32> = u_xlat9;
      let x_1950 : vec3<f32> = (vec3<f32>(x_1946.x, x_1946.y, x_1946.z) * vec3<f32>(5.0f, 5.0f, 5.0f));
      let x_1951 : vec4<f32> = u_xlat9;
      u_xlat9 = vec4<f32>(x_1950.x, x_1950.y, x_1950.z, x_1951.w);
      let x_1953 : vec4<f32> = u_xlat7;
      let x_1955 : vec4<f32> = u_xlat9;
      let x_1957 : vec3<f32> = (vec3<f32>(x_1953.x, x_1953.y, x_1953.z) * vec3<f32>(x_1955.y, x_1955.y, x_1955.y));
      let x_1958 : vec4<f32> = u_xlat7;
      u_xlat7 = vec4<f32>(x_1957.x, x_1957.y, x_1957.z, x_1958.w);
      let x_1960 : vec3<f32> = u_xlat18;
      let x_1961 : vec4<f32> = u_xlat9;
      let x_1964 : vec4<f32> = u_xlat7;
      u_xlat18 = ((x_1960 * vec3<f32>(x_1961.x, x_1961.x, x_1961.x)) + vec3<f32>(x_1964.x, x_1964.y, x_1964.z));
      let x_1967 : vec4<f32> = u_xlat1;
      let x_1969 : vec4<f32> = u_xlat9;
      let x_1972 : vec3<f32> = u_xlat18;
      let x_1973 : vec3<f32> = ((vec3<f32>(x_1967.x, x_1967.y, x_1967.z) * vec3<f32>(x_1969.z, x_1969.z, x_1969.z)) + x_1972);
      let x_1974 : vec4<f32> = u_xlat1;
      u_xlat1 = vec4<f32>(x_1973.x, x_1973.y, x_1973.z, x_1974.w);
      let x_1976 : vec4<f32> = u_xlat1;
      let x_1979 : vec4<f32> = x_1804.unity_ObjectToWorld[1i];
      u_xlat7 = (vec4<f32>(x_1976.y, x_1976.y, x_1976.y, x_1976.y) * x_1979);
      let x_1982 : vec4<f32> = x_1804.unity_ObjectToWorld[0i];
      let x_1983 : vec4<f32> = u_xlat1;
      let x_1986 : vec4<f32> = u_xlat7;
      u_xlat7 = ((x_1982 * vec4<f32>(x_1983.x, x_1983.x, x_1983.x, x_1983.x)) + x_1986);
      let x_1989 : vec4<f32> = x_1804.unity_ObjectToWorld[2i];
      let x_1990 : vec4<f32> = u_xlat1;
      let x_1993 : vec4<f32> = u_xlat7;
      u_xlat1 = ((x_1989 * vec4<f32>(x_1990.z, x_1990.z, x_1990.z, x_1990.z)) + x_1993);
      let x_1995 : vec4<f32> = u_xlat1;
      let x_1997 : vec4<f32> = x_1804.unity_ObjectToWorld[3i];
      u_xlat5 = (x_1995 + x_1997);
      let x_1999 : vec4<f32> = u_xlat0;
      let x_2001 : vec4<f32> = u_xlat5;
      let x_2003 : vec3<f32> = (vec3<f32>(x_1999.x, x_1999.y, x_1999.z) + vec3<f32>(x_2001.x, x_2001.y, x_2001.z));
      let x_2004 : vec4<f32> = u_xlat5;
      u_xlat5 = vec4<f32>(x_2003.x, x_2003.y, x_2003.z, x_2004.w);
    } else {
      let x_2009 : f32 = in_COLOR0.x;
      u_xlatb0 = any(!((vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) == vec4<f32>(x_2009, x_2009, x_2009, x_2009))));
      let x_2013 : bool = u_xlatb0;
      if (x_2013) {
        let x_2018 : i32 = x_76.x_ForceDebugNormalViewBias;
        if ((x_2018 != 0i)) {
          let x_2022 : vec4<f32> = in_POSITION0;
          let x_2025 : f32 = x_76.x_ProbeSize;
          let x_2027 : f32 = x_76.x_ProbeSize;
          let x_2029 : f32 = x_76.x_ProbeSize;
          let x_2030 : vec3<f32> = vec3<f32>(x_2025, x_2027, x_2029);
          let x_2035 : vec3<f32> = (vec3<f32>(x_2022.x, x_2022.y, x_2022.z) * vec3<f32>(x_2030.x, x_2030.y, x_2030.z));
          let x_2036 : vec4<f32> = u_xlat0;
          u_xlat0 = vec4<f32>(x_2035.x, x_2035.y, x_2035.z, x_2036.w);
          let x_2038 : vec4<f32> = u_xlat0;
          let x_2042 : vec3<f32> = (vec3<f32>(x_2038.x, x_2038.y, x_2038.z) * vec3<f32>(1.5f, 1.5f, 1.5f));
          let x_2043 : vec4<f32> = u_xlat0;
          u_xlat0 = vec4<f32>(x_2042.x, x_2042.y, x_2042.z, x_2043.w);
          let x_2045 : vec4<f32> = u_xlat0;
          let x_2048 : vec4<f32> = x_1804.unity_ObjectToWorld[1i];
          u_xlat1 = (vec4<f32>(x_2045.y, x_2045.y, x_2045.y, x_2045.y) * x_2048);
          let x_2051 : vec4<f32> = x_1804.unity_ObjectToWorld[0i];
          let x_2052 : vec4<f32> = u_xlat0;
          let x_2055 : vec4<f32> = u_xlat1;
          u_xlat1 = ((x_2051 * vec4<f32>(x_2052.x, x_2052.x, x_2052.x, x_2052.x)) + x_2055);
          let x_2058 : vec4<f32> = x_1804.unity_ObjectToWorld[2i];
          let x_2059 : vec4<f32> = u_xlat0;
          let x_2062 : vec4<f32> = u_xlat1;
          u_xlat0 = ((x_2058 * vec4<f32>(x_2059.z, x_2059.z, x_2059.z, x_2059.z)) + x_2062);
          let x_2064 : vec4<f32> = u_xlat0;
          let x_2066 : vec4<f32> = x_1804.unity_ObjectToWorld[3i];
          u_xlat0 = (x_2064 + x_2066);
          u_xlat2.w = 0.0f;
          let x_2069 : vec4<f32> = u_xlat0;
          let x_2070 : vec4<f32> = u_xlat2;
          u_xlat5 = (x_2069 + x_2070);
        } else {
          gl_Position = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
          vs_COLOR0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
          phase0_Output0_3 = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
          vs_TEXCOORD1 = vec3<f32>(0.0f, 0.0f, 0.0f);
          let x_2086 : vec4<f32> = phase0_Output0_3;
          vs_TEXCOORD0 = vec2<f32>(x_2086.x, x_2086.y);
          let x_2089 : vec4<f32> = phase0_Output0_3;
          vs_TEXCOORD2 = vec2<f32>(x_2089.z, x_2089.w);
          return;
        }
      } else {
        let x_2093 : vec4<f32> = in_POSITION0;
        let x_2096 : f32 = x_76.x_ProbeSize;
        let x_2098 : f32 = x_76.x_ProbeSize;
        let x_2100 : f32 = x_76.x_ProbeSize;
        let x_2101 : vec3<f32> = vec3<f32>(x_2096, x_2098, x_2100);
        let x_2106 : vec3<f32> = (vec3<f32>(x_2093.x, x_2093.y, x_2093.z) * vec3<f32>(x_2101.x, x_2101.y, x_2101.z));
        let x_2107 : vec4<f32> = u_xlat0;
        u_xlat0 = vec4<f32>(x_2106.x, x_2106.y, x_2106.z, x_2107.w);
        let x_2109 : vec4<f32> = u_xlat0;
        let x_2111 : vec3<f32> = (vec3<f32>(x_2109.x, x_2109.y, x_2109.z) * vec3<f32>(3.0f, 3.0f, 3.0f));
        let x_2112 : vec4<f32> = u_xlat0;
        u_xlat0 = vec4<f32>(x_2111.x, x_2111.y, x_2111.z, x_2112.w);
        let x_2114 : vec4<f32> = u_xlat0;
        let x_2117 : vec4<f32> = x_1804.unity_ObjectToWorld[1i];
        u_xlat1 = (vec4<f32>(x_2114.y, x_2114.y, x_2114.y, x_2114.y) * x_2117);
        let x_2120 : vec4<f32> = x_1804.unity_ObjectToWorld[0i];
        let x_2121 : vec4<f32> = u_xlat0;
        let x_2124 : vec4<f32> = u_xlat1;
        u_xlat1 = ((x_2120 * vec4<f32>(x_2121.x, x_2121.x, x_2121.x, x_2121.x)) + x_2124);
        let x_2127 : vec4<f32> = x_1804.unity_ObjectToWorld[2i];
        let x_2128 : vec4<f32> = u_xlat0;
        let x_2131 : vec4<f32> = u_xlat1;
        u_xlat0 = ((x_2127 * vec4<f32>(x_2128.z, x_2128.z, x_2128.z, x_2128.z)) + x_2131);
        let x_2133 : vec4<f32> = u_xlat0;
        let x_2135 : vec4<f32> = x_1804.unity_ObjectToWorld[3i];
        u_xlat0 = (x_2133 + x_2135);
        let x_2137 : vec3<f32> = u_xlat3;
        let x_2138 : vec3<f32> = u_xlat4;
        let x_2141 : vec4<f32> = u_xlat6;
        let x_2143 : vec3<f32> = ((x_2137 * vec3<f32>(x_2138.x, x_2138.x, x_2138.x)) + vec3<f32>(x_2141.y, x_2141.z, x_2141.w));
        let x_2144 : vec4<f32> = u_xlat1;
        u_xlat1 = vec4<f32>(x_2143.x, x_2143.y, x_2143.z, x_2144.w);
        u_xlat1.w = 0.0f;
        let x_2147 : vec4<f32> = u_xlat0;
        let x_2148 : vec4<f32> = u_xlat1;
        u_xlat5 = (x_2147 + x_2148);
      }
    }
    u_xlat8.x = 0.0f;
    u_xlat8.y = 1.0f;
  }
  let x_2152 : vec4<f32> = u_xlat5;
  let x_2155 : vec4<f32> = x_76.unity_MatrixVP[1i];
  u_xlat0 = (vec4<f32>(x_2152.y, x_2152.y, x_2152.y, x_2152.y) * x_2155);
  let x_2158 : vec4<f32> = x_76.unity_MatrixVP[0i];
  let x_2159 : vec4<f32> = u_xlat5;
  let x_2162 : vec4<f32> = u_xlat0;
  u_xlat0 = ((x_2158 * vec4<f32>(x_2159.x, x_2159.x, x_2159.x, x_2159.x)) + x_2162);
  let x_2165 : vec4<f32> = x_76.unity_MatrixVP[2i];
  let x_2166 : vec4<f32> = u_xlat5;
  let x_2169 : vec4<f32> = u_xlat0;
  u_xlat0 = ((x_2165 * vec4<f32>(x_2166.z, x_2166.z, x_2166.z, x_2166.z)) + x_2169);
  let x_2172 : vec4<f32> = x_76.unity_MatrixVP[3i];
  let x_2173 : vec4<f32> = u_xlat5;
  let x_2176 : vec4<f32> = u_xlat0;
  u_xlat0 = ((x_2172 * vec4<f32>(x_2173.w, x_2173.w, x_2173.w, x_2173.w)) + x_2176);
  let x_2180 : f32 = u_xlat0.z;
  u_xlat28 = (x_2180 + 1.0f);
  let x_2182 : f32 = u_xlat28;
  u_xlat28 = ((x_2182 * 0.19999998807907104492f) + 0.60000002384185791016f);
  let x_2188 : f32 = u_xlat0.w;
  let x_2189 : f32 = u_xlat28;
  gl_Position.z = (x_2188 * x_2189);
  let x_2195 : vec3<f32> = in_NORMAL0;
  let x_2197 : vec4<f32> = x_1804.unity_ObjectToWorld[0i];
  u_xlat1.x = dot(x_2195, vec3<f32>(x_2197.x, x_2197.y, x_2197.z));
  let x_2201 : vec3<f32> = in_NORMAL0;
  let x_2203 : vec4<f32> = x_1804.unity_ObjectToWorld[1i];
  u_xlat1.y = dot(x_2201, vec3<f32>(x_2203.x, x_2203.y, x_2203.z));
  let x_2207 : vec3<f32> = in_NORMAL0;
  let x_2209 : vec4<f32> = x_1804.unity_ObjectToWorld[2i];
  u_xlat1.z = dot(x_2207, vec3<f32>(x_2209.x, x_2209.y, x_2209.z));
  let x_2213 : vec4<f32> = u_xlat1;
  let x_2215 : vec4<f32> = u_xlat1;
  u_xlat28 = dot(vec3<f32>(x_2213.x, x_2213.y, x_2213.z), vec3<f32>(x_2215.x, x_2215.y, x_2215.z));
  let x_2218 : f32 = u_xlat28;
  u_xlat28 = inverseSqrt(x_2218);
  let x_2220 : f32 = u_xlat28;
  let x_2222 : vec4<f32> = u_xlat1;
  vs_TEXCOORD1 = (vec3<f32>(x_2220, x_2220, x_2220) * vec3<f32>(x_2222.x, x_2222.y, x_2222.z));
  let x_2225 : vec4<f32> = u_xlat0;
  let x_2226 : vec3<f32> = vec3<f32>(x_2225.x, x_2225.y, x_2225.w);
  let x_2228 : vec4<f32> = gl_Position;
  gl_Position = vec4<f32>(x_2226.x, x_2226.y, x_2228.z, x_2226.z);
  let x_2230 : vec4<f32> = in_COLOR0;
  vs_COLOR0 = x_2230;
  let x_2233 : vec2<f32> = in_TEXCOORD0;
  let x_2234 : vec4<f32> = u_xlat8;
  u_xlat8 = vec4<f32>(x_2234.x, x_2234.y, x_2233.x, x_2233.y);
  let x_2236 : vec4<f32> = u_xlat8;
  phase0_Output0_3 = vec4<f32>(x_2236.z, x_2236.w, x_2236.x, x_2236.y);
  let x_2238 : vec4<f32> = phase0_Output0_3;
  vs_TEXCOORD0 = vec2<f32>(x_2238.x, x_2238.y);
  let x_2240 : vec4<f32> = phase0_Output0_3;
  vs_TEXCOORD2 = vec2<f32>(x_2240.z, x_2240.w);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0)
  vs_COLOR0_1 : vec4<f32>,
  @location(1)
  vs_TEXCOORD0_1 : vec2<f32>,
  @location(2)
  vs_TEXCOORD2_1 : vec2<f32>,
}

@vertex
fn main(@location(2) in_COLOR0_param : vec4<f32>, @location(0) in_POSITION0_param : vec4<f32>, @location(1) in_NORMAL0_param : vec3<f32>, @location(3) in_TEXCOORD0_param : vec2<f32>) -> main_out {
  in_COLOR0 = in_COLOR0_param;
  in_POSITION0 = in_POSITION0_param;
  in_NORMAL0 = in_NORMAL0_param;
  in_TEXCOORD0 = in_TEXCOORD0_param;
  main_1();
  return main_out(gl_Position, vs_COLOR0, vs_TEXCOORD0, vs_TEXCOORD2);
}

