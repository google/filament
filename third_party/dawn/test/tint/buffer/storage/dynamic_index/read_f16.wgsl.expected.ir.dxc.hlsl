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
  return matrix<float16_t, 4, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)), sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_1(uint offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      a[v_3] = v((offset + (v_3 * 16u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_4[2] = a;
  return v_4;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_5(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 2u)) {
        break;
      }
      a[v_7] = asfloat(sb.Load3((offset + (v_7 * 16u))));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  float3 v_8[2] = a;
  return v_8;
}

matrix<float16_t, 4, 4> v_9(uint offset) {
  return matrix<float16_t, 4, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)), sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

matrix<float16_t, 4, 3> v_10(uint offset) {
  return matrix<float16_t, 4, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)), sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 3, 4> v_11(uint offset) {
  return matrix<float16_t, 3, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 3, 3> v_12(uint offset) {
  return matrix<float16_t, 3, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 2> v_13(uint offset) {
  return matrix<float16_t, 3, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 2, 4> v_14(uint offset) {
  return matrix<float16_t, 2, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 2, 3> v_15(uint offset) {
  return matrix<float16_t, 2, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 2> v_16(uint offset) {
  return matrix<float16_t, 2, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

float4x4 v_17(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_18(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_19(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_20(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_21(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_22(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_23(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_24(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_25(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

void main_inner(uint idx) {
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  float scalar_f32 = asfloat(sb.Load((0u + (min(idx, ((v_26 / 800u) - 1u)) * 800u))));
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  int scalar_i32 = asint(sb.Load((4u + (min(idx, ((v_27 / 800u) - 1u)) * 800u))));
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  uint scalar_u32 = sb.Load((8u + (min(idx, ((v_28 / 800u) - 1u)) * 800u)));
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  float16_t scalar_f16 = sb.Load<float16_t>((12u + (min(idx, ((v_29 / 800u) - 1u)) * 800u)));
  uint v_30 = 0u;
  sb.GetDimensions(v_30);
  float2 vec2_f32 = asfloat(sb.Load2((16u + (min(idx, ((v_30 / 800u) - 1u)) * 800u))));
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  int2 vec2_i32 = asint(sb.Load2((24u + (min(idx, ((v_31 / 800u) - 1u)) * 800u))));
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  uint2 vec2_u32 = sb.Load2((32u + (min(idx, ((v_32 / 800u) - 1u)) * 800u)));
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >((40u + (min(idx, ((v_33 / 800u) - 1u)) * 800u)));
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  float3 vec3_f32 = asfloat(sb.Load3((48u + (min(idx, ((v_34 / 800u) - 1u)) * 800u))));
  uint v_35 = 0u;
  sb.GetDimensions(v_35);
  int3 vec3_i32 = asint(sb.Load3((64u + (min(idx, ((v_35 / 800u) - 1u)) * 800u))));
  uint v_36 = 0u;
  sb.GetDimensions(v_36);
  uint3 vec3_u32 = sb.Load3((80u + (min(idx, ((v_36 / 800u) - 1u)) * 800u)));
  uint v_37 = 0u;
  sb.GetDimensions(v_37);
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >((96u + (min(idx, ((v_37 / 800u) - 1u)) * 800u)));
  uint v_38 = 0u;
  sb.GetDimensions(v_38);
  float4 vec4_f32 = asfloat(sb.Load4((112u + (min(idx, ((v_38 / 800u) - 1u)) * 800u))));
  uint v_39 = 0u;
  sb.GetDimensions(v_39);
  int4 vec4_i32 = asint(sb.Load4((128u + (min(idx, ((v_39 / 800u) - 1u)) * 800u))));
  uint v_40 = 0u;
  sb.GetDimensions(v_40);
  uint4 vec4_u32 = sb.Load4((144u + (min(idx, ((v_40 / 800u) - 1u)) * 800u)));
  uint v_41 = 0u;
  sb.GetDimensions(v_41);
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >((160u + (min(idx, ((v_41 / 800u) - 1u)) * 800u)));
  uint v_42 = 0u;
  sb.GetDimensions(v_42);
  float2x2 mat2x2_f32 = v_25((168u + (min(idx, ((v_42 / 800u) - 1u)) * 800u)));
  uint v_43 = 0u;
  sb.GetDimensions(v_43);
  float2x3 mat2x3_f32 = v_24((192u + (min(idx, ((v_43 / 800u) - 1u)) * 800u)));
  uint v_44 = 0u;
  sb.GetDimensions(v_44);
  float2x4 mat2x4_f32 = v_23((224u + (min(idx, ((v_44 / 800u) - 1u)) * 800u)));
  uint v_45 = 0u;
  sb.GetDimensions(v_45);
  float3x2 mat3x2_f32 = v_22((256u + (min(idx, ((v_45 / 800u) - 1u)) * 800u)));
  uint v_46 = 0u;
  sb.GetDimensions(v_46);
  float3x3 mat3x3_f32 = v_21((288u + (min(idx, ((v_46 / 800u) - 1u)) * 800u)));
  uint v_47 = 0u;
  sb.GetDimensions(v_47);
  float3x4 mat3x4_f32 = v_20((336u + (min(idx, ((v_47 / 800u) - 1u)) * 800u)));
  uint v_48 = 0u;
  sb.GetDimensions(v_48);
  float4x2 mat4x2_f32 = v_19((384u + (min(idx, ((v_48 / 800u) - 1u)) * 800u)));
  uint v_49 = 0u;
  sb.GetDimensions(v_49);
  float4x3 mat4x3_f32 = v_18((416u + (min(idx, ((v_49 / 800u) - 1u)) * 800u)));
  uint v_50 = 0u;
  sb.GetDimensions(v_50);
  float4x4 mat4x4_f32 = v_17((480u + (min(idx, ((v_50 / 800u) - 1u)) * 800u)));
  uint v_51 = 0u;
  sb.GetDimensions(v_51);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_16((544u + (min(idx, ((v_51 / 800u) - 1u)) * 800u)));
  uint v_52 = 0u;
  sb.GetDimensions(v_52);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_15((552u + (min(idx, ((v_52 / 800u) - 1u)) * 800u)));
  uint v_53 = 0u;
  sb.GetDimensions(v_53);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_14((568u + (min(idx, ((v_53 / 800u) - 1u)) * 800u)));
  uint v_54 = 0u;
  sb.GetDimensions(v_54);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_13((584u + (min(idx, ((v_54 / 800u) - 1u)) * 800u)));
  uint v_55 = 0u;
  sb.GetDimensions(v_55);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_12((600u + (min(idx, ((v_55 / 800u) - 1u)) * 800u)));
  uint v_56 = 0u;
  sb.GetDimensions(v_56);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_11((624u + (min(idx, ((v_56 / 800u) - 1u)) * 800u)));
  uint v_57 = 0u;
  sb.GetDimensions(v_57);
  matrix<float16_t, 4, 2> mat4x2_f16 = v((648u + (min(idx, ((v_57 / 800u) - 1u)) * 800u)));
  uint v_58 = 0u;
  sb.GetDimensions(v_58);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_10((664u + (min(idx, ((v_58 / 800u) - 1u)) * 800u)));
  uint v_59 = 0u;
  sb.GetDimensions(v_59);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_9((696u + (min(idx, ((v_59 / 800u) - 1u)) * 800u)));
  uint v_60 = 0u;
  sb.GetDimensions(v_60);
  float3 arr2_vec3_f32[2] = v_5((736u + (min(idx, ((v_60 / 800u) - 1u)) * 800u)));
  uint v_61 = 0u;
  sb.GetDimensions(v_61);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_1((768u + (min(idx, ((v_61 / 800u) - 1u)) * 800u)));
  int v_62 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_63 = (v_62 + int(scalar_u32));
  int v_64 = (v_63 + tint_f16_to_i32(scalar_f16));
  int v_65 = ((v_64 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_66 = (v_65 + int(vec2_u32.x));
  int v_67 = (v_66 + tint_f16_to_i32(vec2_f16.x));
  int v_68 = ((v_67 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_69 = (v_68 + int(vec3_u32.y));
  int v_70 = (v_69 + tint_f16_to_i32(vec3_f16.y));
  int v_71 = ((v_70 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_72 = (v_71 + int(vec4_u32.z));
  int v_73 = (v_72 + tint_f16_to_i32(vec4_f16.z));
  int v_74 = (v_73 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_75 = (v_74 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_76 = (v_75 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_77 = (v_76 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_78 = (v_77 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_79 = (v_78 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_80 = (v_79 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_81 = (v_80 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_82 = (v_81 + tint_f32_to_i32(mat4x4_f32[0u].x));
  int v_83 = (v_82 + tint_f16_to_i32(mat2x2_f16[0u].x));
  int v_84 = (v_83 + tint_f16_to_i32(mat2x3_f16[0u].x));
  int v_85 = (v_84 + tint_f16_to_i32(mat2x4_f16[0u].x));
  int v_86 = (v_85 + tint_f16_to_i32(mat3x2_f16[0u].x));
  int v_87 = (v_86 + tint_f16_to_i32(mat3x3_f16[0u].x));
  int v_88 = (v_87 + tint_f16_to_i32(mat3x4_f16[0u].x));
  int v_89 = (v_88 + tint_f16_to_i32(mat4x2_f16[0u].x));
  int v_90 = (v_89 + tint_f16_to_i32(mat4x3_f16[0u].x));
  int v_91 = (v_90 + tint_f16_to_i32(mat4x4_f16[0u].x));
  int v_92 = (v_91 + tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x));
  s.Store(0u, asuint((v_92 + tint_f32_to_i32(arr2_vec3_f32[0u].x))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

