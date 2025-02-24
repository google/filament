SKIP: INVALID

int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

float2x2 sb_load_16(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

float2x3 sb_load_17(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x4 sb_load_18(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float3x2 sb_load_19(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float3x3 sb_load_20(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x4 sb_load_21(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float4x2 sb_load_22(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float4x3 sb_load_23(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x4 sb_load_24(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

matrix<float16_t, 2, 2> sb_load_25(uint offset) {
  return matrix<float16_t, 2, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

matrix<float16_t, 2, 3> sb_load_26(uint offset) {
  return matrix<float16_t, 2, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 4> sb_load_27(uint offset) {
  return matrix<float16_t, 2, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 3, 2> sb_load_28(uint offset) {
  return matrix<float16_t, 3, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 3, 3> sb_load_29(uint offset) {
  return matrix<float16_t, 3, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 4> sb_load_30(uint offset) {
  return matrix<float16_t, 3, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 4, 2> sb_load_31(uint offset) {
  return matrix<float16_t, 4, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)), sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

matrix<float16_t, 4, 3> sb_load_32(uint offset) {
  return matrix<float16_t, 4, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)), sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 4, 4> sb_load_33(uint offset) {
  return matrix<float16_t, 4, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)), sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

typedef float3 sb_load_34_ret[2];
sb_load_34_ret sb_load_34(uint offset) {
  float3 arr_1[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = asfloat(sb.Load3((offset + (i * 16u))));
    }
  }
  return arr_1;
}

typedef matrix<float16_t, 4, 2> sb_load_35_ret[2];
sb_load_35_ret sb_load_35(uint offset) {
  matrix<float16_t, 4, 2> arr_2[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      arr_2[i_1] = sb_load_31((offset + (i_1 * 16u)));
    }
  }
  return arr_2;
}

void main_inner(uint idx) {
  float scalar_f32 = asfloat(sb.Load((800u * idx)));
  int scalar_i32 = asint(sb.Load(((800u * idx) + 4u)));
  uint scalar_u32 = sb.Load(((800u * idx) + 8u));
  float16_t scalar_f16 = sb.Load<float16_t>(((800u * idx) + 12u));
  float2 vec2_f32 = asfloat(sb.Load2(((800u * idx) + 16u)));
  int2 vec2_i32 = asint(sb.Load2(((800u * idx) + 24u)));
  uint2 vec2_u32 = sb.Load2(((800u * idx) + 32u));
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >(((800u * idx) + 40u));
  float3 vec3_f32 = asfloat(sb.Load3(((800u * idx) + 48u)));
  int3 vec3_i32 = asint(sb.Load3(((800u * idx) + 64u)));
  uint3 vec3_u32 = sb.Load3(((800u * idx) + 80u));
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >(((800u * idx) + 96u));
  float4 vec4_f32 = asfloat(sb.Load4(((800u * idx) + 112u)));
  int4 vec4_i32 = asint(sb.Load4(((800u * idx) + 128u)));
  uint4 vec4_u32 = sb.Load4(((800u * idx) + 144u));
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >(((800u * idx) + 160u));
  float2x2 mat2x2_f32 = sb_load_16(((800u * idx) + 168u));
  float2x3 mat2x3_f32 = sb_load_17(((800u * idx) + 192u));
  float2x4 mat2x4_f32 = sb_load_18(((800u * idx) + 224u));
  float3x2 mat3x2_f32 = sb_load_19(((800u * idx) + 256u));
  float3x3 mat3x3_f32 = sb_load_20(((800u * idx) + 288u));
  float3x4 mat3x4_f32 = sb_load_21(((800u * idx) + 336u));
  float4x2 mat4x2_f32 = sb_load_22(((800u * idx) + 384u));
  float4x3 mat4x3_f32 = sb_load_23(((800u * idx) + 416u));
  float4x4 mat4x4_f32 = sb_load_24(((800u * idx) + 480u));
  matrix<float16_t, 2, 2> mat2x2_f16 = sb_load_25(((800u * idx) + 544u));
  matrix<float16_t, 2, 3> mat2x3_f16 = sb_load_26(((800u * idx) + 552u));
  matrix<float16_t, 2, 4> mat2x4_f16 = sb_load_27(((800u * idx) + 568u));
  matrix<float16_t, 3, 2> mat3x2_f16 = sb_load_28(((800u * idx) + 584u));
  matrix<float16_t, 3, 3> mat3x3_f16 = sb_load_29(((800u * idx) + 600u));
  matrix<float16_t, 3, 4> mat3x4_f16 = sb_load_30(((800u * idx) + 624u));
  matrix<float16_t, 4, 2> mat4x2_f16 = sb_load_31(((800u * idx) + 648u));
  matrix<float16_t, 4, 3> mat4x3_f16 = sb_load_32(((800u * idx) + 664u));
  matrix<float16_t, 4, 4> mat4x4_f16 = sb_load_33(((800u * idx) + 696u));
  float3 arr2_vec3_f32[2] = sb_load_34(((800u * idx) + 736u));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = sb_load_35(((800u * idx) + 768u));
  s.Store(0u, asuint((((((((((((((((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + int(scalar_f16)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + int(vec2_f16.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + int(vec3_f16.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + int(vec4_f16.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + int(mat2x2_f16[0].x)) + int(mat2x3_f16[0].x)) + int(mat2x4_f16[0].x)) + int(mat3x2_f16[0].x)) + int(mat3x3_f16[0].x)) + int(mat3x4_f16[0].x)) + int(mat4x2_f16[0].x)) + int(mat4x3_f16[0].x)) + int(mat4x4_f16[0].x)) + int(arr2_mat4x2_f16[0][0].x)) + tint_ftoi(arr2_vec3_f32[0].x))));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
FXC validation failure:
<scrubbed_path>(48,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
