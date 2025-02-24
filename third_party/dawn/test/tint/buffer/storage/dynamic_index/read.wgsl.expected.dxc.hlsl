int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

float2x2 sb_load_12(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

float2x3 sb_load_13(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x4 sb_load_14(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float3x2 sb_load_15(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float3x3 sb_load_16(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x4 sb_load_17(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float4x2 sb_load_18(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float4x3 sb_load_19(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x4 sb_load_20(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

typedef float3 sb_load_21_ret[2];
sb_load_21_ret sb_load_21(uint offset) {
  float3 arr_1[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = asfloat(sb.Load3((offset + (i * 16u))));
    }
  }
  return arr_1;
}

void main_inner(uint idx) {
  uint tint_symbol_3 = 0u;
  sb.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 544u);
  float scalar_f32 = asfloat(sb.Load((544u * min(idx, (tint_symbol_4 - 1u)))));
  int scalar_i32 = asint(sb.Load(((544u * min(idx, (tint_symbol_4 - 1u))) + 4u)));
  uint scalar_u32 = sb.Load(((544u * min(idx, (tint_symbol_4 - 1u))) + 8u));
  float2 vec2_f32 = asfloat(sb.Load2(((544u * min(idx, (tint_symbol_4 - 1u))) + 16u)));
  int2 vec2_i32 = asint(sb.Load2(((544u * min(idx, (tint_symbol_4 - 1u))) + 24u)));
  uint2 vec2_u32 = sb.Load2(((544u * min(idx, (tint_symbol_4 - 1u))) + 32u));
  float3 vec3_f32 = asfloat(sb.Load3(((544u * min(idx, (tint_symbol_4 - 1u))) + 48u)));
  int3 vec3_i32 = asint(sb.Load3(((544u * min(idx, (tint_symbol_4 - 1u))) + 64u)));
  uint3 vec3_u32 = sb.Load3(((544u * min(idx, (tint_symbol_4 - 1u))) + 80u));
  float4 vec4_f32 = asfloat(sb.Load4(((544u * min(idx, (tint_symbol_4 - 1u))) + 96u)));
  int4 vec4_i32 = asint(sb.Load4(((544u * min(idx, (tint_symbol_4 - 1u))) + 112u)));
  uint4 vec4_u32 = sb.Load4(((544u * min(idx, (tint_symbol_4 - 1u))) + 128u));
  float2x2 mat2x2_f32 = sb_load_12(((544u * min(idx, (tint_symbol_4 - 1u))) + 144u));
  float2x3 mat2x3_f32 = sb_load_13(((544u * min(idx, (tint_symbol_4 - 1u))) + 160u));
  float2x4 mat2x4_f32 = sb_load_14(((544u * min(idx, (tint_symbol_4 - 1u))) + 192u));
  float3x2 mat3x2_f32 = sb_load_15(((544u * min(idx, (tint_symbol_4 - 1u))) + 224u));
  float3x3 mat3x3_f32 = sb_load_16(((544u * min(idx, (tint_symbol_4 - 1u))) + 256u));
  float3x4 mat3x4_f32 = sb_load_17(((544u * min(idx, (tint_symbol_4 - 1u))) + 304u));
  float4x2 mat4x2_f32 = sb_load_18(((544u * min(idx, (tint_symbol_4 - 1u))) + 352u));
  float4x3 mat4x3_f32 = sb_load_19(((544u * min(idx, (tint_symbol_4 - 1u))) + 384u));
  float4x4 mat4x4_f32 = sb_load_20(((544u * min(idx, (tint_symbol_4 - 1u))) + 448u));
  float3 arr2_vec3_f32[2] = sb_load_21(((544u * min(idx, (tint_symbol_4 - 1u))) + 512u));
  s.Store(0u, asuint((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + tint_ftoi(arr2_vec3_f32[0].x))));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
