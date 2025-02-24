#version 310 es


struct S {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec4 b;
  mat2 c;
};

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  S inner;
} v_1;
uint i = 0u;
int idx1() {
  i = (i + 1u);
  return 1;
}
int idx2() {
  i = (i + 2u);
  return 1;
}
int idx3() {
  i = (i + 3u);
  return 1;
}
void foo() {
  float a[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    uint v_2 = min(uint(idx1()), 3u);
    a[v_2] = (a[v_2] * 2.0f);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      uint v_3 = min(uint(idx2()), 3u);
      if ((a[v_3] < 10.0f)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        uint v_4 = min(uint(idx3()), 3u);
        a[v_4] = (a[v_4] + 1.0f);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
