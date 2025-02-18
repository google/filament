SKIP: INVALID

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

Inner v(uint offset) {
  int v_1 = asint(sb.Load((offset + 0u)));
  float v_2 = asfloat(sb.Load((offset + 4u)));
  Inner v_3 = {v_1, v_2, sb.Load<float16_t>((offset + 8u))};
  return v_3;
}

typedef Inner ary_ret[4];
ary_ret v_4(uint offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      Inner v_7 = v((offset + (v_6 * 12u)));
      a[v_6] = v_7;
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  Inner v_8[4] = a;
  return v_8;
}

matrix<float16_t, 4, 2> v_9(uint offset) {
  vector<float16_t, 2> v_10 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  vector<float16_t, 2> v_11 = sb.Load<vector<float16_t, 2> >((offset + 4u));
  vector<float16_t, 2> v_12 = sb.Load<vector<float16_t, 2> >((offset + 8u));
  return matrix<float16_t, 4, 2>(v_10, v_11, v_12, sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_13(uint offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 2u)) {
        break;
      }
      a[v_15] = v_9((offset + (v_15 * 16u)));
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_16[2] = a;
  return v_16;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_17(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 2u)) {
        break;
      }
      a[v_19] = asfloat(sb.Load3((offset + (v_19 * 16u))));
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  float3 v_20[2] = a;
  return v_20;
}

