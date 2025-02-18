#version 310 es


struct Inner {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  uint tint_pad_0;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 vec3_f32;
  uint tint_pad_3;
  ivec3 vec3_i32;
  uint tint_pad_4;
  uvec3 vec3_u32;
  uint tint_pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  mat2 mat2x2_f32;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint tint_pad_6;
  uint tint_pad_7;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  vec3 arr2_vec3_f32[2];
};

layout(binding = 0, std430)
buffer S_1_ssbo {
  Inner arr[];
} sb;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void main_inner(uint idx) {
  uint v_1 = min(idx, (uint(sb.arr.length()) - 1u));
  float scalar_f32 = sb.arr[v_1].scalar_f32;
  uint v_2 = min(idx, (uint(sb.arr.length()) - 1u));
  int scalar_i32 = sb.arr[v_2].scalar_i32;
  uint v_3 = min(idx, (uint(sb.arr.length()) - 1u));
  uint scalar_u32 = sb.arr[v_3].scalar_u32;
  uint v_4 = min(idx, (uint(sb.arr.length()) - 1u));
  vec2 vec2_f32 = sb.arr[v_4].vec2_f32;
  uint v_5 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec2 vec2_i32 = sb.arr[v_5].vec2_i32;
  uint v_6 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec2 vec2_u32 = sb.arr[v_6].vec2_u32;
  uint v_7 = min(idx, (uint(sb.arr.length()) - 1u));
  vec3 vec3_f32 = sb.arr[v_7].vec3_f32;
  uint v_8 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec3 vec3_i32 = sb.arr[v_8].vec3_i32;
  uint v_9 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec3 vec3_u32 = sb.arr[v_9].vec3_u32;
  uint v_10 = min(idx, (uint(sb.arr.length()) - 1u));
  vec4 vec4_f32 = sb.arr[v_10].vec4_f32;
  uint v_11 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec4 vec4_i32 = sb.arr[v_11].vec4_i32;
  uint v_12 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec4 vec4_u32 = sb.arr[v_12].vec4_u32;
  uint v_13 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2 mat2x2_f32 = sb.arr[v_13].mat2x2_f32;
  uint v_14 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2x3 mat2x3_f32 = sb.arr[v_14].mat2x3_f32;
  uint v_15 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2x4 mat2x4_f32 = sb.arr[v_15].mat2x4_f32;
  uint v_16 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3x2 mat3x2_f32 = sb.arr[v_16].mat3x2_f32;
  uint v_17 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3 mat3x3_f32 = sb.arr[v_17].mat3x3_f32;
  uint v_18 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3x4 mat3x4_f32 = sb.arr[v_18].mat3x4_f32;
  uint v_19 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4x2 mat4x2_f32 = sb.arr[v_19].mat4x2_f32;
  uint v_20 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4x3 mat4x3_f32 = sb.arr[v_20].mat4x3_f32;
  uint v_21 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4 mat4x4_f32 = sb.arr[v_21].mat4x4_f32;
  uint v_22 = min(idx, (uint(sb.arr.length()) - 1u));
  vec3 arr2_vec3_f32[2] = sb.arr[v_22].arr2_vec3_f32;
  int v_23 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_24 = (v_23 + int(scalar_u32));
  int v_25 = ((v_24 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_26 = (v_25 + int(vec2_u32.x));
  int v_27 = ((v_26 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_28 = (v_27 + int(vec3_u32.y));
  int v_29 = ((v_28 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_30 = (v_29 + int(vec4_u32.z));
  int v_31 = (v_30 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_32 = (v_31 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_33 = (v_32 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_34 = (v_33 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_35 = (v_34 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_36 = (v_35 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_37 = (v_36 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_38 = (v_37 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_39 = (v_38 + tint_f32_to_i32(mat4x4_f32[0u].x));
  v.inner = (v_39 + tint_f32_to_i32(arr2_vec3_f32[0u].x));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
