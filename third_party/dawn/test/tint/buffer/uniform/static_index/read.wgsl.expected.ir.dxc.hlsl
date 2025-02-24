struct Inner {
  int scalar_i32;
  float scalar_f32;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[44];
};
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

Inner v(uint start_byte_offset) {
  Inner v_1 = {asint(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(ub[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)])};
  return v_1;
}

typedef Inner ary_ret[4];
ary_ret v_2(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      Inner v_5 = v((start_byte_offset + (v_4 * 32u)));
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

typedef float3 ary_ret_1[2];
ary_ret_1 v_7(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 2u)) {
        break;
      }
      a[v_9] = asfloat(ub[((start_byte_offset + (v_9 * 16u)) / 16u)].xyz);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  float3 v_10[2] = a;
  return v_10;
}

float4x4 v_11(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_12(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_13(uint start_byte_offset) {
  uint4 v_14 = ub[(start_byte_offset / 16u)];
  float2 v_15 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_14.zw) : (v_14.xy)));
  uint4 v_16 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_17 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_16.zw) : (v_16.xy)));
  uint4 v_18 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_19 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_18.zw) : (v_18.xy)));
  uint4 v_20 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_15, v_17, v_19, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_20.zw) : (v_20.xy))));
}

float3x4 v_21(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_22(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_23(uint start_byte_offset) {
  uint4 v_24 = ub[(start_byte_offset / 16u)];
  float2 v_25 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_24.zw) : (v_24.xy)));
  uint4 v_26 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_27 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_26.zw) : (v_26.xy)));
  uint4 v_28 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_25, v_27, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_28.zw) : (v_28.xy))));
}

float2x4 v_29(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_30(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_31(uint start_byte_offset) {
  uint4 v_32 = ub[(start_byte_offset / 16u)];
  float2 v_33 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint4 v_34 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_33, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy))));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0u].x);
  int scalar_i32 = asint(ub[0u].y);
  uint scalar_u32 = ub[0u].z;
  float2 vec2_f32 = asfloat(ub[1u].xy);
  int2 vec2_i32 = asint(ub[1u].zw);
  uint2 vec2_u32 = ub[2u].xy;
  float3 vec3_f32 = asfloat(ub[3u].xyz);
  int3 vec3_i32 = asint(ub[4u].xyz);
  uint3 vec3_u32 = ub[5u].xyz;
  float4 vec4_f32 = asfloat(ub[6u]);
  int4 vec4_i32 = asint(ub[7u]);
  uint4 vec4_u32 = ub[8u];
  float2x2 mat2x2_f32 = v_31(144u);
  float2x3 mat2x3_f32 = v_30(160u);
  float2x4 mat2x4_f32 = v_29(192u);
  float3x2 mat3x2_f32 = v_23(224u);
  float3x3 mat3x3_f32 = v_22(256u);
  float3x4 mat3x4_f32 = v_21(304u);
  float4x2 mat4x2_f32 = v_13(352u);
  float4x3 mat4x3_f32 = v_12(384u);
  float4x4 mat4x4_f32 = v_11(448u);
  float3 arr2_vec3_f32[2] = v_7(512u);
  Inner struct_inner = v(544u);
  Inner array_struct_inner[4] = v_2(576u);
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
  s.Store(0u, asuint((((v_51 + tint_f32_to_i32(arr2_vec3_f32[0u].x)) + struct_inner.scalar_i32) + array_struct_inner[0u].scalar_i32)));
}

