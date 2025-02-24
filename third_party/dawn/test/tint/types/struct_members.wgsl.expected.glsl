#version 310 es


struct S_inner {
  float a;
};

struct S {
  bool member_bool;
  int member_i32;
  uint member_u32;
  float member_f32;
  ivec2 member_v2i32;
  uvec3 member_v3u32;
  vec4 member_v4f32;
  mat2x3 member_m2x3;
  float member_arr[4];
  S_inner member_struct;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S s = S(false, 0, 0u, 0.0f, ivec2(0), uvec3(0u), vec4(0.0f), mat2x3(vec3(0.0f), vec3(0.0f)), float[4](0.0f, 0.0f, 0.0f, 0.0f), S_inner(0.0f));
}
