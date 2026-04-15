diagnostic(off, derivative_uniformity);

var<private> u_xlat0 : vec4<f32>;

struct _positionNormalBuffer_type {
  value : array<u32, 4u>,
}

struct _positionNormalBuffer {
  _positionNormalBuffer_buf : array<_positionNormalBuffer_type>,
}

@group(0u) @binding(5u) var<storage, read> v : _positionNormalBuffer;

var<private> u_xlat1 : vec4<f32>;

var<private> u_xlat2 : vec4<f32>;

struct VGlobals_1 {
  _WorldSpaceCameraPos : vec3<f32>,
  unity_MatrixInvV : mat4x4<f32>,
  unity_MatrixVP : mat4x4<f32>,
  _ProbeSize : f32,
  _ForceDebugNormalViewBias : i32,
}

@group(1u) @binding(1u) var<uniform> v_1 : VGlobals_1;

var<private> u_xlat42 : f32;

var<private> u_xlat3 : vec3<f32>;

struct ShaderVariablesProbeVolumes {
  _PoolDim_CellInMeters : vec4<f32>,
  _RcpPoolDim_Padding : vec4<f32>,
  _MinEntryPos_Noise : vec4<f32>,
  _IndicesDim_IndexChunkSize : vec4<f32>,
  _Biases_CellInMinBrick_MinBrickSize : vec4<f32>,
  _LeakReductionParams : vec4<f32>,
  _Weight_MinLoadedCellInEntries : vec4<f32>,
  _MaxLoadedCellInEntries_FrameIndex : vec4<f32>,
  _NormalizationClamp_IndirectionEntryDim_Padding : vec4<f32>,
}

@group(1u) @binding(3u) var<uniform> v_2 : ShaderVariablesProbeVolumes;

var<private> u_xlatb4 : vec3<bool>;

var<private> u_xlatb5 : vec3<bool>;

var<private> u_xlatb42 : bool;

var<private> u_xlat4 : vec3<f32>;

var<private> u_xlati4 : vec3<i32>;

var<private> u_xlati5 : vec3<i32>;

var<private> u_xlati43 : i32;

var<private> u_xlati45 : i32;

var<private> u_xlatu4 : vec4<u32>;

struct _APVResCellIndices_type {
  value : array<u32, 3u>,
}

struct _APVResCellIndices {
  _APVResCellIndices_buf : array<_APVResCellIndices_type>,
}

@group(0u) @binding(3u) var<storage, read> v_3 : _APVResCellIndices;

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

struct _APVResIndex_type {
  value : array<u32, 1u>,
}

struct _APVResIndex {
  _APVResIndex_buf : array<_APVResIndex_type>,
}

@group(0u) @binding(2u) var<storage, read> v_4 : _APVResIndex;

var<private> u_xlatu43 : u32;

var<private> u_xlat5 : vec4<f32>;

var<private> u_xlat6 : vec4<f32>;

var<private> u_xlat8 : vec4<f32>;

@group(0u) @binding(4u) var _APVResValidity : texture_3d<f32>;

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

var<private> u_xlatb8 : vec4<bool>;

var<private> u_xlat18 : vec3<f32>;

var<private> u_xlatb45 : bool;

var<private> u_xlat32 : f32;

var<private> u_xlatb32 : bool;

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

@group(1u) @binding(2u) var<uniform> v_5 : UnityPerDraw_1;

var<private> u_xlatb0 : bool;

struct gl_PerVertex {
  gl_Position : vec4<f32>,
  gl_PointSize : f32,
  gl_ClipDistance : array<f32, 1u>,
}

var<private> v_6 : gl_PerVertex;

var<private> vs_COLOR0 : vec4<f32>;

var<private> phase0_Output0_3 : vec4<f32>;

var<private> vs_TEXCOORD1 : vec3<f32>;

var<private> vs_TEXCOORD0 : vec2<f32>;

var<private> vs_TEXCOORD2 : vec2<f32>;

var<private> u_xlat28 : f32;

