#version 310 es

layout(binding = 2, std430)
buffer _storage_block_1_ssbo {
  ivec2 inner;
} v;
void v_1() {
  ivec2 vec = ivec2(0);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((vec.y >= v.inner.y)) {
        break;
      }
      if ((vec.y >= 0)) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
