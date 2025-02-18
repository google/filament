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
  Inner v_1 = {asint(sb.Load((offset + 0u))), asfloat(sb.Load((offset + 4u))), sb.Load<float16_t>((offset + 8u))};
  return v_1;
}

typedef Inner ary_ret[4];
ary_ret v_2(uint offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      Inner v_5 = v((offset + (v_4 * 12u)));
      a[v_4] = v_5;
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  Inner v_6[4] = a;
  return v_6;
}

matrix<float16_t, 4, 2> v_7(uint offset) {
  return matrix<float16_t, 4, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)), sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_8(uint offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = v_7((offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_11[2] = a;
  return v_11;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_12(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 2u)) {
        break;
      }
      a[v_14] = asfloat(sb.Load3((offset + (v_14 * 16u))));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  float3 v_15[2] = a;
  return v_15;
}

matrix<float16_t, 4, 4> v_16(uint offset) {
  return matrix<float16_t, 4, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)), sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

matrix<float16_t, 4, 3> v_17(uint offset) {
  return matrix<float16_t, 4, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)), sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 3, 4> v_18(uint offset) {
  return matrix<float16_t, 3, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 3, 3> v_19(uint offset) {
  return matrix<float16_t, 3, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 2> v_20(uint offset) {
  return matrix<float16_t, 3, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 2, 4> v_21(uint offset) {
  return matrix<float16_t, 2, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 2, 3> v_22(uint offset) {
  return matrix<float16_t, 2, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 2> v_23(uint offset) {
  return matrix<float16_t, 2, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

float4x4 v_24(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_25(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_26(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_27(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_28(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_29(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_30(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_31(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_32(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
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
  float2x2 mat2x2_f32 = v_32(168u);
  float2x3 mat2x3_f32 = v_31(192u);
  float2x4 mat2x4_f32 = v_30(224u);
  float3x2 mat3x2_f32 = v_29(256u);
  float3x3 mat3x3_f32 = v_28(288u);
  float3x4 mat3x4_f32 = v_27(336u);
  float4x2 mat4x2_f32 = v_26(384u);
  float4x3 mat4x3_f32 = v_25(416u);
  float4x4 mat4x4_f32 = v_24(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_23(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_22(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_21(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_20(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_19(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_18(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_7(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_17(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_16(696u);
  float3 arr2_vec3_f32[2] = v_12(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_8(768u);
  Inner struct_inner = v(800u);
  Inner array_struct_inner[4] = v_2(812u);
  int v_33 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_34 = (v_33 + int(scalar_u32));
  int v_35 = (v_34 + tint_f16_to_i32(scalar_f16));
  int v_36 = ((v_35 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_37 = (v_36 + int(vec2_u32.x));
  int v_38 = (v_37 + tint_f16_to_i32(vec2_f16.x));
  int v_39 = ((v_38 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_40 = (v_39 + int(vec3_u32.y));
  int v_41 = (v_40 + tint_f16_to_i32(vec3_f16.y));
  int v_42 = ((v_41 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_43 = (v_42 + int(vec4_u32.z));
  int v_44 = (v_43 + tint_f16_to_i32(vec4_f16.z));
  int v_45 = (v_44 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_46 = (v_45 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_47 = (v_46 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_48 = (v_47 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_49 = (v_48 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_50 = (v_49 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_51 = (v_50 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_52 = (v_51 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_53 = (v_52 + tint_f32_to_i32(mat4x4_f32[0u].x));
  int v_54 = (v_53 + tint_f16_to_i32(mat2x2_f16[0u].x));
  int v_55 = (v_54 + tint_f16_to_i32(mat2x3_f16[0u].x));
  int v_56 = (v_55 + tint_f16_to_i32(mat2x4_f16[0u].x));
  int v_57 = (v_56 + tint_f16_to_i32(mat3x2_f16[0u].x));
  int v_58 = (v_57 + tint_f16_to_i32(mat3x3_f16[0u].x));
  int v_59 = (v_58 + tint_f16_to_i32(mat3x4_f16[0u].x));
  int v_60 = (v_59 + tint_f16_to_i32(mat4x2_f16[0u].x));
  int v_61 = (v_60 + tint_f16_to_i32(mat4x3_f16[0u].x));
  int v_62 = (v_61 + tint_f16_to_i32(mat4x4_f16[0u].x));
  int v_63 = (v_62 + tint_f32_to_i32(arr2_vec3_f32[0u].x));
  s.Store(0u, asuint((((v_63 + tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x)) + struct_inner.scalar_i32) + array_struct_inner[0u].scalar_i32)));
}

