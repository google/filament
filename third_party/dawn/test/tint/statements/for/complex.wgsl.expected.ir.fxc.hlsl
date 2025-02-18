
void some_loop_body() {
}

void f() {
  int j = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      bool v = false;
      if ((i < int(5))) {
        v = (j < int(10));
      } else {
        v = false;
      }
      if (v) {
      } else {
        break;
      }
      some_loop_body();
      j = (i * int(30));
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
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

