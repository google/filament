
int f() {
  int i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i > int(4))) {
        return i;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + int(1));
      }
      continue;
    }
  }
  /* unreachable */
  return int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

