#version 310 es

layout(binding = 0, std430)
buffer tint_struct_1_ssbo {
  uint member_0[];
} v;
void v_1(uint v_2) {
  int v_3 = 0;
  int v_4 = 0;
  int v_5 = 0;
  uint v_6 = min(v_2, (uint(v.member_0.length()) - 1u));
  uint v_7 = v.member_0[v_6];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1(gl_LocalInvocationIndex);
}