fn main_inner(in_COLOR0 : vec4<f32>, in_POSITION0 : vec4<f32>, in_NORMAL0 : vec3<f32>, in_TEXCOORD0 : vec2<f32>) {
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
  var v_7 : vec3<u32>;
  var v_8 : vec3<u32>;
  var v_9 : vec2<u32>;
  var v_10 : vec3<f32>;
  var v_11 : f32;
  var v_12 : f32;
  var v_13 : f32;
  var v_14 : f32;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  let v_15 = vec3<f32>(bitcast<f32>(v._positionNormalBuffer_buf[0i].value[0i]), bitcast<f32>(v._positionNormalBuffer_buf[0i].value[1i]), bitcast<f32>(v._positionNormalBuffer_buf[0i].value[2i]));
  u_xlat0 = vec4<f32>(v_15.xyz, u_xlat0.w);
  let v_16 = vec3<f32>(bitcast<f32>(v._positionNormalBuffer_buf[1i].value[0i]), bitcast<f32>(v._positionNormalBuffer_buf[1i].value[1i]), bitcast<f32>(v._positionNormalBuffer_buf[1i].value[2i]));
  u_xlat1 = vec4<f32>(v_16.xyz, u_xlat1.w);
  let v_17 = (-(u_xlat0.xyz) + v_1._WorldSpaceCameraPos);
  u_xlat2 = vec4<f32>(v_17.xyz, u_xlat2.w);
  u_xlat42 = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat42 = inverseSqrt(u_xlat42);
  let v_18 = u_xlat42;
  let v_19 = (vec3<f32>(v_18, v_18, v_18) * u_xlat2.xyz);
  u_xlat2 = vec4<f32>(v_19.xyz, u_xlat2.w);
  u_xlat3 = ((u_xlat1.xyz * v_2._Biases_CellInMinBrick_MinBrickSize.xxx) + u_xlat0.xyz);
  let v_20 = ((u_xlat2.xyz * v_2._Biases_CellInMinBrick_MinBrickSize.yyy) + u_xlat3);
  u_xlat2 = vec4<f32>(v_20.xyz, u_xlat2.w);
  u_xlat3 = (u_xlat2.xyz / v_2._NormalizationClamp_IndirectionEntryDim_Padding.zzz);
  u_xlat3 = floor(u_xlat3);
  u_xlatb4 = ((u_xlat3.xyzx >= v_2._Weight_MinLoadedCellInEntries.yzwy)).xyz;
  u_xlatb5 = ((v_2._MaxLoadedCellInEntries_FrameIndex.xyzx >= u_xlat3.xyzx)).xyz;
  u_xlatb4.x = (u_xlatb4.x & u_xlatb5.x);
  u_xlatb4.y = (u_xlatb4.y & u_xlatb5.y);
  u_xlatb4.z = (u_xlatb4.z & u_xlatb5.z);
  u_xlatb42 = (u_xlatb4.y & u_xlatb4.x);
  u_xlatb42 = (u_xlatb4.z & u_xlatb42);
  u_xlat4 = (u_xlat3 + -(v_2._MinEntryPos_Noise.xyz));
  u_xlati4 = vec3<i32>(u_xlat4);
  u_xlati5 = vec3<i32>(v_2._IndicesDim_IndexChunkSize.xyw);
  u_xlati43 = (u_xlati5.y * u_xlati5.x);
  u_xlati45 = ((u_xlati4.y * u_xlati5.x) + u_xlati4.x);
  u_xlati43 = ((u_xlati4.z * u_xlati43) + u_xlati45);
  let v_21 = vec3<u32>(v_3._APVResCellIndices_buf[u_xlati43].value[0i], v_3._APVResCellIndices_buf[u_xlati43].value[1i], v_3._APVResCellIndices_buf[u_xlati43].value[2i]);
  u_xlatu4 = vec4<u32>(v_21.xyz, u_xlatu4.w);
  u_xlatb43 = (bitcast<i32>(u_xlatu4.x) != -1i);
  u_xlatu45 = (u_xlatu4.x >> 29u);
  u_xlat45 = f32(u_xlatu45);
  u_xlat45 = (u_xlat45 * 1.58496248722076416016f);
  u_xlat45 = exp2(u_xlat45);
  u_xlatu6.w = bitcast<u32>(i32(u_xlat45));
  let v_22 = (u_xlatu4.xyz & vec3<u32>(536870911u, 1023u, 1023u));
  u_xlatu6 = vec4<u32>(v_22.xyz, u_xlatu6.w);
  param = u_xlatu4.y;
  param_1 = 10i;
  param_2 = 10i;
  let v_24 = v_23(&(param), &(param_1), &(param_2));
  param_3 = u_xlatu4.z;
  param_4 = 10i;
  param_5 = 10i;
  let v_25 = v_23(&(param_3), &(param_4), &(param_5));
  param_6 = u_xlatu4.z;
  param_7 = 20i;
  param_8 = 10i;
  let v_26 = v_23(&(param_6), &(param_7), &(param_8));
  param_9 = u_xlatu4.y;
  param_10 = 20i;
  param_11 = 10i;
  u_xlatu4 = vec4<u32>(v_24, v_25, v_26, v_23(&(param_9), &(param_10), &(param_11)));
  u_xlatu7.x = u_xlatu6.y;
  let v_27 = u_xlatu4.xw;
  let v_28 = u_xlatu7;
  u_xlatu7 = vec4<u32>(v_28.x, v_27.xy, v_28.w);
  if (u_xlatb43) {
    v_7 = u_xlatu7.xyz;
  } else {
    v_7 = vec3<u32>(4294967295u);
  }
  let v_29 = v_7;
  u_xlatu5 = vec4<u32>(v_29.xy, u_xlatu5.z, v_29.z);
  u_xlatu4.x = u_xlatu6.z;
  if (u_xlatb43) {
    v_8 = u_xlatu4.xyz;
  } else {
    v_8 = vec3<u32>(4294967295u);
  }
  let v_30 = v_8;
  u_xlatu4 = vec4<u32>(v_30.xyz, u_xlatu4.w);
  if (u_xlatb43) {
    v_9 = u_xlatu6.xw;
  } else {
    v_9 = vec2<u32>(4294967295u);
  }
  let v_31 = v_9;
  u_xlatu6 = vec4<u32>(v_31.xy, u_xlatu6.zw);
  u_xlatb42 = (u_xlatb42 & u_xlatb43);
  u_xlat3 = ((-(u_xlat3) * v_2._NormalizationClamp_IndirectionEntryDim_Padding.zzz) + u_xlat2.xyz);
  u_xlat43 = f32(bitcast<i32>(u_xlatu6.y));
  u_xlat43 = (u_xlat43 * v_2._Biases_CellInMinBrick_MinBrickSize.w);
  let v_32 = u_xlat3;
  let v_33 = u_xlat43;
  u_xlat3 = (v_32 / vec3<f32>(v_33, v_33, v_33));
  u_xlat3 = floor(u_xlat3);
  u_xlati3 = vec3<i32>(u_xlat3);
  u_xlatb20 = ((u_xlati3.xyzz >= bitcast<vec4<i32>>(u_xlatu5.xyww))).xyz;
  u_xlatb7 = ((u_xlati3.xyzx < bitcast<vec4<i32>>(u_xlatu4.xyzx))).xyz;
  u_xlatb20.x = (u_xlatb20.x & u_xlatb7.x);
  u_xlatb20.y = (u_xlatb20.y & u_xlatb7.y);
  u_xlatb20.z = (u_xlatb20.z & u_xlatb7.z);
  u_xlatb43 = (u_xlatb20.y & u_xlatb20.x);
  u_xlati7.x = bitcast<i32>(((select(0u, 1u, u_xlatb20.z) * 4294967295u) & (select(0u, 1u, u_xlatb43) * 4294967295u)));
  let v_34 = (-(bitcast<vec2<i32>>(u_xlatu5.xy)) + bitcast<vec2<i32>>(u_xlatu4.xy));
  u_xlati4 = vec3<i32>(v_34.xy, u_xlati4.z);
  u_xlati3 = (-(bitcast<vec3<i32>>(u_xlatu5.xyw)) + u_xlati3);
  u_xlati43 = (u_xlati4.y * u_xlati4.x);
  u_xlati3.x = ((u_xlati3.x * u_xlati4.y) + u_xlati3.y);
  u_xlati43 = ((u_xlati3.z * u_xlati43) + u_xlati3.x);
  u_xlati7.y = ((bitcast<i32>(u_xlatu6.x) * u_xlati5.z) + u_xlati43);
  let v_35 = u_xlatb42;
  let v_36 = bitcast<vec2<i32>>(((select(vec2<u32>(), vec2<u32>(1u), vec2<bool>(v_35, v_35)) * vec2<u32>(4294967295u, 4294967295u)) & bitcast<vec2<u32>>(u_xlati7)));
  u_xlati3 = vec3<i32>(v_36.xy, u_xlati3.z);
  u_xlatu42 = v_4._APVResIndex_buf[u_xlati3.y].value[0i];
  let v_37 = (u_xlati3.x != 0i);
  u_xlatu42 = select(4294967295u, u_xlatu42, v_37);
  u_xlatu43 = (u_xlatu42 >> 28u);
  u_xlat43 = f32(u_xlatu43);
  u_xlat43 = (u_xlat43 * 1.58496248722076416016f);
  u_xlat43 = exp2(u_xlat43);
  u_xlatu42 = (u_xlatu42 & 268435455u);
  u_xlat42 = f32(u_xlatu42);
  u_xlat3.x = (u_xlat42 * v_2._RcpPoolDim_Padding.w);
  u_xlat3.z = floor(u_xlat3.x);
  u_xlat45 = (v_2._PoolDim_CellInMeters.y * v_2._PoolDim_CellInMeters.x);
  u_xlat42 = ((-(u_xlat3.z) * u_xlat45) + u_xlat42);
  u_xlat45 = (u_xlat42 * v_2._RcpPoolDim_Padding.x);
  u_xlat3.y = floor(u_xlat45);
  u_xlat42 = ((-(u_xlat3.y) * v_2._PoolDim_CellInMeters.x) + u_xlat42);
  u_xlat3.x = floor(u_xlat42);
  u_xlat4 = (u_xlat2.xyz / v_2._Biases_CellInMinBrick_MinBrickSize.www);
  let v_38 = u_xlat4;
  let v_39 = u_xlat43;
  u_xlat4 = (v_38 / vec3<f32>(v_39, v_39, v_39));
  u_xlat4 = fract(u_xlat4);
  u_xlat3 = (u_xlat3 + vec3<f32>(0.5f));
  u_xlat3 = ((u_xlat4 * vec3<f32>(3.0f)) + u_xlat3);
  u_xlat3 = (u_xlat3 * v_2._RcpPoolDim_Padding.xyz);
  u_xlat42 = (u_xlat43 * v_2._Biases_CellInMinBrick_MinBrickSize.w);
  let v_40 = u_xlat42;
  u_xlat4 = (vec3<f32>(v_40, v_40, v_40) * vec3<f32>(0.3333333432674407959f, 0.3333333432674407959f, 0.0f));
  let v_41 = (u_xlat2.xyz / u_xlat4.xxx);
  u_xlat5 = vec4<f32>(v_41.xyz, u_xlat5.w);
  let v_42 = fract(u_xlat5.xyz);
  u_xlat6 = vec4<f32>(v_42.xyz, u_xlat6.w);
  let v_43 = (u_xlat5.xyz + -(u_xlat6.xyz));
  u_xlat5 = vec4<f32>(v_43.xyz, u_xlat5.w);
  let v_44 = (u_xlat4.xxx * u_xlat5.xyz);
  u_xlat6 = vec4<f32>(u_xlat6.x, v_44.xyz);
  u_xlat3 = ((u_xlat3 * v_2._PoolDim_CellInMeters.xyz) + vec3<f32>(-0.5f));
  let v_45 = bitcast<vec3<u32>>(vec3<i32>(u_xlat3));
  u_xlatu7 = vec4<u32>(v_45.xyz, u_xlatu7.w);
  u_xlat3 = fract(u_xlat3);
  let v_46 = (-(u_xlat3) + vec3<f32>(1.0f));
  u_xlat8 = vec4<f32>(v_46.xyz, u_xlat8.w);
  u_xlatu7.w = 0u;
  u_xlat43 = textureLoad(_APVResValidity, bitcast<vec3<i32>>(u_xlatu7.xyz), bitcast<i32>(u_xlatu7.w)).x;
  u_xlat43 = (u_xlat43 * 255.0f);
  u_xlatu43 = u32(u_xlat43);
  u_xlat45 = (u_xlat8.y * u_xlat8.x);
  u_xlat46 = (u_xlat8.z * u_xlat45);
  let v_47 = u_xlatu43;
  u_xlatu7 = (vec4<u32>(v_47, v_47, v_47, v_47) & vec4<u32>(1u, 2u, 4u, 8u));
  u_xlat6.x = f32(bitcast<i32>(u_xlatu7.x));
  let v_48 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat0.xyz));
  u_xlat9 = vec4<f32>(v_48.xyz, u_xlat9.w);
  u_xlat47 = dot(u_xlat9.xyz, u_xlat9.xyz);
  u_xlat47 = inverseSqrt(u_xlat47);
  let v_49 = u_xlat47;
  let v_50 = (vec3<f32>(v_49, v_49, v_49) * u_xlat9.xyz);
  u_xlat10 = vec4<f32>(v_50.xyz, u_xlat10.w);
  u_xlat47 = dot(u_xlat10.xyz, u_xlat1.xyz);
  u_xlat47 = (u_xlat47 + -(v_2._LeakReductionParams.z));
  u_xlat47 = clamp(u_xlat47, 0.0f, 1.0f);
  u_xlat47 = (u_xlat6.x * u_xlat47);
  let v_51 = (u_xlat3.xy * u_xlat8.yx);
  u_xlat8 = vec4<f32>(v_51.xy, u_xlat8.zw);
  let v_52 = (u_xlat8.zz * u_xlat8.xy);
  u_xlat10 = vec4<f32>(v_52.xy, u_xlat10.zw);
  let v_53 = min(u_xlatu7.yzw, vec3<u32>(1u));
  u_xlatu7 = vec4<u32>(v_53.xyz, u_xlatu7.w);
  let v_54 = vec3<f32>(bitcast<vec3<i32>>(u_xlatu7.xyz));
  u_xlat7 = vec4<f32>(v_54.xyz, u_xlat7.w);
  let v_55 = u_xlat42;
  let v_56 = ((vec3<f32>(v_55, v_55, v_55) * vec3<f32>(0.3333333432674407959f, 0.0f, 0.0f)) + u_xlat9.xyz);
  u_xlat11 = vec4<f32>(v_56.xyz, u_xlat11.w);
  u_xlat50 = dot(u_xlat11.xyz, u_xlat11.xyz);
  u_xlat50 = inverseSqrt(u_xlat50);
  let v_57 = u_xlat50;
  let v_58 = (vec3<f32>(v_57, v_57, v_57) * u_xlat11.xyz);
  u_xlat11 = vec4<f32>(v_58.xyz, u_xlat11.w);
  u_xlat50 = dot(u_xlat11.xyz, u_xlat1.xyz);
  u_xlat50 = (u_xlat50 + -(v_2._LeakReductionParams.z));
  u_xlat50 = clamp(u_xlat50, 0.0f, 1.0f);
  u_xlat50 = (u_xlat7.x * u_xlat50);
  u_xlat11.x = (u_xlat50 * u_xlat10.x);
  u_xlat46 = ((u_xlat46 * u_xlat47) + u_xlat11.x);
  let v_59 = u_xlat42;
  let v_60 = ((vec3<f32>(v_59, v_59, v_59) * vec3<f32>(0.0f, 0.3333333432674407959f, 0.0f)) + u_xlat9.xyz);
  u_xlat10 = vec4<f32>(v_60.x, u_xlat10.y, v_60.yz);
  u_xlat47 = dot(u_xlat10.xzw, u_xlat10.xzw);
  u_xlat47 = inverseSqrt(u_xlat47);
  let v_61 = u_xlat47;
  let v_62 = (vec3<f32>(v_61, v_61, v_61) * u_xlat10.xzw);
  u_xlat10 = vec4<f32>(v_62.x, u_xlat10.y, v_62.yz);
  u_xlat47 = dot(u_xlat10.xzw, u_xlat1.xyz);
  u_xlat47 = (u_xlat47 + -(v_2._LeakReductionParams.z));
  u_xlat47 = clamp(u_xlat47, 0.0f, 1.0f);
  u_xlat47 = (u_xlat7.y * u_xlat47);
  u_xlat11.y = (u_xlat47 * u_xlat10.y);
  u_xlat46 = ((u_xlat10.y * u_xlat47) + u_xlat46);
  u_xlat47 = (u_xlat3.y * u_xlat3.x);
  u_xlat36 = (u_xlat8.z * u_xlat47);
  let v_63 = u_xlat42;
  let v_64 = ((vec3<f32>(v_63, v_63, v_63) * vec3<f32>(0.3333333432674407959f, 0.3333333432674407959f, 0.0f)) + u_xlat9.xyz);
  u_xlat10 = vec4<f32>(v_64.xyz, u_xlat10.w);
  u_xlat50 = dot(u_xlat10.xyz, u_xlat10.xyz);
  u_xlat50 = inverseSqrt(u_xlat50);
  let v_65 = u_xlat50;
  let v_66 = (vec3<f32>(v_65, v_65, v_65) * u_xlat10.xyz);
  u_xlat10 = vec4<f32>(v_66.xyz, u_xlat10.w);
  u_xlat50 = dot(u_xlat10.xyz, u_xlat1.xyz);
  u_xlat50 = (u_xlat50 + -(v_2._LeakReductionParams.z));
  u_xlat50 = clamp(u_xlat50, 0.0f, 1.0f);
  u_xlat50 = (u_xlat7.z * u_xlat50);
  u_xlat11.z = (u_xlat50 * u_xlat36);
  u_xlat46 = ((u_xlat36 * u_xlat50) + u_xlat46);
  u_xlat45 = (u_xlat3.z * u_xlat45);
  let v_67 = u_xlatu43;
  u_xlatu10 = (vec4<u32>(v_67, v_67, v_67, v_67) & vec4<u32>(16u, 32u, 64u, 128u));
  u_xlatu10 = min(u_xlatu10, vec4<u32>(1u));
  u_xlat10 = vec4<f32>(bitcast<vec4<i32>>(u_xlatu10.yxzw));
  let v_68 = u_xlat42;
  u_xlat12 = ((vec3<f32>(v_68, v_68, v_68) * vec3<f32>(0.0f, 0.0f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat43 = dot(u_xlat12, u_xlat12);
  u_xlat43 = inverseSqrt(u_xlat43);
  let v_69 = u_xlat43;
  u_xlat12 = (vec3<f32>(v_69, v_69, v_69) * u_xlat12);
  u_xlat43 = dot(u_xlat12, u_xlat1.xyz);
  u_xlat43 = (u_xlat43 + -(v_2._LeakReductionParams.z));
  u_xlat43 = clamp(u_xlat43, 0.0f, 1.0f);
  u_xlat43 = (u_xlat10.y * u_xlat43);
  u_xlat12.x = (u_xlat43 * u_xlat45);
  u_xlat43 = ((u_xlat45 * u_xlat43) + u_xlat46);
  let v_70 = (u_xlat3.zz * u_xlat8.xy);
  u_xlat8 = vec4<f32>(v_70.xy, u_xlat8.zw);
  let v_71 = u_xlat42;
  u_xlat13 = ((vec3<f32>(v_71, v_71, v_71) * vec3<f32>(0.3333333432674407959f, 0.0f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat45 = dot(u_xlat13, u_xlat13);
  u_xlat45 = inverseSqrt(u_xlat45);
  let v_72 = u_xlat45;
  u_xlat13 = (vec3<f32>(v_72, v_72, v_72) * u_xlat13);
  u_xlat45 = dot(u_xlat13, u_xlat1.xyz);
  u_xlat45 = (u_xlat45 + -(v_2._LeakReductionParams.z));
  u_xlat45 = clamp(u_xlat45, 0.0f, 1.0f);
  u_xlat45 = (u_xlat10.x * u_xlat45);
  u_xlat12.y = (u_xlat45 * u_xlat8.x);
  u_xlat43 = ((u_xlat8.x * u_xlat45) + u_xlat43);
  let v_73 = u_xlat42;
  let v_74 = ((vec3<f32>(v_73, v_73, v_73) * vec3<f32>(0.0f, 0.3333333432674407959f, 0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat8 = vec4<f32>(v_74.x, u_xlat8.y, v_74.yz);
  u_xlat45 = dot(u_xlat8.xzw, u_xlat8.xzw);
  u_xlat45 = inverseSqrt(u_xlat45);
  let v_75 = u_xlat45;
  let v_76 = (vec3<f32>(v_75, v_75, v_75) * u_xlat8.xzw);
  u_xlat8 = vec4<f32>(v_76.x, u_xlat8.y, v_76.yz);
  u_xlat45 = dot(u_xlat8.xzw, u_xlat1.xyz);
  u_xlat45 = (u_xlat45 + -(v_2._LeakReductionParams.z));
  u_xlat45 = clamp(u_xlat45, 0.0f, 1.0f);
  u_xlat45 = (u_xlat10.z * u_xlat45);
  u_xlat12.z = (u_xlat45 * u_xlat8.y);
  u_xlat43 = ((u_xlat8.y * u_xlat45) + u_xlat43);
  u_xlat45 = (u_xlat3.z * u_xlat47);
  let v_77 = u_xlat42;
  let v_78 = ((vec3<f32>(v_77, v_77, v_77) * vec3<f32>(0.3333333432674407959f)) + u_xlat9.xyz);
  u_xlat8 = vec4<f32>(v_78.xyz, u_xlat8.w);
  u_xlat46 = dot(u_xlat8.xyz, u_xlat8.xyz);
  u_xlat46 = inverseSqrt(u_xlat46);
  let v_79 = u_xlat46;
  let v_80 = (vec3<f32>(v_79, v_79, v_79) * u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_80.xyz, u_xlat8.w);
  u_xlat46 = dot(u_xlat8.xyz, u_xlat1.xyz);
  u_xlat46 = (u_xlat46 + -(v_2._LeakReductionParams.z));
  u_xlat46 = clamp(u_xlat46, 0.0f, 1.0f);
  u_xlat46 = (u_xlat10.w * u_xlat46);
  u_xlat5.w = (u_xlat45 * u_xlat46);
  u_xlat43 = ((u_xlat45 * u_xlat46) + u_xlat43);
  u_xlat43 = max(u_xlat43, 0.00009999999747378752f);
  u_xlat43 = (1.0f / u_xlat43);
  let v_81 = u_xlat43;
  let v_82 = (vec3<f32>(v_81, v_81, v_81) * u_xlat11.xyz);
  u_xlat8 = vec4<f32>(v_82.xyz, u_xlat8.w);
  let v_83 = u_xlat43;
  let v_84 = (vec3<f32>(v_83, v_83, v_83) * u_xlat12);
  u_xlat9 = vec4<f32>(v_84.xyz, u_xlat9.w);
  let v_85 = ((u_xlat8.xxx * vec3<f32>(1.0f, 0.0f, 0.0f)) + -(u_xlat3));
  u_xlat11 = vec4<f32>(v_85.xyz, u_xlat11.w);
  let v_86 = ((u_xlat8.yyy * vec3<f32>(0.0f, 1.0f, 0.0f)) + u_xlat11.xyz);
  u_xlat8 = vec4<f32>(v_86.xy, u_xlat8.z, v_86.z);
  let v_87 = ((u_xlat8.zzz * vec3<f32>(1.0f, 1.0f, 0.0f)) + u_xlat8.xyw);
  u_xlat8 = vec4<f32>(v_87.xyz, u_xlat8.w);
  let v_88 = ((u_xlat9.xxx * vec3<f32>(0.0f, 0.0f, 1.0f)) + u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_88.xyz, u_xlat8.w);
  let v_89 = ((u_xlat9.yyy * vec3<f32>(1.0f, 0.0f, 1.0f)) + u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_89.xyz, u_xlat8.w);
  let v_90 = ((u_xlat9.zzz * vec3<f32>(0.0f, 1.0f, 1.0f)) + u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_90.xyz, u_xlat8.w);
  let v_91 = u_xlat5.www;
  let v_92 = u_xlat43;
  let v_93 = ((v_91 * vec3<f32>(v_92, v_92, v_92)) + u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_93.xyz, u_xlat8.w);
  u_xlat3 = (u_xlat3 + u_xlat8.xyz);
  u_xlatb43 = !((v_2._LeakReductionParams.x == 0.0f));
  let v_94 = ((-(u_xlat5.xyz) * u_xlat4.xxx) + u_xlat2.xyz);
  u_xlat8 = vec4<f32>(v_94.xyz, u_xlat8.w);
  let v_95 = (u_xlat8.xyz / u_xlat4.xxx);
  u_xlat8 = vec4<f32>(v_95.xyz, u_xlat8.w);
  if (u_xlatb43) {
    v_10 = u_xlat3;
  } else {
    v_10 = u_xlat8.xyz;
  }
  u_xlat3 = v_10;
  let v_96 = in_COLOR0.z;
  u_xlatb43 = any(!((vec4<f32>() == vec4<f32>(v_96, v_96, v_96, v_96))));
  if (u_xlatb43) {
    u_xlat8 = (in_COLOR0.zzzz + vec4<f32>(-0.20000000298023223877f, -0.30000001192092895508f, -0.40000000596046447754f, -0.5f));
    u_xlatb8 = (abs(u_xlat8) < vec4<f32>(0.01999999955296516418f));
    let v_97 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zyz);
    u_xlat9 = vec4<f32>(u_xlat9.x, v_97.xyz);
    u_xlat9.x = u_xlat7.y;
    let v_98 = u_xlatb8.x;
    let v_99 = u_xlat9;
    u_xlat9 = select(u_xlat6, v_99, vec4<bool>(v_98, v_98, v_98, v_98));
    let v_100 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yyz);
    u_xlat11 = vec4<f32>(u_xlat11.x, v_100.xyz);
    u_xlat11.x = u_xlat7.z;
    let v_101 = u_xlatb8.y;
    let v_102 = u_xlat11;
    u_xlat9 = select(u_xlat9, v_102, vec4<bool>(v_101, v_101, v_101, v_101));
    let v_103 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yzz);
    u_xlat7 = vec4<f32>(u_xlat7.x, v_103.xyz);
    let v_104 = u_xlatb8.z;
    let v_105 = u_xlat7;
    u_xlat7 = select(u_xlat9, v_105, vec4<bool>(v_104, v_104, v_104, v_104));
    let v_106 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zzy);
    u_xlat9 = vec4<f32>(u_xlat9.x, v_106.xyz);
    u_xlat9.x = u_xlat10.y;
    let v_107 = u_xlatb8.w;
    let v_108 = u_xlat9;
    u_xlat7 = select(u_xlat7, v_108, vec4<bool>(v_107, v_107, v_107, v_107));
    u_xlat8 = vec4<f32>(((in_COLOR0.zzz + vec3<f32>(-0.60000002384185791016f, -0.69999998807907104492f, -0.80000001192092895508f))).xyz, u_xlat8.w);
    let v_109 = ((abs(u_xlat8.xyzx) < vec4<f32>(0.01999999955296516418f, 0.01999999955296516418f, 0.01999999955296516418f, 0.0f))).xyz;
    u_xlatb8 = vec4<bool>(v_109.xyz, u_xlatb8.w);
    let v_110 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.zyy);
    u_xlat9 = vec4<f32>(u_xlat9.x, v_110.xyz);
    u_xlat9.x = u_xlat10.z;
    let v_111 = u_xlatb8.x;
    let v_112 = u_xlat9;
    u_xlat7 = select(u_xlat7, v_112, vec4<bool>(v_111, v_111, v_111, v_111));
    let v_113 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.xxx);
    u_xlat9 = vec4<f32>(u_xlat9.x, v_113.xyz);
    u_xlat9.x = u_xlat10.w;
    let v_114 = u_xlatb8.y;
    let v_115 = u_xlat9;
    u_xlat7 = select(u_xlat7, v_115, vec4<bool>(v_114, v_114, v_114, v_114));
    let v_116 = ((u_xlat5.xyz * u_xlat4.xxx) + u_xlat4.yzy);
    u_xlat10 = vec4<f32>(u_xlat10.x, v_116.xyz);
    let v_117 = u_xlatb8.z;
    let v_118 = u_xlat10;
    u_xlat7 = select(u_xlat7, v_118, vec4<bool>(v_117, v_117, v_117, v_117));
    u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat7.yzw));
    u_xlat43 = dot(u_xlat18, u_xlat18);
    u_xlat43 = sqrt(u_xlat43);
    u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
    if (u_xlatb43) {
      u_xlat18 = (-(u_xlat3) + vec3<f32>(1.0f));
      u_xlat43 = (u_xlat18.y * u_xlat18.x);
      u_xlat8.x = (u_xlat18.z * u_xlat43);
    } else {
      let v_119 = u_xlat42;
      u_xlat18 = ((vec3<f32>(v_119, v_119, v_119) * vec3<f32>(-0.3333333432674407959f, 0.0f, 0.0f)) + u_xlat7.yzw);
      u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
      u_xlat43 = dot(u_xlat18, u_xlat18);
      u_xlat43 = sqrt(u_xlat43);
      u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
      if (u_xlatb43) {
        let v_120 = (-(u_xlat3.yz) + vec2<f32>(1.0f));
        u_xlat18 = vec3<f32>(v_120.xy, u_xlat18.z);
        u_xlat43 = (u_xlat3.x * u_xlat18.x);
        u_xlat8.x = (u_xlat18.y * u_xlat43);
      } else {
        let v_121 = u_xlat42;
        u_xlat18 = ((vec3<f32>(v_121, v_121, v_121) * vec3<f32>(-0.3333333432674407959f, -0.3333333432674407959f, 0.0f)) + u_xlat7.yzw);
        u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
        u_xlat43 = dot(u_xlat18, u_xlat18);
        u_xlat43 = sqrt(u_xlat43);
        u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
        if (u_xlatb43) {
          u_xlat43 = (u_xlat3.y * u_xlat3.x);
          u_xlat45 = (-(u_xlat3.z) + 1.0f);
          u_xlat8.x = (u_xlat43 * u_xlat45);
        } else {
          let v_122 = u_xlat42;
          u_xlat18 = ((vec3<f32>(v_122, v_122, v_122) * vec3<f32>(0.0f, -0.3333333432674407959f, 0.0f)) + u_xlat7.yzw);
          u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
          u_xlat43 = dot(u_xlat18, u_xlat18);
          u_xlat43 = sqrt(u_xlat43);
          u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
          if (u_xlatb43) {
            let v_123 = (-(u_xlat3.xz) + vec2<f32>(1.0f));
            u_xlat18 = vec3<f32>(v_123.xy, u_xlat18.z);
            u_xlat43 = (u_xlat3.y * u_xlat18.x);
            u_xlat8.x = (u_xlat18.y * u_xlat43);
          } else {
            let v_124 = u_xlat42;
            u_xlat18 = ((vec3<f32>(v_124, v_124, v_124) * vec3<f32>(-0.3333333432674407959f, 0.0f, -0.3333333432674407959f)) + u_xlat7.yzw);
            u_xlat18 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat18));
            u_xlat43 = dot(u_xlat18, u_xlat18);
            u_xlat43 = sqrt(u_xlat43);
            u_xlatb43 = (u_xlat43 < 0.00009999999747378752f);
            let v_125 = (-(u_xlat3.yx) + vec2<f32>(1.0f));
            u_xlat18 = vec3<f32>(v_125.xy, u_xlat18.z);
            let v_126 = (u_xlat3.xy * u_xlat18.xy);
            u_xlat9 = vec4<f32>(v_126.xy, u_xlat9.zw);
            let v_127 = u_xlat42;
            let v_128 = ((vec3<f32>(v_127, v_127, v_127) * vec3<f32>(0.0f, 0.0f, -0.3333333432674407959f)) + u_xlat7.yzw);
            u_xlat10 = vec4<f32>(v_128.xyz, u_xlat10.w);
            let v_129 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz));
            u_xlat10 = vec4<f32>(v_129.xyz, u_xlat10.w);
            u_xlat45 = dot(u_xlat10.xyz, u_xlat10.xyz);
            u_xlat45 = sqrt(u_xlat45);
            u_xlatb45 = (u_xlat45 < 0.00009999999747378752f);
            u_xlat18.x = (u_xlat18.x * u_xlat18.y);
            let v_130 = u_xlat42;
            let v_131 = ((vec3<f32>(v_130, v_130, v_130) * vec3<f32>(-0.3333333432674407959f)) + u_xlat7.yzw);
            u_xlat10 = vec4<f32>(v_131.xyz, u_xlat10.w);
            let v_132 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz));
            u_xlat10 = vec4<f32>(v_132.xyz, u_xlat10.w);
            u_xlat32 = dot(u_xlat10.xyz, u_xlat10.xyz);
            u_xlat32 = sqrt(u_xlat32);
            u_xlatb32 = (u_xlat32 < 0.00009999999747378752f);
            u_xlat18.z = (u_xlat3.y * u_xlat3.x);
            let v_133 = (u_xlat3.zz * u_xlat18.xz);
            u_xlat18 = vec3<f32>(v_133.x, u_xlat18.y, v_133.y);
            let v_134 = u_xlat42;
            let v_135 = ((vec3<f32>(v_134, v_134, v_134) * vec3<f32>(0.0f, -0.3333333432674407959f, -0.3333333432674407959f)) + u_xlat7.yzw);
            u_xlat10 = vec4<f32>(v_135.xyz, u_xlat10.w);
            let v_136 = ((u_xlat5.xyz * u_xlat4.xxx) + -(u_xlat10.xyz));
            u_xlat5 = vec4<f32>(v_136.xyz, u_xlat5.w);
            u_xlat42 = dot(u_xlat5.xyz, u_xlat5.xyz);
            u_xlat42 = sqrt(u_xlat42);
            u_xlatb42 = (u_xlat42 < 0.00009999999747378752f);
            let v_137 = (u_xlat3.zz * u_xlat9.xy);
            u_xlat5 = vec4<f32>(v_137.xy, u_xlat5.zw);
            if (u_xlatb42) {
              v_11 = u_xlat5.y;
            } else {
              v_11 = 0.0f;
            }
            u_xlat42 = v_11;
            if (u_xlatb32) {
              v_12 = u_xlat18.z;
            } else {
              v_12 = u_xlat42;
            }
            u_xlat42 = v_12;
            if (u_xlatb45) {
              v_13 = u_xlat18.x;
            } else {
              v_13 = u_xlat42;
            }
            u_xlat42 = v_13;
            if (u_xlatb43) {
              v_14 = u_xlat5.x;
            } else {
              v_14 = u_xlat42;
            }
            u_xlat8.x = v_14;
          }
        }
      }
    }
    let v_138 = v_1.unity_MatrixInvV[1i].xyz;
    let v_139 = vec3<f32>(v_1._ProbeSize, v_1._ProbeSize, v_1._ProbeSize);
    u_xlat18 = (v_138 * vec3<f32>(v_139.x, v_139.y, v_139.z));
    u_xlat18 = ((u_xlat18 * vec3<f32>(0.6666666865348815918f)) + u_xlat7.yzw);
    let v_140 = (u_xlat18 + v_5.unity_ObjectToWorld[3i].xyz);
    u_xlat5 = vec4<f32>(v_140.xyz, u_xlat5.w);
    u_xlat18 = (in_POSITION0.yyy * v_1.unity_MatrixInvV[1i].xyz);
    u_xlat18 = (u_xlat18 * vec3<f32>(0.5f));
    u_xlat18 = ((in_POSITION0.xxx * -(v_1.unity_MatrixInvV[0i].xyz)) + u_xlat18);
    let v_141 = u_xlat18;
    let v_142 = vec3<f32>(v_1._ProbeSize, v_1._ProbeSize, v_1._ProbeSize);
    u_xlat18 = (v_141 * vec3<f32>(v_142.x, v_142.y, v_142.z));
    let v_143 = (u_xlat18 * vec3<f32>(20.0f));
    u_xlat9 = vec4<f32>(v_143.xyz, u_xlat9.w);
    u_xlat5.w = v_5.unity_ObjectToWorld[3i].w;
    u_xlat9.w = 0.0f;
    u_xlat5 = (u_xlat5 + u_xlat9);
    u_xlat8.y = u_xlat7.x;
  } else {
    let v_144 = in_COLOR0.y;
    u_xlatb42 = any(!((vec4<f32>() == vec4<f32>(v_144, v_144, v_144, v_144))));
    if (u_xlatb42) {
      u_xlat42 = dot(u_xlat1.xyz, u_xlat1.xyz);
      u_xlat42 = inverseSqrt(u_xlat42);
      let v_145 = u_xlat42;
      let v_146 = (vec3<f32>(v_145, v_145, v_145) * u_xlat1.xyz);
      u_xlat1 = vec4<f32>(v_146.xyz, u_xlat1.w);
      u_xlatb42 = (0.89999997615814208984f < u_xlat1.y);
      let v_147 = u_xlatb42;
      u_xlat18 = select(vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 1.0f), vec3<bool>(v_147, v_147, v_147));
      let v_148 = (u_xlat1.zxy * u_xlat18);
      u_xlat7 = vec4<f32>(v_148.xyz, u_xlat7.w);
      u_xlat18 = ((u_xlat1.yzx * u_xlat18.yzx) + -(u_xlat7.xyz));
      u_xlat42 = dot(u_xlat18, u_xlat18);
      u_xlat42 = inverseSqrt(u_xlat42);
      let v_149 = u_xlat42;
      u_xlat18 = (vec3<f32>(v_149, v_149, v_149) * u_xlat18);
      let v_150 = (u_xlat1.yzx * u_xlat18.zxy);
      u_xlat7 = vec4<f32>(v_150.xyz, u_xlat7.w);
      let v_151 = ((u_xlat18.yzx * u_xlat1.zxy) + -(u_xlat7.xyz));
      u_xlat7 = vec4<f32>(v_151.xyz, u_xlat7.w);
      let v_152 = vec3<f32>(v_1._ProbeSize, v_1._ProbeSize, v_1._ProbeSize);
      u_xlat9 = vec4<f32>(((in_POSITION0.xyz * vec3<f32>(v_152.x, v_152.y, v_152.z))).xyz, u_xlat9.w);
      let v_153 = (u_xlat9.xyz * vec3<f32>(5.0f));
      u_xlat9 = vec4<f32>(v_153.xyz, u_xlat9.w);
      let v_154 = (u_xlat7.xyz * u_xlat9.yyy);
      u_xlat7 = vec4<f32>(v_154.xyz, u_xlat7.w);
      u_xlat18 = ((u_xlat18 * u_xlat9.xxx) + u_xlat7.xyz);
      let v_155 = ((u_xlat1.xyz * u_xlat9.zzz) + u_xlat18);
      u_xlat1 = vec4<f32>(v_155.xyz, u_xlat1.w);
      u_xlat7 = (u_xlat1.yyyy * v_5.unity_ObjectToWorld[1i]);
      u_xlat7 = ((v_5.unity_ObjectToWorld[0i] * u_xlat1.xxxx) + u_xlat7);
      u_xlat1 = ((v_5.unity_ObjectToWorld[2i] * u_xlat1.zzzz) + u_xlat7);
      u_xlat5 = (u_xlat1 + v_5.unity_ObjectToWorld[3i]);
      let v_156 = (u_xlat0.xyz + u_xlat5.xyz);
      u_xlat5 = vec4<f32>(v_156.xyz, u_xlat5.w);
    } else {
      let v_157 = in_COLOR0.x;
      u_xlatb0 = any(!((vec4<f32>() == vec4<f32>(v_157, v_157, v_157, v_157))));
      if (u_xlatb0) {
        if ((v_1._ForceDebugNormalViewBias != 0i)) {
          let v_158 = vec3<f32>(v_1._ProbeSize, v_1._ProbeSize, v_1._ProbeSize);
          u_xlat0 = vec4<f32>(((in_POSITION0.xyz * vec3<f32>(v_158.x, v_158.y, v_158.z))).xyz, u_xlat0.w);
          let v_159 = (u_xlat0.xyz * vec3<f32>(1.5f));
          u_xlat0 = vec4<f32>(v_159.xyz, u_xlat0.w);
          u_xlat1 = (u_xlat0.yyyy * v_5.unity_ObjectToWorld[1i]);
          u_xlat1 = ((v_5.unity_ObjectToWorld[0i] * u_xlat0.xxxx) + u_xlat1);
          u_xlat0 = ((v_5.unity_ObjectToWorld[2i] * u_xlat0.zzzz) + u_xlat1);
          u_xlat0 = (u_xlat0 + v_5.unity_ObjectToWorld[3i]);
          u_xlat2.w = 0.0f;
          u_xlat5 = (u_xlat0 + u_xlat2);
        } else {
          v_6.gl_Position = vec4<f32>();
          vs_COLOR0 = vec4<f32>();
          phase0_Output0_3 = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
          vs_TEXCOORD1 = vec3<f32>();
          vs_TEXCOORD0 = phase0_Output0_3.xy;
          vs_TEXCOORD2 = phase0_Output0_3.zw;
          return;
        }
      } else {
        let v_160 = vec3<f32>(v_1._ProbeSize, v_1._ProbeSize, v_1._ProbeSize);
        u_xlat0 = vec4<f32>(((in_POSITION0.xyz * vec3<f32>(v_160.x, v_160.y, v_160.z))).xyz, u_xlat0.w);
        let v_161 = (u_xlat0.xyz * vec3<f32>(3.0f));
        u_xlat0 = vec4<f32>(v_161.xyz, u_xlat0.w);
        u_xlat1 = (u_xlat0.yyyy * v_5.unity_ObjectToWorld[1i]);
        u_xlat1 = ((v_5.unity_ObjectToWorld[0i] * u_xlat0.xxxx) + u_xlat1);
        u_xlat0 = ((v_5.unity_ObjectToWorld[2i] * u_xlat0.zzzz) + u_xlat1);
        u_xlat0 = (u_xlat0 + v_5.unity_ObjectToWorld[3i]);
        let v_162 = ((u_xlat3 * u_xlat4.xxx) + u_xlat6.yzw);
        u_xlat1 = vec4<f32>(v_162.xyz, u_xlat1.w);
        u_xlat1.w = 0.0f;
        u_xlat5 = (u_xlat0 + u_xlat1);
      }
    }
    u_xlat8.x = 0.0f;
    u_xlat8.y = 1.0f;
  }
  u_xlat0 = (u_xlat5.yyyy * v_1.unity_MatrixVP[1i]);
  u_xlat0 = ((v_1.unity_MatrixVP[0i] * u_xlat5.xxxx) + u_xlat0);
  u_xlat0 = ((v_1.unity_MatrixVP[2i] * u_xlat5.zzzz) + u_xlat0);
  u_xlat0 = ((v_1.unity_MatrixVP[3i] * u_xlat5.wwww) + u_xlat0);
  u_xlat28 = (u_xlat0.z + 1.0f);
  u_xlat28 = ((u_xlat28 * 0.19999998807907104492f) + 0.60000002384185791016f);
  v_6.gl_Position.z = (u_xlat0.w * u_xlat28);
  u_xlat1.x = dot(in_NORMAL0, v_5.unity_ObjectToWorld[0i].xyz);
  u_xlat1.y = dot(in_NORMAL0, v_5.unity_ObjectToWorld[1i].xyz);
  u_xlat1.z = dot(in_NORMAL0, v_5.unity_ObjectToWorld[2i].xyz);
  u_xlat28 = dot(u_xlat1.xyz, u_xlat1.xyz);
  u_xlat28 = inverseSqrt(u_xlat28);
  let v_163 = u_xlat28;
  vs_TEXCOORD1 = (vec3<f32>(v_163, v_163, v_163) * u_xlat1.xyz);
  let v_164 = u_xlat0.xyw;
  let v_165 = &(v_6.gl_Position);
  *(v_165) = vec4<f32>(v_164.xy, (*(v_165)).z, v_164.z);
  vs_COLOR0 = in_COLOR0;
  u_xlat8 = vec4<f32>(u_xlat8.xy, in_TEXCOORD0.xy);
  phase0_Output0_3 = u_xlat8.zwxy;
  vs_TEXCOORD0 = phase0_Output0_3.xy;
  vs_TEXCOORD2 = phase0_Output0_3.zw;
}

fn v_23(value : ptr<function, u32>, offset : ptr<function, i32>, bits : ptr<function, i32>) -> u32 {
  return ((*(value) >> bitcast<u32>(*(offset))) & ~((4294967295u << bitcast<u32>(*(bits)))));
}

struct tint_symbol {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0u)
  vs_COLOR0 : vec4<f32>,
  @location(1u)
  vs_TEXCOORD0 : vec2<f32>,
  @location(2u)
  vs_TEXCOORD2 : vec2<f32>,
}

@vertex
fn main(@location(2u) in_COLOR0 : vec4<f32>, @location(0u) in_POSITION0 : vec4<f32>, @location(1u) in_NORMAL0 : vec3<f32>, @location(3u) in_TEXCOORD0 : vec2<f32>) -> tint_symbol {
  main_inner(in_COLOR0, in_POSITION0, in_NORMAL0, in_TEXCOORD0);
  return tint_symbol(v_6.gl_Position, vs_COLOR0, vs_TEXCOORD0, vs_TEXCOORD2);
}
