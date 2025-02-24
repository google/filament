#version 310 es

void constant_with_non_constant() {
  float a = 0.0f;
  vec2 b = vec2(1.0f, a);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool bool_var1 = true;
  bool bool_var2 = true;
  bool bool_var3 = true;
  int i32_var1 = 123;
  int i32_var2 = 123;
  int i32_var3 = 1;
  uint u32_var1 = 123u;
  uint u32_var2 = 123u;
  uint u32_var3 = 1u;
  bvec3 v3bool_var1 = bvec3(true);
  bvec3 v3bool_var11 = bvec3(true);
  bvec3 v3bool_var2 = bvec3(true);
  bvec3 v3bool_var3 = bvec3(true);
  ivec3 v3i32_var1 = ivec3(123);
  ivec3 v3i32_var2 = ivec3(123);
  ivec3 v3i32_var3 = ivec3(1);
  uvec3 v3u32_var1 = uvec3(123u);
  uvec3 v3u32_var2 = uvec3(123u);
  uvec3 v3u32_var3 = uvec3(1u);
  bvec3 v3bool_var4 = bvec3(true);
  bvec4 v4bool_var5 = bvec4(true, false, true, false);
  constant_with_non_constant();
}
