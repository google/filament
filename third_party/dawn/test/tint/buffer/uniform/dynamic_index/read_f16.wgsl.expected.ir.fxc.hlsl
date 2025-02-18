SKIP: INVALID

struct main_inputs {
  uint idx : SV_GroupIndex;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[400];
};
RWByteAddressBuffer s : register(u1);
int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = ub[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x)));
  uint4 v_7 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x)));
  uint4 v_9 = ub[((12u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(((((((12u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.z) : (v_9.x))));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_10(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 2u)) {
        break;
      }
      a[v_12] = v_2((start_byte_offset + (v_12 * 16u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_13[2] = a;
  return v_13;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_14(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 2u)) {
        break;
      }
      a[v_16] = asfloat(ub[((start_byte_offset + (v_16 * 16u)) / 16u)].xyz);
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  float3 v_17[2] = a;
  return v_17;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_18 = float16_t(t_low.x);
  float16_t v_19 = float16_t(t_high.x);
  float16_t v_20 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_18, v_19, v_20, float16_t(t_high.y));
}

matrix<float16_t, 4, 4> v_21(uint start_byte_offset) {
  vector<float16_t, 4> v_22 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_23 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]);
  vector<float16_t, 4> v_24 = tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 4, 4>(v_22, v_23, v_24, tint_bitcast_to_f16_1(ub[((24u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 4, 3> v_25(uint start_byte_offset) {
  vector<float16_t, 3> v_26 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_27 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz;
  vector<float16_t, 3> v_28 = tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 4, 3>(v_26, v_27, v_28, tint_bitcast_to_f16_1(ub[((24u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 3, 4> v_29(uint start_byte_offset) {
  vector<float16_t, 4> v_30 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_31 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 3, 4>(v_30, v_31, tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 3, 3> v_32(uint start_byte_offset) {
  vector<float16_t, 3> v_33 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_34 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 3, 3>(v_33, v_34, tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 3, 2> v_35(uint start_byte_offset) {
  uint4 v_36 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_37 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_36.z) : (v_36.x)));
  uint4 v_38 = ub[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_39 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.z) : (v_38.x)));
  uint4 v_40 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 2>(v_37, v_39, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_40.z) : (v_40.x))));
}

matrix<float16_t, 2, 4> v_41(uint start_byte_offset) {
  vector<float16_t, 4> v_42 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_42, tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 2, 3> v_43(uint start_byte_offset) {
  vector<float16_t, 3> v_44 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  return matrix<float16_t, 2, 3>(v_44, tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 2, 2> v_45(uint start_byte_offset) {
  uint4 v_46 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_47 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_46.z) : (v_46.x)));
  uint4 v_48 = ub[((4u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 2>(v_47, tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_48.z) : (v_48.x))));
}

float4x4 v_49(uint start_byte_offset) {
  float4 v_50 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_51 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  float4 v_52 = asfloat(ub[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_50, v_51, v_52, asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_53(uint start_byte_offset) {
  float3 v_54 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_55 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_56 = asfloat(ub[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_54, v_55, v_56, asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_57(uint start_byte_offset) {
  uint4 v_58 = ub[(start_byte_offset / 16u)];
  float2 v_59 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_58.zw) : (v_58.xy)));
  uint4 v_60 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_61 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint4 v_62 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_63 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_62.zw) : (v_62.xy)));
  uint4 v_64 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_59, v_61, v_63, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_64.zw) : (v_64.xy))));
}

float3x4 v_65(uint start_byte_offset) {
  float4 v_66 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_67 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_66, v_67, asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_68(uint start_byte_offset) {
  float3 v_69 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_70 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_69, v_70, asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_71(uint start_byte_offset) {
  uint4 v_72 = ub[(start_byte_offset / 16u)];
  float2 v_73 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_72.zw) : (v_72.xy)));
  uint4 v_74 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_75 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_74.zw) : (v_74.xy)));
  uint4 v_76 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_73, v_75, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_76.zw) : (v_76.xy))));
}

float2x4 v_77(uint start_byte_offset) {
  float4 v_78 = asfloat(ub[(start_byte_offset / 16u)]);
  return float2x4(v_78, asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_79(uint start_byte_offset) {
  float3 v_80 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  return float2x3(v_80, asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_81(uint start_byte_offset) {
  uint4 v_82 = ub[(start_byte_offset / 16u)];
  float2 v_83 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_82.zw) : (v_82.xy)));
  uint4 v_84 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_83, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_84.zw) : (v_84.xy))));
}

