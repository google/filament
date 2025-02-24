#version 310 es

void f_1() {
  uvec3 v = uvec3(0u);
  uvec3 n = uvec3(0u);
  uint offset_1 = 0u;
  uint count = 0u;
  uvec3 v_1 = v;
  uvec3 v_2 = n;
  uint v_3 = min(offset_1, 32u);
  uint v_4 = min(count, (32u - v_3));
  int v_5 = int(v_3);
  uvec3 x_15 = bitfieldInsert(v_1, v_2, v_5, int(v_4));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
