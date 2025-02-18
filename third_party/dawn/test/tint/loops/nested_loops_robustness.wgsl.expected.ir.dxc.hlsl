
int f() {
  int i = int(0);
  int j = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      i = (i + int(1));
      if ((i > int(4))) {
        return int(1);
      }
      {
        uint2 tint_loop_idx_1 = (0u).xx;
        while(true) {
          if (all((tint_loop_idx_1 == (4294967295u).xx))) {
            break;
          }
          j = (j + int(1));
          if ((j > int(4))) {
            return int(2);
          }
          {
            uint tint_low_inc_1 = (tint_loop_idx_1.x + 1u);
            tint_loop_idx_1.x = tint_low_inc_1;
            uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
            tint_loop_idx_1.y = (tint_loop_idx_1.y + tint_carry_1);
          }
          continue;
        }
      }
      /* unreachable */
      return int(0);
    }
  }
  /* unreachable */
  return int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

