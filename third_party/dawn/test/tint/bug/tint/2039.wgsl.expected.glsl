#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v = 0u;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      bool tint_continue = false;
      switch(2) {
        case 1:
        {
          tint_continue = true;
          break;
        }
        default:
        {
          break;
        }
      }
      if (tint_continue) {
        {
          uint tint_low_inc = (tint_loop_idx.x + 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 0u));
          tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
          if (true) { break; }
        }
        continue;
      }
      v = (v + 1u);
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        if (true) { break; }
      }
      continue;
    }
  }
}
