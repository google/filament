#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

struct S {
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
  Inner struct_inner;
  Inner array_struct_inner[4];
  uint tint_pad_13;
};

layout(binding = 0, std430)
buffer sb_block_1_ssbo {
  S inner;
} v;
void tint_store_and_preserve_padding_8(uint target_indices[1], Inner value_param) {
  v.inner.array_struct_inner[target_indices[0u]].scalar_i32 = value_param.scalar_i32;
  v.inner.array_struct_inner[target_indices[0u]].scalar_f32 = value_param.scalar_f32;
  v.inner.array_struct_inner[target_indices[0u]].scalar_f16 = value_param.scalar_f16;
}
void tint_store_and_preserve_padding_9(Inner value_param[4]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_8(uint[1](v_2), value_param[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_7(Inner value_param) {
  v.inner.struct_inner.scalar_i32 = value_param.scalar_i32;
  v.inner.struct_inner.scalar_f32 = value_param.scalar_f32;
  v.inner.struct_inner.scalar_f16 = value_param.scalar_f16;
}
void tint_store_and_preserve_padding_6(vec3 value_param[2]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 2u)) {
        break;
      }
      v.inner.arr2_vec3_f32[v_4] = value_param[v_4];
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_5(f16mat4x3 value_param) {
  v.inner.mat4x3_f16[0u] = value_param[0u];
  v.inner.mat4x3_f16[1u] = value_param[1u];
  v.inner.mat4x3_f16[2u] = value_param[2u];
  v.inner.mat4x3_f16[3u] = value_param[3u];
}
void tint_store_and_preserve_padding_4(f16mat3 value_param) {
  v.inner.mat3x3_f16[0u] = value_param[0u];
  v.inner.mat3x3_f16[1u] = value_param[1u];
  v.inner.mat3x3_f16[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_3(f16mat2x3 value_param) {
  v.inner.mat2x3_f16[0u] = value_param[0u];
  v.inner.mat2x3_f16[1u] = value_param[1u];
}
void tint_store_and_preserve_padding_2(mat4x3 value_param) {
  v.inner.mat4x3_f32[0u] = value_param[0u];
  v.inner.mat4x3_f32[1u] = value_param[1u];
  v.inner.mat4x3_f32[2u] = value_param[2u];
  v.inner.mat4x3_f32[3u] = value_param[3u];
}
void tint_store_and_preserve_padding_1(mat3 value_param) {
  v.inner.mat3x3_f32[0u] = value_param[0u];
  v.inner.mat3x3_f32[1u] = value_param[1u];
  v.inner.mat3x3_f32[2u] = value_param[2u];
}
void tint_store_and_preserve_padding(mat2x3 value_param) {
  v.inner.mat2x3_f32[0u] = value_param[0u];
  v.inner.mat2x3_f32[1u] = value_param[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner.scalar_f32 = 0.0f;
  v.inner.scalar_i32 = 0;
  v.inner.scalar_u32 = 0u;
  v.inner.scalar_f16 = 0.0hf;
  v.inner.vec2_f32 = vec2(0.0f);
  v.inner.vec2_i32 = ivec2(0);
  v.inner.vec2_u32 = uvec2(0u);
  v.inner.vec2_f16 = f16vec2(0.0hf);
  v.inner.vec3_f32 = vec3(0.0f);
  v.inner.vec3_i32 = ivec3(0);
  v.inner.vec3_u32 = uvec3(0u);
  v.inner.vec3_f16 = f16vec3(0.0hf);
  v.inner.vec4_f32 = vec4(0.0f);
  v.inner.vec4_i32 = ivec4(0);
  v.inner.vec4_u32 = uvec4(0u);
  v.inner.vec4_f16 = f16vec4(0.0hf);
  v.inner.mat2x2_f32 = mat2(vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding(mat2x3(vec3(0.0f), vec3(0.0f)));
  v.inner.mat2x4_f32 = mat2x4(vec4(0.0f), vec4(0.0f));
  v.inner.mat3x2_f32 = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_1(mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  v.inner.mat3x4_f32 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  v.inner.mat4x2_f32 = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_2(mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  v.inner.mat4x4_f32 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  v.inner.mat2x2_f16 = f16mat2(f16vec2(0.0hf), f16vec2(0.0hf));
  tint_store_and_preserve_padding_3(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  v.inner.mat2x4_f16 = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
  v.inner.mat3x2_f16 = f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  tint_store_and_preserve_padding_4(f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)));
  v.inner.mat3x4_f16 = f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  v.inner.mat4x2_f16 = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  tint_store_and_preserve_padding_5(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)));
  v.inner.mat4x4_f16 = f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  tint_store_and_preserve_padding_6(vec3[2](vec3(0.0f), vec3(0.0f)));
  v.inner.arr2_mat4x2_f16 = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  tint_store_and_preserve_padding_7(Inner(0, 0.0f, 0.0hf));
  tint_store_and_preserve_padding_9(Inner[4](Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf)));
}
