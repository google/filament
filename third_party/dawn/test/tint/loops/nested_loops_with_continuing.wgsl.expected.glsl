#version 310 es

int f() {
  int i = 0;
  int j = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i > 4)) {
        return 1;
      }
      {
        uvec2 tint_loop_idx_1 = uvec2(0u);
        while(true) {
          if (all(equal(tint_loop_idx_1, uvec2(4294967295u)))) {
            break;
          }
          if ((j > 4)) {
            return 2;
          }
          {
            uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
            tint_loop_idx_1.x = tint_low_inc_1;
            uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
            tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
            j = (j + 1);
          }
          continue;
        }
      }
      /* unreachable */
      return 0;
    }
  }
  /* unreachable */
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
