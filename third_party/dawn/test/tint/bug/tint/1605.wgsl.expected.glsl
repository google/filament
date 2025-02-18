#version 310 es

layout(binding = 0, std140)
uniform b_block_1_ubo {
  int inner;
} v;
bool func_3() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i < v.inner)) {
      } else {
        break;
      }
      {
        uvec2 tint_loop_idx_1 = uvec2(0u);
        int j = -1;
        while(true) {
          if (all(equal(tint_loop_idx_1, uvec2(4294967295u)))) {
            break;
          }
          if ((j == 1)) {
          } else {
            break;
          }
          return false;
        }
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + 1);
      }
      continue;
    }
  }
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func_3();
}
