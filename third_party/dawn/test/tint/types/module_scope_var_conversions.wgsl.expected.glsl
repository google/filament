#version 310 es

bool bool_var1 = true;
bool bool_var2 = true;
bool bool_var3 = true;
int i32_var1 = 1;
int i32_var2 = 1;
int i32_var3 = 1;
uint u32_var1 = 1u;
uint u32_var2 = 1u;
uint u32_var3 = 1u;
bvec3 v3bool_var1 = bvec3(true);
bvec3 v3bool_var2 = bvec3(true);
bvec3 v3bool_var3 = bvec3(true);
ivec3 v3i32_var1 = ivec3(1);
ivec3 v3i32_var2 = ivec3(1);
ivec3 v3i32_var3 = ivec3(1);
uvec3 v3u32_var1 = uvec3(1u);
uvec3 v3u32_var2 = uvec3(1u);
uvec3 v3u32_var3 = uvec3(1u);
bvec3 v3bool_var4 = bvec3(true);
bvec4 v4bool_var5 = bvec4(true, false, true, false);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool_var1 = false;
  bool_var2 = false;
  bool_var3 = false;
  i32_var1 = 0;
  i32_var2 = 0;
  i32_var3 = 0;
  u32_var1 = 0u;
  u32_var2 = 0u;
  u32_var3 = 0u;
  v3bool_var1 = bvec3(false);
  v3bool_var2 = bvec3(false);
  v3bool_var3 = bvec3(false);
  v3bool_var4 = bvec3(false);
  v4bool_var5 = bvec4(false);
  v3i32_var1 = ivec3(0);
  v3i32_var2 = ivec3(0);
  v3i32_var3 = ivec3(0);
  v3u32_var1 = uvec3(0u);
  v3u32_var2 = uvec3(0u);
  v3u32_var3 = uvec3(0u);
}
