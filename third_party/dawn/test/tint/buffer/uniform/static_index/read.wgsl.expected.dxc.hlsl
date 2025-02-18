int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

struct Inner {
  int scalar_i32;
  float scalar_f32;
};

cbuffer cbuffer_ub : register(b0) {
  uint4 ub[44];
};
RWByteAddressBuffer s : register(u1);

float2x2 ub_load_12(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = ub[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = ub[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

float2x3 ub_load_13(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 16u)) / 4;
  return float2x3(asfloat(ub[scalar_offset_2 / 4].xyz), asfloat(ub[scalar_offset_3 / 4].xyz));
}

float2x4 ub_load_14(uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 16u)) / 4;
  return float2x4(asfloat(ub[scalar_offset_4 / 4]), asfloat(ub[scalar_offset_5 / 4]));
}

float3x2 ub_load_15(uint offset) {
  const uint scalar_offset_6 = ((offset + 0u)) / 4;
  uint4 ubo_load_2 = ub[scalar_offset_6 / 4];
  const uint scalar_offset_7 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = ub[scalar_offset_7 / 4];
  const uint scalar_offset_8 = ((offset + 16u)) / 4;
  uint4 ubo_load_4 = ub[scalar_offset_8 / 4];
  return float3x2(asfloat(((scalar_offset_6 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_7 & 2) ? ubo_load_3.zw : ubo_load_3.xy)), asfloat(((scalar_offset_8 & 2) ? ubo_load_4.zw : ubo_load_4.xy)));
}

float3x3 ub_load_16(uint offset) {
  const uint scalar_offset_9 = ((offset + 0u)) / 4;
  const uint scalar_offset_10 = ((offset + 16u)) / 4;
  const uint scalar_offset_11 = ((offset + 32u)) / 4;
  return float3x3(asfloat(ub[scalar_offset_9 / 4].xyz), asfloat(ub[scalar_offset_10 / 4].xyz), asfloat(ub[scalar_offset_11 / 4].xyz));
}

float3x4 ub_load_17(uint offset) {
  const uint scalar_offset_12 = ((offset + 0u)) / 4;
  const uint scalar_offset_13 = ((offset + 16u)) / 4;
  const uint scalar_offset_14 = ((offset + 32u)) / 4;
  return float3x4(asfloat(ub[scalar_offset_12 / 4]), asfloat(ub[scalar_offset_13 / 4]), asfloat(ub[scalar_offset_14 / 4]));
}

float4x2 ub_load_18(uint offset) {
  const uint scalar_offset_15 = ((offset + 0u)) / 4;
  uint4 ubo_load_5 = ub[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 8u)) / 4;
  uint4 ubo_load_6 = ub[scalar_offset_16 / 4];
  const uint scalar_offset_17 = ((offset + 16u)) / 4;
  uint4 ubo_load_7 = ub[scalar_offset_17 / 4];
  const uint scalar_offset_18 = ((offset + 24u)) / 4;
  uint4 ubo_load_8 = ub[scalar_offset_18 / 4];
  return float4x2(asfloat(((scalar_offset_15 & 2) ? ubo_load_5.zw : ubo_load_5.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_6.zw : ubo_load_6.xy)), asfloat(((scalar_offset_17 & 2) ? ubo_load_7.zw : ubo_load_7.xy)), asfloat(((scalar_offset_18 & 2) ? ubo_load_8.zw : ubo_load_8.xy)));
}

float4x3 ub_load_19(uint offset) {
  const uint scalar_offset_19 = ((offset + 0u)) / 4;
  const uint scalar_offset_20 = ((offset + 16u)) / 4;
  const uint scalar_offset_21 = ((offset + 32u)) / 4;
  const uint scalar_offset_22 = ((offset + 48u)) / 4;
  return float4x3(asfloat(ub[scalar_offset_19 / 4].xyz), asfloat(ub[scalar_offset_20 / 4].xyz), asfloat(ub[scalar_offset_21 / 4].xyz), asfloat(ub[scalar_offset_22 / 4].xyz));
}

float4x4 ub_load_20(uint offset) {
  const uint scalar_offset_23 = ((offset + 0u)) / 4;
  const uint scalar_offset_24 = ((offset + 16u)) / 4;
  const uint scalar_offset_25 = ((offset + 32u)) / 4;
  const uint scalar_offset_26 = ((offset + 48u)) / 4;
  return float4x4(asfloat(ub[scalar_offset_23 / 4]), asfloat(ub[scalar_offset_24 / 4]), asfloat(ub[scalar_offset_25 / 4]), asfloat(ub[scalar_offset_26 / 4]));
}

typedef float3 ub_load_21_ret[2];
ub_load_21_ret ub_load_21(uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      const uint scalar_offset_27 = ((offset + (i * 16u))) / 4;
      arr[i] = asfloat(ub[scalar_offset_27 / 4].xyz);
    }
  }
  return arr;
}

Inner ub_load_22(uint offset) {
  const uint scalar_offset_28 = ((offset + 0u)) / 4;
  const uint scalar_offset_29 = ((offset + 16u)) / 4;
  Inner tint_symbol = {asint(ub[scalar_offset_28 / 4][scalar_offset_28 % 4]), asfloat(ub[scalar_offset_29 / 4][scalar_offset_29 % 4])};
  return tint_symbol;
}

typedef Inner ub_load_23_ret[4];
ub_load_23_ret ub_load_23(uint offset) {
  Inner arr_1[4] = (Inner[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = ub_load_22((offset + (i_1 * 32u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0].x);
  int scalar_i32 = asint(ub[0].y);
  uint scalar_u32 = ub[0].z;
  float2 vec2_f32 = asfloat(ub[1].xy);
  int2 vec2_i32 = asint(ub[1].zw);
  uint2 vec2_u32 = ub[2].xy;
  float3 vec3_f32 = asfloat(ub[3].xyz);
  int3 vec3_i32 = asint(ub[4].xyz);
  uint3 vec3_u32 = ub[5].xyz;
  float4 vec4_f32 = asfloat(ub[6]);
  int4 vec4_i32 = asint(ub[7]);
  uint4 vec4_u32 = ub[8];
  float2x2 mat2x2_f32 = ub_load_12(144u);
  float2x3 mat2x3_f32 = ub_load_13(160u);
  float2x4 mat2x4_f32 = ub_load_14(192u);
  float3x2 mat3x2_f32 = ub_load_15(224u);
  float3x3 mat3x3_f32 = ub_load_16(256u);
  float3x4 mat3x4_f32 = ub_load_17(304u);
  float4x2 mat4x2_f32 = ub_load_18(352u);
  float4x3 mat4x3_f32 = ub_load_19(384u);
  float4x4 mat4x4_f32 = ub_load_20(448u);
  float3 arr2_vec3_f32[2] = ub_load_21(512u);
  Inner struct_inner = ub_load_22(544u);
  Inner array_struct_inner[4] = ub_load_23(576u);
  s.Store(0u, asuint((((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + tint_ftoi(arr2_vec3_f32[0].x)) + struct_inner.scalar_i32) + array_struct_inner[0].scalar_i32)));
  return;
}
