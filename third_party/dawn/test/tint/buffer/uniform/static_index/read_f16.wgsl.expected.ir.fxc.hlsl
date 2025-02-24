SKIP: INVALID

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[55];
};
RWByteAddressBuffer s : register(u1);
int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

Inner v_1(uint start_byte_offset) {
  int v_2 = asint(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_3 = asfloat(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  uint v_4 = ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)];
  Inner v_5 = {v_2, v_3, float16_t(f16tof32((v_4 >> (((((8u + start_byte_offset) % 4u) == 0u)) ? (0u) : (16u)))))};
  return v_5;
}

typedef Inner ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      Inner v_9 = v_1((start_byte_offset + (v_8 * 16u)));
      a[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  Inner v_10[4] = a;
  return v_10;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_11 = float16_t(t_low);
  return vector<float16_t, 2>(v_11, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_12(uint start_byte_offset) {
  uint4 v_13 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_14 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_13.z) : (v_13.x)));
  uint4 v_15 = ub[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_16 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.z) : (v_15.x)));
  uint4 v_17 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_18 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_17.z) : (v_17.x)));
  uint4 v_19 = ub[((12u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 2>(v_14, v_16, v_18, tint_bitcast_to_f16(((((((12u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.z) : (v_19.x))));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_20(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 2u)) {
        break;
      }
      a[v_22] = v_12((start_byte_offset + (v_22 * 16u)));
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_23[2] = a;
  return v_23;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_24(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_25 = 0u;
    v_25 = 0u;
    while(true) {
      uint v_26 = v_25;
      if ((v_26 >= 2u)) {
        break;
      }
      a[v_26] = asfloat(ub[((start_byte_offset + (v_26 * 16u)) / 16u)].xyz);
      {
        v_25 = (v_26 + 1u);
      }
      continue;
    }
  }
  float3 v_27[2] = a;
  return v_27;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_28 = float16_t(t_low.x);
  float16_t v_29 = float16_t(t_high.x);
  float16_t v_30 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_28, v_29, v_30, float16_t(t_high.y));
}

matrix<float16_t, 4, 4> v_31(uint start_byte_offset) {
  vector<float16_t, 4> v_32 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_33 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]);
  vector<float16_t, 4> v_34 = tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 4, 4>(v_32, v_33, v_34, tint_bitcast_to_f16_1(ub[((24u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 4, 3> v_35(uint start_byte_offset) {
  vector<float16_t, 3> v_36 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_37 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz;
  vector<float16_t, 3> v_38 = tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 4, 3>(v_36, v_37, v_38, tint_bitcast_to_f16_1(ub[((24u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 3, 4> v_39(uint start_byte_offset) {
  vector<float16_t, 4> v_40 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_41 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 3, 4>(v_40, v_41, tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 3, 3> v_42(uint start_byte_offset) {
  vector<float16_t, 3> v_43 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_44 = tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 3, 3>(v_43, v_44, tint_bitcast_to_f16_1(ub[((16u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 3, 2> v_45(uint start_byte_offset) {
  uint4 v_46 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_47 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_46.z) : (v_46.x)));
  uint4 v_48 = ub[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_49 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_48.z) : (v_48.x)));
  uint4 v_50 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 2>(v_47, v_49, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_50.z) : (v_50.x))));
}

matrix<float16_t, 2, 4> v_51(uint start_byte_offset) {
  vector<float16_t, 4> v_52 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_52, tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]));
}

matrix<float16_t, 2, 3> v_53(uint start_byte_offset) {
  vector<float16_t, 3> v_54 = tint_bitcast_to_f16_1(ub[(start_byte_offset / 16u)]).xyz;
  return matrix<float16_t, 2, 3>(v_54, tint_bitcast_to_f16_1(ub[((8u + start_byte_offset) / 16u)]).xyz);
}

matrix<float16_t, 2, 2> v_55(uint start_byte_offset) {
  uint4 v_56 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_57 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_56.z) : (v_56.x)));
  uint4 v_58 = ub[((4u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 2>(v_57, tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_58.z) : (v_58.x))));
}

float4x4 v_59(uint start_byte_offset) {
  float4 v_60 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_61 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  float4 v_62 = asfloat(ub[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_60, v_61, v_62, asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_63(uint start_byte_offset) {
  float3 v_64 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_65 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_66 = asfloat(ub[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_64, v_65, v_66, asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_67(uint start_byte_offset) {
  uint4 v_68 = ub[(start_byte_offset / 16u)];
  float2 v_69 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_68.zw) : (v_68.xy)));
  uint4 v_70 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_71 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_70.zw) : (v_70.xy)));
  uint4 v_72 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_73 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_72.zw) : (v_72.xy)));
  uint4 v_74 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_69, v_71, v_73, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_74.zw) : (v_74.xy))));
}

float3x4 v_75(uint start_byte_offset) {
  float4 v_76 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_77 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_76, v_77, asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_78(uint start_byte_offset) {
  float3 v_79 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_80 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_79, v_80, asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_81(uint start_byte_offset) {
  uint4 v_82 = ub[(start_byte_offset / 16u)];
  float2 v_83 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_82.zw) : (v_82.xy)));
  uint4 v_84 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_85 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_84.zw) : (v_84.xy)));
  uint4 v_86 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_83, v_85, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_86.zw) : (v_86.xy))));
}

