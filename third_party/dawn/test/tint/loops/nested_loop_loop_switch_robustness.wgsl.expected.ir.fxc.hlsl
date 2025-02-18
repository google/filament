
[numthreads(1, 1, 1)]
void main() {
  {
    uint2 tint_loop_idx = (0u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < int(2))) {
      } else {
        break;
      }
      {
        uint2 tint_loop_idx_1 = (0u).xx;
        int j = int(0);
        while(true) {
          if (all((tint_loop_idx_1 == (4294967295u).xx))) {
            break;
          }
          if ((j < int(2))) {
          } else {
            break;
          }
          bool tint_continue = false;
          switch(i) {
            case int(0):
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
              uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
              tint_loop_idx_1.x = tint_low_inc_1;
              uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
              tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
              j = (j + int(2));
            }
            continue;
          }
          {
            uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
            tint_loop_idx_1.x = tint_low_inc_1;
            uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
            tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
            j = (j + int(2));
          }
          continue;
        }
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + int(2));
      }
      continue;
    }
  }
}

