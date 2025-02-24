#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x2_f16_std140 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
  uint tint_pad_0;
};

struct S_std140 {
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
  vec2 mat2x2_f32_col0;
  vec2 mat2x2_f32_col1;
  uint tint_pad_6;
  uint tint_pad_7;
  vec3 mat2x3_f32_col0;
  uint tint_pad_8;
  vec3 mat2x3_f32_col1;
  uint tint_pad_9;
  mat2x4 mat2x4_f32;
  vec2 mat3x2_f32_col0;
  vec2 mat3x2_f32_col1;
  vec2 mat3x2_f32_col2;
  uint tint_pad_10;
  uint tint_pad_11;
  vec3 mat3x3_f32_col0;
  uint tint_pad_12;
  vec3 mat3x3_f32_col1;
  uint tint_pad_13;
  vec3 mat3x3_f32_col2;
  uint tint_pad_14;
  mat3x4 mat3x4_f32;
  vec2 mat4x2_f32_col0;
  vec2 mat4x2_f32_col1;
  vec2 mat4x2_f32_col2;
  vec2 mat4x2_f32_col3;
  vec3 mat4x3_f32_col0;
  uint tint_pad_15;
  vec3 mat4x3_f32_col1;
  uint tint_pad_16;
  vec3 mat4x3_f32_col2;
  uint tint_pad_17;
  vec3 mat4x3_f32_col3;
  uint tint_pad_18;
  mat4 mat4x4_f32;
  f16vec2 mat2x2_f16_col0;
  f16vec2 mat2x2_f16_col1;
  f16vec3 mat2x3_f16_col0;
  f16vec3 mat2x3_f16_col1;
  f16vec4 mat2x4_f16_col0;
  f16vec4 mat2x4_f16_col1;
  f16vec2 mat3x2_f16_col0;
  f16vec2 mat3x2_f16_col1;
  f16vec2 mat3x2_f16_col2;
  uint tint_pad_19;
  f16vec3 mat3x3_f16_col0;
  f16vec3 mat3x3_f16_col1;
  f16vec3 mat3x3_f16_col2;
  f16vec4 mat3x4_f16_col0;
  f16vec4 mat3x4_f16_col1;
  f16vec4 mat3x4_f16_col2;
  f16vec2 mat4x2_f16_col0;
  f16vec2 mat4x2_f16_col1;
  f16vec2 mat4x2_f16_col2;
  f16vec2 mat4x2_f16_col3;
  f16vec3 mat4x3_f16_col0;
  f16vec3 mat4x3_f16_col1;
  f16vec3 mat4x3_f16_col2;
  f16vec3 mat4x3_f16_col3;
  f16vec4 mat4x4_f16_col0;
  f16vec4 mat4x4_f16_col1;
  f16vec4 mat4x4_f16_col2;
  f16vec4 mat4x4_f16_col3;
  uint tint_pad_20;
  uint tint_pad_21;
  vec3 arr2_vec3_f32[2];
  mat4x2_f16_std140 arr2_mat4x2_f16[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
};

layout(binding = 0, std140)
uniform ub_block_std140_1_ubo {
  S_std140 inner;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f16_to_i32(float16_t value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -65504.0hf)), (value <= 65504.0hf));
}
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float scalar_f32 = v.inner.scalar_f32;
  int scalar_i32 = v.inner.scalar_i32;
  uint scalar_u32 = v.inner.scalar_u32;
  float16_t scalar_f16 = v.inner.scalar_f16;
  vec2 vec2_f32 = v.inner.vec2_f32;
  ivec2 vec2_i32 = v.inner.vec2_i32;
  uvec2 vec2_u32 = v.inner.vec2_u32;
  f16vec2 vec2_f16 = v.inner.vec2_f16;
  vec3 vec3_f32 = v.inner.vec3_f32;
  ivec3 vec3_i32 = v.inner.vec3_i32;
  uvec3 vec3_u32 = v.inner.vec3_u32;
  f16vec3 vec3_f16 = v.inner.vec3_f16;
  vec4 vec4_f32 = v.inner.vec4_f32;
  ivec4 vec4_i32 = v.inner.vec4_i32;
  uvec4 vec4_u32 = v.inner.vec4_u32;
  f16vec4 vec4_f16 = v.inner.vec4_f16;
  mat2 mat2x2_f32 = mat2(v.inner.mat2x2_f32_col0, v.inner.mat2x2_f32_col1);
  mat2x3 mat2x3_f32 = mat2x3(v.inner.mat2x3_f32_col0, v.inner.mat2x3_f32_col1);
  mat2x4 mat2x4_f32 = v.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = mat3x2(v.inner.mat3x2_f32_col0, v.inner.mat3x2_f32_col1, v.inner.mat3x2_f32_col2);
  mat3 mat3x3_f32 = mat3(v.inner.mat3x3_f32_col0, v.inner.mat3x3_f32_col1, v.inner.mat3x3_f32_col2);
  mat3x4 mat3x4_f32 = v.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = mat4x2(v.inner.mat4x2_f32_col0, v.inner.mat4x2_f32_col1, v.inner.mat4x2_f32_col2, v.inner.mat4x2_f32_col3);
  mat4x3 mat4x3_f32 = mat4x3(v.inner.mat4x3_f32_col0, v.inner.mat4x3_f32_col1, v.inner.mat4x3_f32_col2, v.inner.mat4x3_f32_col3);
  mat4 mat4x4_f32 = v.inner.mat4x4_f32;
  f16mat2 mat2x2_f16 = f16mat2(v.inner.mat2x2_f16_col0, v.inner.mat2x2_f16_col1);
  f16mat2x3 mat2x3_f16 = f16mat2x3(v.inner.mat2x3_f16_col0, v.inner.mat2x3_f16_col1);
  f16mat2x4 mat2x4_f16 = f16mat2x4(v.inner.mat2x4_f16_col0, v.inner.mat2x4_f16_col1);
  f16mat3x2 mat3x2_f16 = f16mat3x2(v.inner.mat3x2_f16_col0, v.inner.mat3x2_f16_col1, v.inner.mat3x2_f16_col2);
  f16mat3 mat3x3_f16 = f16mat3(v.inner.mat3x3_f16_col0, v.inner.mat3x3_f16_col1, v.inner.mat3x3_f16_col2);
  f16mat3x4 mat3x4_f16 = f16mat3x4(v.inner.mat3x4_f16_col0, v.inner.mat3x4_f16_col1, v.inner.mat3x4_f16_col2);
  f16mat4x2 mat4x2_f16 = f16mat4x2(v.inner.mat4x2_f16_col0, v.inner.mat4x2_f16_col1, v.inner.mat4x2_f16_col2, v.inner.mat4x2_f16_col3);
  f16mat4x3 mat4x3_f16 = f16mat4x3(v.inner.mat4x3_f16_col0, v.inner.mat4x3_f16_col1, v.inner.mat4x3_f16_col2, v.inner.mat4x3_f16_col3);
  f16mat4 mat4x4_f16 = f16mat4(v.inner.mat4x4_f16_col0, v.inner.mat4x4_f16_col1, v.inner.mat4x4_f16_col2, v.inner.mat4x4_f16_col3);
  vec3 arr2_vec3_f32[2] = v.inner.arr2_vec3_f32;
  mat4x2_f16_std140 v_2[2] = v.inner.arr2_mat4x2_f16;
  f16mat4x2 v_3[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 2u)) {
        break;
      }
      v_3[v_5] = f16mat4x2(v_2[v_5].col0, v_2[v_5].col1, v_2[v_5].col2, v_2[v_5].col3);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  f16mat4x2 arr2_mat4x2_f16[2] = v_3;
  Inner struct_inner = v.inner.struct_inner;
  Inner array_struct_inner[4] = v.inner.array_struct_inner;
  int v_6 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_7 = (v_6 + int(scalar_u32));
  int v_8 = (v_7 + tint_f16_to_i32(scalar_f16));
  int v_9 = ((v_8 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_10 = (v_9 + int(vec2_u32.x));
  int v_11 = (v_10 + tint_f16_to_i32(vec2_f16.x));
  int v_12 = ((v_11 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_13 = (v_12 + int(vec3_u32.y));
  int v_14 = (v_13 + tint_f16_to_i32(vec3_f16.y));
  int v_15 = ((v_14 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_16 = (v_15 + int(vec4_u32.z));
  int v_17 = (v_16 + tint_f16_to_i32(vec4_f16.z));
  int v_18 = (v_17 + tint_f32_to_i32(mat2x2_f32[0u].x));
  int v_19 = (v_18 + tint_f32_to_i32(mat2x3_f32[0u].x));
  int v_20 = (v_19 + tint_f32_to_i32(mat2x4_f32[0u].x));
  int v_21 = (v_20 + tint_f32_to_i32(mat3x2_f32[0u].x));
  int v_22 = (v_21 + tint_f32_to_i32(mat3x3_f32[0u].x));
  int v_23 = (v_22 + tint_f32_to_i32(mat3x4_f32[0u].x));
  int v_24 = (v_23 + tint_f32_to_i32(mat4x2_f32[0u].x));
  int v_25 = (v_24 + tint_f32_to_i32(mat4x3_f32[0u].x));
  int v_26 = (v_25 + tint_f32_to_i32(mat4x4_f32[0u].x));
  int v_27 = (v_26 + tint_f16_to_i32(mat2x2_f16[0u].x));
  int v_28 = (v_27 + tint_f16_to_i32(mat2x3_f16[0u].x));
  int v_29 = (v_28 + tint_f16_to_i32(mat2x4_f16[0u].x));
  int v_30 = (v_29 + tint_f16_to_i32(mat3x2_f16[0u].x));
  int v_31 = (v_30 + tint_f16_to_i32(mat3x3_f16[0u].x));
  int v_32 = (v_31 + tint_f16_to_i32(mat3x4_f16[0u].x));
  int v_33 = (v_32 + tint_f16_to_i32(mat4x2_f16[0u].x));
  int v_34 = (v_33 + tint_f16_to_i32(mat4x3_f16[0u].x));
  int v_35 = (v_34 + tint_f16_to_i32(mat4x4_f16[0u].x));
  int v_36 = (v_35 + tint_f32_to_i32(arr2_vec3_f32[0u].x));
  v_1.inner = (((v_36 + tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x)) + struct_inner.scalar_i32) + array_struct_inner[0u].scalar_i32);
}
