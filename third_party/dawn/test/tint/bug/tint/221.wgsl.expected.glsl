#version 310 es


struct Buf {
  uint count;
  uint data[50];
};

layout(binding = 0, std430)
buffer b_block_1_ssbo {
  Buf inner;
} v;
uint tint_mod_u32(uint lhs, uint rhs) {
  return (lhs - ((lhs / mix(rhs, 1u, (rhs == 0u))) * mix(rhs, 1u, (rhs == 0u))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint i = 0u;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i >= v.inner.count)) {
        break;
      }
      uint v_1 = min(i, 49u);
      if ((tint_mod_u32(i, 2u) == 0u)) {
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
          v.inner.data[v_1] = (v.inner.data[v_1] * 2u);
          i = (i + 1u);
        }
        continue;
      }
      v.inner.data[v_1] = 0u;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        v.inner.data[v_1] = (v.inner.data[v_1] * 2u);
        i = (i + 1u);
      }
      continue;
    }
  }
}
