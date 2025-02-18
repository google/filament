
RWByteAddressBuffer i : register(u0);
void main() {
  {
    uint2 tint_loop_idx = (0u).xx;
    i.Store(0u, (i.Load(0u) - 1u));
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i.Load(0u) < 10u)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

