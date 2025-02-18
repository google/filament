
groupshared int a;
groupshared int b;
void foo() {
  {
    uint2 tint_loop_idx = (0u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      int v = i;
      GroupMemoryBarrierWithGroupSync();
      int v_1 = a;
      GroupMemoryBarrierWithGroupSync();
      if ((v < v_1)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        GroupMemoryBarrierWithGroupSync();
        int v_2 = b;
        GroupMemoryBarrierWithGroupSync();
        i = (i + v_2);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

