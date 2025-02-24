#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int j = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((i < 2)) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(i) {
        case 0:
        {
          bool tint_continue_1 = false;
          switch(j) {
            case 0:
            {
              tint_continue_1 = true;
              break;
            }
            default:
            {
              break;
            }
          }
          if (tint_continue_1) {
            tint_continue = true;
            break;
          }
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
          i = (i + 2);
        }
        continue;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + 2);
      }
      continue;
    }
  }
}
