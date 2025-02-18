#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  float16_t scalar_f16;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  f16vec2 vec2_f16;
  uint tint_pad_0;
  vec3 vec3_f32;
  uint tint_pad_1;
  ivec3 vec3_i32;
  uint tint_pad_2;
  uvec3 vec3_u32;
  uint tint_pad_3;
  f16vec3 vec3_f16;
  uint tint_pad_4;
  uint tint_pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  f16vec4 vec4_f16;
  mat2 mat2x2_f32;
  uint tint_pad_6;
  uint tint_pad_7;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint tint_pad_8;
  uint tint_pad_9;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  f16mat2 mat2x2_f16;
  f16mat2x3 mat2x3_f16;
  f16mat2x4 mat2x4_f16;
  f16mat3x2 mat3x2_f16;
  uint tint_pad_10;
  f16mat3 mat3x3_f16;
  f16mat3x4 mat3x4_f16;
  f16mat4x2 mat4x2_f16;
  f16mat4x3 mat4x3_f16;
  f16mat4 mat4x4_f16;
  uint tint_pad_11;
  uint tint_pad_12;
  vec3 arr2_vec3_f32[2];
  f16mat4x2 arr2_mat4x2_f16[2];
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
int tint_f16_to_i32(float16_t value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -65504.0hf)), (value <= 65504.0hf));
}
void main_inner(uint idx) {
  uint v_1 = min(idx, (uint(sb.arr.length()) - 1u));
  float scalar_f32 = sb.arr[v_1].scalar_f32;
  uint v_2 = min(idx, (uint(sb.arr.length()) - 1u));
  int scalar_i32 = sb.arr[v_2].scalar_i32;
  uint v_3 = min(idx, (uint(sb.arr.length()) - 1u));
  uint scalar_u32 = sb.arr[v_3].scalar_u32;
  uint v_4 = min(idx, (uint(sb.arr.length()) - 1u));
  float16_t scalar_f16 = sb.arr[v_4].scalar_f16;
  uint v_5 = min(idx, (uint(sb.arr.length()) - 1u));
  vec2 vec2_f32 = sb.arr[v_5].vec2_f32;
  uint v_6 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec2 vec2_i32 = sb.arr[v_6].vec2_i32;
  uint v_7 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec2 vec2_u32 = sb.arr[v_7].vec2_u32;
  uint v_8 = min(idx, (uint(sb.arr.length()) - 1u));
  f16vec2 vec2_f16 = sb.arr[v_8].vec2_f16;
  uint v_9 = min(idx, (uint(sb.arr.length()) - 1u));
  vec3 vec3_f32 = sb.arr[v_9].vec3_f32;
  uint v_10 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec3 vec3_i32 = sb.arr[v_10].vec3_i32;
  uint v_11 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec3 vec3_u32 = sb.arr[v_11].vec3_u32;
  uint v_12 = min(idx, (uint(sb.arr.length()) - 1u));
  f16vec3 vec3_f16 = sb.arr[v_12].vec3_f16;
  uint v_13 = min(idx, (uint(sb.arr.length()) - 1u));
  vec4 vec4_f32 = sb.arr[v_13].vec4_f32;
  uint v_14 = min(idx, (uint(sb.arr.length()) - 1u));
  ivec4 vec4_i32 = sb.arr[v_14].vec4_i32;
  uint v_15 = min(idx, (uint(sb.arr.length()) - 1u));
  uvec4 vec4_u32 = sb.arr[v_15].vec4_u32;
  uint v_16 = min(idx, (uint(sb.arr.length()) - 1u));
  f16vec4 vec4_f16 = sb.arr[v_16].vec4_f16;
  uint v_17 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2 mat2x2_f32 = sb.arr[v_17].mat2x2_f32;
  uint v_18 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2x3 mat2x3_f32 = sb.arr[v_18].mat2x3_f32;
  uint v_19 = min(idx, (uint(sb.arr.length()) - 1u));
  mat2x4 mat2x4_f32 = sb.arr[v_19].mat2x4_f32;
  uint v_20 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3x2 mat3x2_f32 = sb.arr[v_20].mat3x2_f32;
  uint v_21 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3 mat3x3_f32 = sb.arr[v_21].mat3x3_f32;
  uint v_22 = min(idx, (uint(sb.arr.length()) - 1u));
  mat3x4 mat3x4_f32 = sb.arr[v_22].mat3x4_f32;
  uint v_23 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4x2 mat4x2_f32 = sb.arr[v_23].mat4x2_f32;
  uint v_24 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4x3 mat4x3_f32 = sb.arr[v_24].mat4x3_f32;
  uint v_25 = min(idx, (uint(sb.arr.length()) - 1u));
  mat4 mat4x4_f32 = sb.arr[v_25].mat4x4_f32;
  uint v_26 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat2 mat2x2_f16 = sb.arr[v_26].mat2x2_f16;
  uint v_27 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat2x3 mat2x3_f16 = sb.arr[v_27].mat2x3_f16;
  uint v_28 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat2x4 mat2x4_f16 = sb.arr[v_28].mat2x4_f16;
  uint v_29 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat3x2 mat3x2_f16 = sb.arr[v_29].mat3x2_f16;
  uint v_30 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat3 mat3x3_f16 = sb.arr[v_30].mat3x3_f16;
  uint v_31 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat3x4 mat3x4_f16 = sb.arr[v_31].mat3x4_f16;
  uint v_32 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat4x2 mat4x2_f16 = sb.arr[v_32].mat4x2_f16;
  uint v_33 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat4x3 mat4x3_f16 = sb.arr[v_33].mat4x3_f16;
  uint v_34 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat4 mat4x4_f16 = sb.arr[v_34].mat4x4_f16;
  uint v_35 = min(idx, (uint(sb.arr.length()) - 1u));
  vec3 arr2_vec3_f32[2] = sb.arr[v_35].arr2_vec3_f32;
  uint v_36 = min(idx, (uint(sb.arr.length()) - 1u));
  f16mat4x2 arr2_mat4x2_f16[2] = sb.arr[v_36].arr2_mat4x2_f16;
  int v_37 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_38 = (v_37 + int(scalar_u32));
  int v_39 = (v_38 + tint_f16_to_i32(scalar_f16));
  int v_40 = ((v_39 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_41 = (v_40 + int(vec2_u32.x));
  int v_42 = (v_41 + tint_f16_to_i32(vec2_f16.x));
  int v_43 = ((v_42 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_44 = (v_43 + int(vec3_u32.y));
  int v_45 = (v_44 + tint_f16_to_i32(vec3_f16.y));
  int v_46 = ((v_45 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_47 = (v_46 + int(vec4_u32.z));
  int v_48 = (v_47 + tint_f16_to_i32(vec4_f16.z));
  int v_49 = (v_48 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_50 = (v_49 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_51 = (v_50 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_52 = (v_51 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_53 = (v_52 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_54 = (v_53 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_55 = (v_54 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_56 = (v_55 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_57 = (v_56 + tint_f32_to_i32(mat4x4_f32[0u].x));
  int v_58 = (v_57 + tint_f16_to_i32(mat2x2_f16[0u].x));
  int v_59 = (v_58 + tint_f16_to_i32(mat2x3_f16[0u].x));
  int v_60 = (v_59 + tint_f16_to_i32(mat2x4_f16[0u].x));
  int v_61 = (v_60 + tint_f16_to_i32(mat3x2_f16[0u].x));
  int v_62 = (v_61 + tint_f16_to_i32(mat3x3_f16[0u].x));
  int v_63 = (v_62 + tint_f16_to_i32(mat3x4_f16[0u].x));
  int v_64 = (v_63 + tint_f16_to_i32(mat4x2_f16[0u].x));
  int v_65 = (v_64 + tint_f16_to_i32(mat4x3_f16[0u].x));
  int v_66 = (v_65 + tint_f16_to_i32(mat4x4_f16[0u].x));
  int v_67 = (v_66 + tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x));
  v.inner = (v_67 + tint_f32_to_i32(arr2_vec3_f32[0u].x));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