void main_inner(uint idx) {
  uint v_85 = (800u * uint(idx));
  float scalar_f32 = asfloat(ub[(v_85 / 16u)][((v_85 % 16u) / 4u)]);
  uint v_86 = (4u + (800u * uint(idx)));
  int scalar_i32 = asint(ub[(v_86 / 16u)][((v_86 % 16u) / 4u)]);
  uint v_87 = (8u + (800u * uint(idx)));
  uint scalar_u32 = ub[(v_87 / 16u)][((v_87 % 16u) / 4u)];
  uint v_88 = (12u + (800u * uint(idx)));
  uint v_89 = ub[(v_88 / 16u)][((v_88 % 16u) / 4u)];
  float16_t scalar_f16 = float16_t(f16tof32((v_89 >> ((((v_88 % 4u) == 0u)) ? (0u) : (16u)))));
  uint v_90 = (16u + (800u * uint(idx)));
  uint4 v_91 = ub[(v_90 / 16u)];
  float2 vec2_f32 = asfloat((((((v_90 % 16u) / 4u) == 2u)) ? (v_91.zw) : (v_91.xy)));
  uint v_92 = (24u + (800u * uint(idx)));
  uint4 v_93 = ub[(v_92 / 16u)];
  int2 vec2_i32 = asint((((((v_92 % 16u) / 4u) == 2u)) ? (v_93.zw) : (v_93.xy)));
  uint v_94 = (32u + (800u * uint(idx)));
  uint4 v_95 = ub[(v_94 / 16u)];
  uint2 vec2_u32 = (((((v_94 % 16u) / 4u) == 2u)) ? (v_95.zw) : (v_95.xy));
  uint v_96 = (40u + (800u * uint(idx)));
  uint4 v_97 = ub[(v_96 / 16u)];
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16((((((v_96 % 16u) / 4u) == 2u)) ? (v_97.z) : (v_97.x)));
  uint v_98 = ((48u + (800u * uint(idx))) / 16u);
  float3 vec3_f32 = asfloat(ub[v_98].xyz);
  uint v_99 = ((64u + (800u * uint(idx))) / 16u);
  int3 vec3_i32 = asint(ub[v_99].xyz);
  uint v_100 = ((80u + (800u * uint(idx))) / 16u);
  uint3 vec3_u32 = ub[v_100].xyz;
  uint v_101 = ((96u + (800u * uint(idx))) / 16u);
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(ub[v_101]).xyz;
  uint v_102 = ((112u + (800u * uint(idx))) / 16u);
  float4 vec4_f32 = asfloat(ub[v_102]);
  uint v_103 = ((128u + (800u * uint(idx))) / 16u);
  int4 vec4_i32 = asint(ub[v_103]);
  uint v_104 = ((144u + (800u * uint(idx))) / 16u);
  uint4 vec4_u32 = ub[v_104];
  uint v_105 = ((160u + (800u * uint(idx))) / 16u);
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(ub[v_105]);
  float2x2 mat2x2_f32 = v_81((168u + (800u * uint(idx))));
  float2x3 mat2x3_f32 = v_79((192u + (800u * uint(idx))));
  float2x4 mat2x4_f32 = v_77((224u + (800u * uint(idx))));
  float3x2 mat3x2_f32 = v_71((256u + (800u * uint(idx))));
  float3x3 mat3x3_f32 = v_68((288u + (800u * uint(idx))));
  float3x4 mat3x4_f32 = v_65((336u + (800u * uint(idx))));
  float4x2 mat4x2_f32 = v_57((384u + (800u * uint(idx))));
  float4x3 mat4x3_f32 = v_53((416u + (800u * uint(idx))));
  float4x4 mat4x4_f32 = v_49((480u + (800u * uint(idx))));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_45((544u + (800u * uint(idx))));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_43((552u + (800u * uint(idx))));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_41((568u + (800u * uint(idx))));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_35((584u + (800u * uint(idx))));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_32((600u + (800u * uint(idx))));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_29((624u + (800u * uint(idx))));
  matrix<float16_t, 4, 2> mat4x2_f16 = v_2((648u + (800u * uint(idx))));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_25((664u + (800u * uint(idx))));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_21((696u + (800u * uint(idx))));
  float3 v_106[2] = v_14((736u + (800u * uint(idx))));
  matrix<float16_t, 4, 2> v_107[2] = v_10((768u + (800u * uint(idx))));
  int v_108 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_109 = (v_108 + int(scalar_u32));
  int v_110 = (v_109 + tint_f16_to_i32(scalar_f16));
  int v_111 = ((v_110 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_112 = (v_111 + int(vec2_u32[0u]));
  int v_113 = (v_112 + tint_f16_to_i32(vec2_f16[0u]));
  int v_114 = ((v_113 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_115 = (v_114 + int(vec3_u32[1u]));
  int v_116 = (v_115 + tint_f16_to_i32(vec3_f16[1u]));
  int v_117 = ((v_116 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_118 = (v_117 + int(vec4_u32[2u]));
  int v_119 = (v_118 + tint_f16_to_i32(vec4_f16[2u]));
  int v_120 = (v_119 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_121 = (v_120 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_122 = (v_121 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_123 = (v_122 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_124 = (v_123 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_125 = (v_124 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_126 = (v_125 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_127 = (v_126 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_128 = (v_127 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  int v_129 = (v_128 + tint_f16_to_i32(mat2x2_f16[int(0)][0u]));
  int v_130 = (v_129 + tint_f16_to_i32(mat2x3_f16[int(0)][0u]));
  int v_131 = (v_130 + tint_f16_to_i32(mat2x4_f16[int(0)][0u]));
  int v_132 = (v_131 + tint_f16_to_i32(mat3x2_f16[int(0)][0u]));
  int v_133 = (v_132 + tint_f16_to_i32(mat3x3_f16[int(0)][0u]));
  int v_134 = (v_133 + tint_f16_to_i32(mat3x4_f16[int(0)][0u]));
  int v_135 = (v_134 + tint_f16_to_i32(mat4x2_f16[int(0)][0u]));
  int v_136 = (v_135 + tint_f16_to_i32(mat4x3_f16[int(0)][0u]));
  int v_137 = (v_136 + tint_f16_to_i32(mat4x4_f16[int(0)][0u]));
  float3 arr2_vec3_f32[2] = v_106;
  int v_138 = (v_137 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_107;
  s.Store(0u, asuint((v_138 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)][0u]))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

FXC validation failure:
<scrubbed_path>(10,21-29): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