float2x4 v_87(uint start_byte_offset) {
  float4 v_88 = asfloat(ub[(start_byte_offset / 16u)]);
  return float2x4(v_88, asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_89(uint start_byte_offset) {
  float3 v_90 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  return float2x3(v_90, asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_91(uint start_byte_offset) {
  uint4 v_92 = ub[(start_byte_offset / 16u)];
  float2 v_93 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_92.zw) : (v_92.xy)));
  uint4 v_94 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_93, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_94.zw) : (v_94.xy))));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0u].x);
  int scalar_i32 = asint(ub[0u].y);
  uint scalar_u32 = ub[0u].z;
  float16_t scalar_f16 = float16_t(f16tof32(ub[0u].w));
  float2 vec2_f32 = asfloat(ub[1u].xy);
  int2 vec2_i32 = asint(ub[1u].zw);
  uint2 vec2_u32 = ub[2u].xy;
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[2u].z);
  float3 vec3_f32 = asfloat(ub[3u].xyz);
  int3 vec3_i32 = asint(ub[4u].xyz);
  uint3 vec3_u32 = ub[5u].xyz;
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(ub[6u]).xyz;
  float4 vec4_f32 = asfloat(ub[7u]);
  int4 vec4_i32 = asint(ub[8u]);
  uint4 vec4_u32 = ub[9u];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(ub[10u]);
  float2x2 mat2x2_f32 = v_91(168u);
  float2x3 mat2x3_f32 = v_89(192u);
  float2x4 mat2x4_f32 = v_87(224u);
  float3x2 mat3x2_f32 = v_81(256u);
  float3x3 mat3x3_f32 = v_78(288u);
  float3x4 mat3x4_f32 = v_75(336u);
  float4x2 mat4x2_f32 = v_67(384u);
  float4x3 mat4x3_f32 = v_63(416u);
  float4x4 mat4x4_f32 = v_59(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_55(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_53(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_51(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_45(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_42(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_39(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_12(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_35(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_31(696u);
  float3 v_95[2] = v_24(736u);
  matrix<float16_t, 4, 2> v_96[2] = v_20(768u);
  Inner v_97 = v_1(800u);
  Inner v_98[4] = v_6(816u);
  int v_99 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_100 = (v_99 + int(scalar_u32));
  int v_101 = (v_100 + tint_f16_to_i32(scalar_f16));
  int v_102 = ((v_101 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_103 = (v_102 + int(vec2_u32[0u]));
  int v_104 = (v_103 + tint_f16_to_i32(vec2_f16[0u]));
  int v_105 = ((v_104 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_106 = (v_105 + int(vec3_u32[1u]));
  int v_107 = (v_106 + tint_f16_to_i32(vec3_f16[1u]));
  int v_108 = ((v_107 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_109 = (v_108 + int(vec4_u32[2u]));
  int v_110 = (v_109 + tint_f16_to_i32(vec4_f16[2u]));
  int v_111 = (v_110 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_112 = (v_111 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_113 = (v_112 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_114 = (v_113 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_115 = (v_114 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_116 = (v_115 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_117 = (v_116 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_118 = (v_117 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_119 = (v_118 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  int v_120 = (v_119 + tint_f16_to_i32(mat2x2_f16[int(0)][0u]));
  int v_121 = (v_120 + tint_f16_to_i32(mat2x3_f16[int(0)][0u]));
  int v_122 = (v_121 + tint_f16_to_i32(mat2x4_f16[int(0)][0u]));
  int v_123 = (v_122 + tint_f16_to_i32(mat3x2_f16[int(0)][0u]));
  int v_124 = (v_123 + tint_f16_to_i32(mat3x3_f16[int(0)][0u]));
  int v_125 = (v_124 + tint_f16_to_i32(mat3x4_f16[int(0)][0u]));
  int v_126 = (v_125 + tint_f16_to_i32(mat4x2_f16[int(0)][0u]));
  int v_127 = (v_126 + tint_f16_to_i32(mat4x3_f16[int(0)][0u]));
  int v_128 = (v_127 + tint_f16_to_i32(mat4x4_f16[int(0)][0u]));
  float3 arr2_vec3_f32[2] = v_95;
  int v_129 = (v_128 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_96;
  Inner struct_inner = v_97;
  Inner array_struct_inner[4] = v_98;
  s.Store(0u, asuint((((v_129 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)][0u])) + struct_inner.scalar_i32) + array_struct_inner[int(0)].scalar_i32)));
}

FXC validation failure:
<scrubbed_path>(4,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
