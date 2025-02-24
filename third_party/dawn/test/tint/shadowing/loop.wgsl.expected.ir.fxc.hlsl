
RWByteAddressBuffer output : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  int i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      int x = asint(output.Load((0u + (min(uint(i), 9u) * 4u))));
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        int x_1 = asint(output.Load((0u + (min(uint(x), 9u) * 4u))));
        i = (i + x_1);
        if ((i > int(10))) { break; }
      }
      continue;
    }
  }
  output.Store(0u, asuint(i));
}

