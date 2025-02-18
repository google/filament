SKIP: INVALID

struct main_inputs {
  uint idx : SV_GroupIndex;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

matrix<float16_t, 4, 2> v(uint offset) {
  vector<float16_t, 2> v_1 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  vector<float16_t, 2> v_2 = sb.Load<vector<float16_t, 2> >((offset + 4u));
  vector<float16_t, 2> v_3 = sb.Load<vector<float16_t, 2> >((offset + 8u));
  return matrix<float16_t, 4, 2>(v_1, v_2, v_3, sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_4(uint offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 2u)) {
        break;
      }
      a[v_6] = v((offset + (v_6 * 16u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_7[2] = a;
  return v_7;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_8(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = asfloat(sb.Load3((offset + (v_10 * 16u))));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  float3 v_11[2] = a;
  return v_11;
}

matrix<float16_t, 4, 4> v_12(uint offset) {
  vector<float16_t, 4> v_13 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  vector<float16_t, 4> v_14 = sb.Load<vector<float16_t, 4> >((offset + 8u));
  vector<float16_t, 4> v_15 = sb.Load<vector<float16_t, 4> >((offset + 16u));
  return matrix<float16_t, 4, 4>(v_13, v_14, v_15, sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

matrix<float16_t, 4, 3> v_16(uint offset) {
  vector<float16_t, 3> v_17 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  vector<float16_t, 3> v_18 = sb.Load<vector<float16_t, 3> >((offset + 8u));
  vector<float16_t, 3> v_19 = sb.Load<vector<float16_t, 3> >((offset + 16u));
  return matrix<float16_t, 4, 3>(v_17, v_18, v_19, sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 3, 4> v_20(uint offset) {
  vector<float16_t, 4> v_21 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  vector<float16_t, 4> v_22 = sb.Load<vector<float16_t, 4> >((offset + 8u));
  return matrix<float16_t, 3, 4>(v_21, v_22, sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 3, 3> v_23(uint offset) {
  vector<float16_t, 3> v_24 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  vector<float16_t, 3> v_25 = sb.Load<vector<float16_t, 3> >((offset + 8u));
  return matrix<float16_t, 3, 3>(v_24, v_25, sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 2> v_26(uint offset) {
  vector<float16_t, 2> v_27 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  vector<float16_t, 2> v_28 = sb.Load<vector<float16_t, 2> >((offset + 4u));
  return matrix<float16_t, 3, 2>(v_27, v_28, sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 2, 4> v_29(uint offset) {
  vector<float16_t, 4> v_30 = sb.Load<vector<float16_t, 4> >((offset + 0u));
  return matrix<float16_t, 2, 4>(v_30, sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 2, 3> v_31(uint offset) {
  vector<float16_t, 3> v_32 = sb.Load<vector<float16_t, 3> >((offset + 0u));
  return matrix<float16_t, 2, 3>(v_32, sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 2> v_33(uint offset) {
  vector<float16_t, 2> v_34 = sb.Load<vector<float16_t, 2> >((offset + 0u));
  return matrix<float16_t, 2, 2>(v_34, sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

float4x4 v_35(uint offset) {
  float4 v_36 = asfloat(sb.Load4((offset + 0u)));
  float4 v_37 = asfloat(sb.Load4((offset + 16u)));
  float4 v_38 = asfloat(sb.Load4((offset + 32u)));
  return float4x4(v_36, v_37, v_38, asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_39(uint offset) {
  float3 v_40 = asfloat(sb.Load3((offset + 0u)));
  float3 v_41 = asfloat(sb.Load3((offset + 16u)));
  float3 v_42 = asfloat(sb.Load3((offset + 32u)));
  return float4x3(v_40, v_41, v_42, asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_43(uint offset) {
  float2 v_44 = asfloat(sb.Load2((offset + 0u)));
  float2 v_45 = asfloat(sb.Load2((offset + 8u)));
  float2 v_46 = asfloat(sb.Load2((offset + 16u)));
  return float4x2(v_44, v_45, v_46, asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_47(uint offset) {
  float4 v_48 = asfloat(sb.Load4((offset + 0u)));
  float4 v_49 = asfloat(sb.Load4((offset + 16u)));
  return float3x4(v_48, v_49, asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_50(uint offset) {
  float3 v_51 = asfloat(sb.Load3((offset + 0u)));
  float3 v_52 = asfloat(sb.Load3((offset + 16u)));
  return float3x3(v_51, v_52, asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_53(uint offset) {
  float2 v_54 = asfloat(sb.Load2((offset + 0u)));
  float2 v_55 = asfloat(sb.Load2((offset + 8u)));
  return float3x2(v_54, v_55, asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_56(uint offset) {
  float4 v_57 = asfloat(sb.Load4((offset + 0u)));
  return float2x4(v_57, asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_58(uint offset) {
  float3 v_59 = asfloat(sb.Load3((offset + 0u)));
  return float2x3(v_59, asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_60(uint offset) {
  float2 v_61 = asfloat(sb.Load2((offset + 0u)));
  return float2x2(v_61, asfloat(sb.Load2((offset + 8u))));
}

void main_inner(uint idx) {
  float scalar_f32 = asfloat(sb.Load((0u + (uint(idx) * 800u))));
  int scalar_i32 = asint(sb.Load((4u + (uint(idx) * 800u))));
  uint scalar_u32 = sb.Load((8u + (uint(idx) * 800u)));
  float16_t scalar_f16 = sb.Load<float16_t>((12u + (uint(idx) * 800u)));
  float2 vec2_f32 = asfloat(sb.Load2((16u + (uint(idx) * 800u))));
  int2 vec2_i32 = asint(sb.Load2((24u + (uint(idx) * 800u))));
  uint2 vec2_u32 = sb.Load2((32u + (uint(idx) * 800u)));
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >((40u + (uint(idx) * 800u)));
  float3 vec3_f32 = asfloat(sb.Load3((48u + (uint(idx) * 800u))));
  int3 vec3_i32 = asint(sb.Load3((64u + (uint(idx) * 800u))));
  uint3 vec3_u32 = sb.Load3((80u + (uint(idx) * 800u)));
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >((96u + (uint(idx) * 800u)));
  float4 vec4_f32 = asfloat(sb.Load4((112u + (uint(idx) * 800u))));
  int4 vec4_i32 = asint(sb.Load4((128u + (uint(idx) * 800u))));
  uint4 vec4_u32 = sb.Load4((144u + (uint(idx) * 800u)));
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >((160u + (uint(idx) * 800u)));
  float2x2 mat2x2_f32 = v_60((168u + (uint(idx) * 800u)));
  float2x3 mat2x3_f32 = v_58((192u + (uint(idx) * 800u)));
  float2x4 mat2x4_f32 = v_56((224u + (uint(idx) * 800u)));
  float3x2 mat3x2_f32 = v_53((256u + (uint(idx) * 800u)));
  float3x3 mat3x3_f32 = v_50((288u + (uint(idx) * 800u)));
  float3x4 mat3x4_f32 = v_47((336u + (uint(idx) * 800u)));
  float4x2 mat4x2_f32 = v_43((384u + (uint(idx) * 800u)));
  float4x3 mat4x3_f32 = v_39((416u + (uint(idx) * 800u)));
  float4x4 mat4x4_f32 = v_35((480u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_33((544u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_31((552u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_29((568u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_26((584u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_23((600u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_20((624u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 2> mat4x2_f16 = v((648u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_16((664u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_12((696u + (uint(idx) * 800u)));
  float3 v_62[2] = v_8((736u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 2> v_63[2] = v_4((768u + (uint(idx) * 800u)));
  int v_64 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_65 = (v_64 + int(scalar_u32));
  int v_66 = (v_65 + tint_f16_to_i32(scalar_f16));
  int v_67 = ((v_66 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_68 = (v_67 + int(vec2_u32[0u]));
  int v_69 = (v_68 + tint_f16_to_i32(vec2_f16[0u]));
  int v_70 = ((v_69 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_71 = (v_70 + int(vec3_u32[1u]));
  int v_72 = (v_71 + tint_f16_to_i32(vec3_f16[1u]));
  int v_73 = ((v_72 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_74 = (v_73 + int(vec4_u32[2u]));
  int v_75 = (v_74 + tint_f16_to_i32(vec4_f16[2u]));
  int v_76 = (v_75 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_77 = (v_76 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_78 = (v_77 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_79 = (v_78 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_80 = (v_79 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_81 = (v_80 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_82 = (v_81 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_83 = (v_82 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_84 = (v_83 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  int v_85 = (v_84 + tint_f16_to_i32(mat2x2_f16[int(0)][0u]));
  int v_86 = (v_85 + tint_f16_to_i32(mat2x3_f16[int(0)][0u]));
  int v_87 = (v_86 + tint_f16_to_i32(mat2x4_f16[int(0)][0u]));
  int v_88 = (v_87 + tint_f16_to_i32(mat3x2_f16[int(0)][0u]));
  int v_89 = (v_88 + tint_f16_to_i32(mat3x3_f16[int(0)][0u]));
  int v_90 = (v_89 + tint_f16_to_i32(mat3x4_f16[int(0)][0u]));
  int v_91 = (v_90 + tint_f16_to_i32(mat4x2_f16[int(0)][0u]));
  int v_92 = (v_91 + tint_f16_to_i32(mat4x3_f16[int(0)][0u]));
  int v_93 = (v_92 + tint_f16_to_i32(mat4x4_f16[int(0)][0u]));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_63;
  int v_94 = (v_93 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)][0u]));
  float3 arr2_vec3_f32[2] = v_62;
  s.Store(0u, asuint((v_94 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

FXC validation failure:
<scrubbed_path>(12,21-29): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
