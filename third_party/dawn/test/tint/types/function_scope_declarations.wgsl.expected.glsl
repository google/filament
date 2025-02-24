#version 310 es


struct S {
  float a;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool bool_var = false;
  bool bool_let = false;
  int i32_var = 0;
  int i32_let = 0;
  uint u32_var = 0u;
  uint u32_let = 0u;
  float f32_var = 0.0f;
  float f32_let = 0.0f;
  ivec2 v2i32_var = ivec2(0);
  ivec2 v2i32_let = ivec2(0);
  uvec3 v3u32_var = uvec3(0u);
  uvec3 v3u32_let = uvec3(0u);
  vec4 v4f32_var = vec4(0.0f);
  vec4 v4f32_let = vec4(0.0f);
  mat2x3 m2x3_var = mat2x3(vec3(0.0f), vec3(0.0f));
  mat3x4 m3x4_let = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  float arr_var[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float arr_let[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  S struct_var = S(0.0f);
  S struct_let = S(0.0f);
}
