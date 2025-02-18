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
  vector<float16_t, 2> v_13 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_14 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_15 = tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_13, v_14, v_15, tint_bitcast_to_f16(ub[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_16(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 2u)) {
        break;
      }
      a[v_18] = v_12((start_byte_offset + (v_18 * 16u)));
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_19[2] = a;
  return v_19;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_20(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 2u)) {
        break;
      }
      a[v_22] = asfloat(ub[((start_byte_offset + (v_22 * 16u)) / 16u)].xyz);
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  float3 v_23[2] = a;
  return v_23;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_24 = float16_t(t_low.x);
  float16_t v_25 = float16_t(t_high.x);
  float16_t v_26 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_24, v_25, v_26, float16_t(t_high.y));
}

matrix<float16_t, 4, 4> v_27(uint start_byte_offset) {
  uint4 v_28 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_29 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_28.zw) : (v_28.xy)));
  uint4 v_30 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_31 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint4 v_32 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_33 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint4 v_34 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_29, v_31, v_33, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy))));
}

matrix<float16_t, 4, 3> v_35(uint start_byte_offset) {
  uint4 v_36 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_37 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy))).xyz;
  uint4 v_38 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_39 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.zw) : (v_38.xy))).xyz;
  uint4 v_40 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_41 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_40.zw) : (v_40.xy))).xyz;
  uint4 v_42 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_37, v_39, v_41, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_42.zw) : (v_42.xy))).xyz);
}

matrix<float16_t, 3, 4> v_43(uint start_byte_offset) {
  uint4 v_44 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_45 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_44.zw) : (v_44.xy)));
  uint4 v_46 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_47 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_46.zw) : (v_46.xy)));
  uint4 v_48 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_45, v_47, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_48.zw) : (v_48.xy))));
}

matrix<float16_t, 3, 3> v_49(uint start_byte_offset) {
  uint4 v_50 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_51 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_50.zw) : (v_50.xy))).xyz;
  uint4 v_52 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_53 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_52.zw) : (v_52.xy))).xyz;
  uint4 v_54 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 3>(v_51, v_53, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_54.zw) : (v_54.xy))).xyz);
}

matrix<float16_t, 3, 2> v_55(uint start_byte_offset) {
  vector<float16_t, 2> v_56 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_57 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 3, 2>(v_56, v_57, tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}

matrix<float16_t, 2, 4> v_58(uint start_byte_offset) {
  uint4 v_59 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_60 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_59.zw) : (v_59.xy)));
  uint4 v_61 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_60, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_61.zw) : (v_61.xy))));
}

matrix<float16_t, 2, 3> v_62(uint start_byte_offset) {
  uint4 v_63 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_64 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_63.zw) : (v_63.xy))).xyz;
  uint4 v_65 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_64, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_65.zw) : (v_65.xy))).xyz);
}

matrix<float16_t, 2, 2> v_66(uint start_byte_offset) {
  vector<float16_t, 2> v_67 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_67, tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

float4x4 v_68(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_69(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_70(uint start_byte_offset) {
  uint4 v_71 = ub[(start_byte_offset / 16u)];
  float2 v_72 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_71.zw) : (v_71.xy)));
  uint4 v_73 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_74 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_73.zw) : (v_73.xy)));
  uint4 v_75 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_76 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_75.zw) : (v_75.xy)));
  uint4 v_77 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_72, v_74, v_76, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_77.zw) : (v_77.xy))));
}

float3x4 v_78(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_79(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_80(uint start_byte_offset) {
  uint4 v_81 = ub[(start_byte_offset / 16u)];
  float2 v_82 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_81.zw) : (v_81.xy)));
  uint4 v_83 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_84 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_83.zw) : (v_83.xy)));
  uint4 v_85 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_82, v_84, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_85.zw) : (v_85.xy))));
}

float2x4 v_86(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_87(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_88(uint start_byte_offset) {
  uint4 v_89 = ub[(start_byte_offset / 16u)];
  float2 v_90 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_89.zw) : (v_89.xy)));
  uint4 v_91 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_90, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_91.zw) : (v_91.xy))));
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
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(ub[6u].xy).xyz;
  float4 vec4_f32 = asfloat(ub[7u]);
  int4 vec4_i32 = asint(ub[8u]);
  uint4 vec4_u32 = ub[9u];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(ub[10u].xy);
  float2x2 mat2x2_f32 = v_88(168u);
  float2x3 mat2x3_f32 = v_87(192u);
  float2x4 mat2x4_f32 = v_86(224u);
  float3x2 mat3x2_f32 = v_80(256u);
  float3x3 mat3x3_f32 = v_79(288u);
  float3x4 mat3x4_f32 = v_78(336u);
  float4x2 mat4x2_f32 = v_70(384u);
  float4x3 mat4x3_f32 = v_69(416u);
  float4x4 mat4x4_f32 = v_68(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_66(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_62(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_58(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_55(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_49(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_43(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_12(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_35(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_27(696u);
  float3 arr2_vec3_f32[2] = v_20(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_16(768u);
  Inner struct_inner = v_1(800u);
  Inner array_struct_inner[4] = v_6(816u);
  int v_92 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_93 = (v_92 + int(scalar_u32));
  int v_94 = (v_93 + tint_f16_to_i32(scalar_f16));
  int v_95 = ((v_94 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_96 = (v_95 + int(vec2_u32.x));
  int v_97 = (v_96 + tint_f16_to_i32(vec2_f16.x));
  int v_98 = ((v_97 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_99 = (v_98 + int(vec3_u32.y));
  int v_100 = (v_99 + tint_f16_to_i32(vec3_f16.y));
  int v_101 = ((v_100 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_102 = (v_101 + int(vec4_u32.z));
  int v_103 = (v_102 + tint_f16_to_i32(vec4_f16.z));
  int v_104 = (v_103 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_105 = (v_104 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_106 = (v_105 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_107 = (v_106 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_108 = (v_107 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_109 = (v_108 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_110 = (v_109 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_111 = (v_110 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_112 = (v_111 + tint_f32_to_i32(mat4x4_f32[0u].x));
  int v_113 = (v_112 + tint_f16_to_i32(mat2x2_f16[0u].x));
  int v_114 = (v_113 + tint_f16_to_i32(mat2x3_f16[0u].x));
  int v_115 = (v_114 + tint_f16_to_i32(mat2x4_f16[0u].x));
  int v_116 = (v_115 + tint_f16_to_i32(mat3x2_f16[0u].x));
  int v_117 = (v_116 + tint_f16_to_i32(mat3x3_f16[0u].x));
  int v_118 = (v_117 + tint_f16_to_i32(mat3x4_f16[0u].x));
  int v_119 = (v_118 + tint_f16_to_i32(mat4x2_f16[0u].x));
  int v_120 = (v_119 + tint_f16_to_i32(mat4x3_f16[0u].x));
  int v_121 = (v_120 + tint_f16_to_i32(mat4x4_f16[0u].x));
  int v_122 = (v_121 + tint_f32_to_i32(arr2_vec3_f32[0u].x));
  s.Store(0u, asuint((((v_122 + tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x)) + struct_inner.scalar_i32) + array_struct_inner[0u].scalar_i32)));
}