matrix<float16_t, 4, 4> v_21(uint offset) {
  vector<float16_t, 4> v_22 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  vector<float16_t, 4> v_23 = sb.Load<vector<float16_t, 4> >((offset + 8u));
  vector<float16_t, 4> v_24 = sb.Load<vector<float16_t, 4> >((offset + 16u));
  return matrix<float16_t, 4, 4>(v_22, v_23, v_24, sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

matrix<float16_t, 4, 3> v_25(uint offset) {
  vector<float16_t, 3> v_26 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  vector<float16_t, 3> v_27 = sb.Load<vector<float16_t, 3> >((offset + 8u));
  vector<float16_t, 3> v_28 = sb.Load<vector<float16_t, 3> >((offset + 16u));
  return matrix<float16_t, 4, 3>(v_26, v_27, v_28, sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 3, 4> v_29(uint offset) {
  vector<float16_t, 4> v_30 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  vector<float16_t, 4> v_31 = sb.Load<vector<float16_t, 4> >((offset + 8u));
  return matrix<float16_t, 3, 4>(v_30, v_31, sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 3, 3> v_32(uint offset) {
  vector<float16_t, 3> v_33 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  vector<float16_t, 3> v_34 = sb.Load<vector<float16_t, 3> >((offset + 8u));
  return matrix<float16_t, 3, 3>(v_33, v_34, sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 2> v_35(uint offset) {
  vector<float16_t, 2> v_36 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  vector<float16_t, 2> v_37 = sb.Load<vector<float16_t, 2> >((offset + 4u));
  return matrix<float16_t, 3, 2>(v_36, v_37, sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 2, 4> v_38(uint offset) {
  vector<float16_t, 4> v_39 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  return matrix<float16_t, 2, 4>(v_39, sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 2, 3> v_40(uint offset) {
  vector<float16_t, 3> v_41 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  return matrix<float16_t, 2, 3>(v_41, sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 2> v_42(uint offset) {
  vector<float16_t, 2> v_43 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  return matrix<float16_t, 2, 2>(v_43, sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

float4x4 v_44(uint offset) {
  float4 v_45 = asfloat(sb.Load4((offset + 0u)));
  float4 v_46 = asfloat(sb.Load4((offset + 16u)));
  float4 v_47 = asfloat(sb.Load4((offset + 32u)));
  return float4x4(v_45, v_46, v_47, asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_48(uint offset) {
  float3 v_49 = asfloat(sb.Load3((offset + 0u)));
  float3 v_50 = asfloat(sb.Load3((offset + 16u)));
  float3 v_51 = asfloat(sb.Load3((offset + 32u)));
  return float4x3(v_49, v_50, v_51, asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_52(uint offset) {
  float2 v_53 = asfloat(sb.Load2((offset + 0u)));
  float2 v_54 = asfloat(sb.Load2((offset + 8u)));
  float2 v_55 = asfloat(sb.Load2((offset + 16u)));
  return float4x2(v_53, v_54, v_55, asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_56(uint offset) {
  float4 v_57 = asfloat(sb.Load4((offset + 0u)));
  float4 v_58 = asfloat(sb.Load4((offset + 16u)));
  return float3x4(v_57, v_58, asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_59(uint offset) {
  float3 v_60 = asfloat(sb.Load3((offset + 0u)));
  float3 v_61 = asfloat(sb.Load3((offset + 16u)));
  return float3x3(v_60, v_61, asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_62(uint offset) {
  float2 v_63 = asfloat(sb.Load2((offset + 0u)));
  float2 v_64 = asfloat(sb.Load2((offset + 8u)));
  return float3x2(v_63, v_64, asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_65(uint offset) {
  float4 v_66 = asfloat(sb.Load4((offset + 0u)));
  return float2x4(v_66, asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_67(uint offset) {
  float3 v_68 = asfloat(sb.Load3((offset + 0u)));
  return float2x3(v_68, asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_69(uint offset) {
  float2 v_70 = asfloat(sb.Load2((offset + 0u)));
  return float2x2(v_70, asfloat(sb.Load2((offset + 8u))));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(sb.Load(0u));
  int scalar_i32 = asint(sb.Load(4u));
  uint scalar_u32 = sb.Load(8u);
  float16_t scalar_f16 = sb.Load<float16_t>(12u);
  float2 vec2_f32 = asfloat(sb.Load2(16u));
  int2 vec2_i32 = asint(sb.Load2(24u));
  uint2 vec2_u32 = sb.Load2(32u);
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >(40u);
  float3 vec3_f32 = asfloat(sb.Load3(48u));
  int3 vec3_i32 = asint(sb.Load3(64u));
  uint3 vec3_u32 = sb.Load3(80u);
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >(96u);
  float4 vec4_f32 = asfloat(sb.Load4(112u));
  int4 vec4_i32 = asint(sb.Load4(128u));
  uint4 vec4_u32 = sb.Load4(144u);
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >(160u);
  float2x2 mat2x2_f32 = v_69(168u);
  float2x3 mat2x3_f32 = v_67(192u);
  float2x4 mat2x4_f32 = v_65(224u);
  float3x2 mat3x2_f32 = v_62(256u);
  float3x3 mat3x3_f32 = v_59(288u);
  float3x4 mat3x4_f32 = v_56(336u);
  float4x2 mat4x2_f32 = v_52(384u);
  float4x3 mat4x3_f32 = v_48(416u);
  float4x4 mat4x4_f32 = v_44(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_42(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_40(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_38(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_35(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_32(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_29(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_9(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_25(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_21(696u);
  float3 v_71[2] = v_17(736u);
  matrix<float16_t, 4, 2> v_72[2] = v_13(768u);
  Inner v_73 = v(800u);
  Inner v_74[4] = v_4(812u);
  int v_75 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_76 = (v_75 + int(scalar_u32));
  int v_77 = (v_76 + tint_f16_to_i32(scalar_f16));
  int v_78 = ((v_77 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_79 = (v_78 + int(vec2_u32[0u]));
  int v_80 = (v_79 + tint_f16_to_i32(vec2_f16[0u]));
  int v_81 = ((v_80 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_82 = (v_81 + int(vec3_u32[1u]));
  int v_83 = (v_82 + tint_f16_to_i32(vec3_f16[1u]));
  int v_84 = ((v_83 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_85 = (v_84 + int(vec4_u32[2u]));
  int v_86 = (v_85 + tint_f16_to_i32(vec4_f16[2u]));
  int v_87 = (v_86 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_88 = (v_87 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_89 = (v_88 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_90 = (v_89 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_91 = (v_90 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_92 = (v_91 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_93 = (v_92 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_94 = (v_93 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_95 = (v_94 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  int v_96 = (v_95 + tint_f16_to_i32(mat2x2_f16[int(0)][0u]));
  int v_97 = (v_96 + tint_f16_to_i32(mat2x3_f16[int(0)][0u]));
  int v_98 = (v_97 + tint_f16_to_i32(mat2x4_f16[int(0)][0u]));
  int v_99 = (v_98 + tint_f16_to_i32(mat3x2_f16[int(0)][0u]));
  int v_100 = (v_99 + tint_f16_to_i32(mat3x3_f16[int(0)][0u]));
  int v_101 = (v_100 + tint_f16_to_i32(mat3x4_f16[int(0)][0u]));
  int v_102 = (v_101 + tint_f16_to_i32(mat4x2_f16[int(0)][0u]));
  int v_103 = (v_102 + tint_f16_to_i32(mat4x3_f16[int(0)][0u]));
  int v_104 = (v_103 + tint_f16_to_i32(mat4x4_f16[int(0)][0u]));
  float3 arr2_vec3_f32[2] = v_71;
  int v_105 = (v_104 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_72;
  Inner struct_inner = v_73;
  Inner array_struct_inner[4] = v_74;
  s.Store(0u, asuint((((v_105 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)][0u])) + struct_inner.scalar_i32) + array_struct_inner[int(0)].scalar_i32)));
}

FXC validation failure:
<scrubbed_path>(4,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
