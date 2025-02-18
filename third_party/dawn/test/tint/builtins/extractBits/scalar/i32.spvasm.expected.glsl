#version 310 es

void f_1() {
  int v = 0;
  uint offset_1 = 0u;
  uint count = 0u;
  int v_1 = v;
  uint v_2 = min(offset_1, 32u);
  uint v_3 = min(count, (32u - v_2));
  int v_4 = int(v_2);
  int x_14 = bitfieldExtract(v_1, v_4, int(v_3));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
