struct main_inputs {
  uint idx : SV_GroupIndex;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

typedef float3 ary_ret[2];
ary_ret v(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      a[v_2] = asfloat(sb.Load3((offset + (v_2 * 16u))));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  float3 v_3[2] = a;
  return v_3;
}

float4x4 v_4(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_5(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_6(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_7(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_8(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_9(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_10(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_11(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_12(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

void main_inner(uint idx) {
  uint v_13 = 0u;
  sb.GetDimensions(v_13);
  float scalar_f32 = asfloat(sb.Load((0u + (min(idx, ((v_13 / 544u) - 1u)) * 544u))));
  uint v_14 = 0u;
  sb.GetDimensions(v_14);
  int scalar_i32 = asint(sb.Load((4u + (min(idx, ((v_14 / 544u) - 1u)) * 544u))));
  uint v_15 = 0u;
  sb.GetDimensions(v_15);
  uint scalar_u32 = sb.Load((8u + (min(idx, ((v_15 / 544u) - 1u)) * 544u)));
  uint v_16 = 0u;
  sb.GetDimensions(v_16);
  float2 vec2_f32 = asfloat(sb.Load2((16u + (min(idx, ((v_16 / 544u) - 1u)) * 544u))));
  uint v_17 = 0u;
  sb.GetDimensions(v_17);
  int2 vec2_i32 = asint(sb.Load2((24u + (min(idx, ((v_17 / 544u) - 1u)) * 544u))));
  uint v_18 = 0u;
  sb.GetDimensions(v_18);
  uint2 vec2_u32 = sb.Load2((32u + (min(idx, ((v_18 / 544u) - 1u)) * 544u)));
  uint v_19 = 0u;
  sb.GetDimensions(v_19);
  float3 vec3_f32 = asfloat(sb.Load3((48u + (min(idx, ((v_19 / 544u) - 1u)) * 544u))));
  uint v_20 = 0u;
  sb.GetDimensions(v_20);
  int3 vec3_i32 = asint(sb.Load3((64u + (min(idx, ((v_20 / 544u) - 1u)) * 544u))));
  uint v_21 = 0u;
  sb.GetDimensions(v_21);
  uint3 vec3_u32 = sb.Load3((80u + (min(idx, ((v_21 / 544u) - 1u)) * 544u)));
  uint v_22 = 0u;
  sb.GetDimensions(v_22);
  float4 vec4_f32 = asfloat(sb.Load4((96u + (min(idx, ((v_22 / 544u) - 1u)) * 544u))));
  uint v_23 = 0u;
  sb.GetDimensions(v_23);
  int4 vec4_i32 = asint(sb.Load4((112u + (min(idx, ((v_23 / 544u) - 1u)) * 544u))));
  uint v_24 = 0u;
  sb.GetDimensions(v_24);
  uint4 vec4_u32 = sb.Load4((128u + (min(idx, ((v_24 / 544u) - 1u)) * 544u)));
  uint v_25 = 0u;
  sb.GetDimensions(v_25);
  float2x2 mat2x2_f32 = v_12((144u + (min(idx, ((v_25 / 544u) - 1u)) * 544u)));
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  float2x3 mat2x3_f32 = v_11((160u + (min(idx, ((v_26 / 544u) - 1u)) * 544u)));
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  float2x4 mat2x4_f32 = v_10((192u + (min(idx, ((v_27 / 544u) - 1u)) * 544u)));
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  float3x2 mat3x2_f32 = v_9((224u + (min(idx, ((v_28 / 544u) - 1u)) * 544u)));
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  float3x3 mat3x3_f32 = v_8((256u + (min(idx, ((v_29 / 544u) - 1u)) * 544u)));
  uint v_30 = 0u;
  sb.GetDimensions(v_30);
  float3x4 mat3x4_f32 = v_7((304u + (min(idx, ((v_30 / 544u) - 1u)) * 544u)));
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  float4x2 mat4x2_f32 = v_6((352u + (min(idx, ((v_31 / 544u) - 1u)) * 544u)));
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  float4x3 mat4x3_f32 = v_5((384u + (min(idx, ((v_32 / 544u) - 1u)) * 544u)));
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  float4x4 mat4x4_f32 = v_4((448u + (min(idx, ((v_33 / 544u) - 1u)) * 544u)));
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  float3 arr2_vec3_f32[2] = v((512u + (min(idx, ((v_34 / 544u) - 1u)) * 544u)));
  int v_35 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_36 = (v_35 + int(scalar_u32));
  int v_37 = ((v_36 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_38 = (v_37 + int(vec2_u32.x));
  int v_39 = ((v_38 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_40 = (v_39 + int(vec3_u32.y));
  int v_41 = ((v_40 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_42 = (v_41 + int(vec4_u32.z));
  int v_43 = (v_42 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_44 = (v_43 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_45 = (v_44 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_46 = (v_45 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_47 = (v_46 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_48 = (v_47 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_49 = (v_48 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_50 = (v_49 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_51 = (v_50 + tint_f32_to_i32(mat4x4_f32[0u].x));
  s.Store(0u, asuint((v_51 + tint_f32_to_i32(arr2_vec3_f32[0u].x))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

