#version 310 es

layout(binding = 1, std430)
buffer buf_block_1_ssbo {
  uint inner[1];
} v;
int g() {
  return 0;
}
int f() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      g();
      break;
    }
  }
  int o = g();
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((v.inner[0u] == 0u)) {
        break;
      }
      int s = f();
      v.inner[0u] = 0u;
      {
        uint tint_low_inc_1 = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc_1;
        uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry_1);
      }
      continue;
    }
  }
}
