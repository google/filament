#version 310 es


struct S {
  float a;
};

void foo(bool param_bool, int param_i32, uint param_u32, float param_f32, ivec2 param_v2i32, uvec3 param_v3u32, vec4 param_v4f32, mat2x3 param_m2x3, float param_arr[4], S param_struct, inout float param_ptr_f32, inout vec4 param_ptr_vec, inout float param_ptr_arr[4]) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float b = 1.0f;
  vec4 c = vec4(0.0f);
  float d[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  foo(true, 1, 1u, 1.0f, ivec2(3), uvec3(4u), vec4(5.0f), mat2x3(vec3(0.0f), vec3(0.0f)), a, S(0.0f), b, c, d);
}
