
RWByteAddressBuffer buffer : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int i = asint(buffer.Load(0u));
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        {
          uint2 tint_loop_idx_1 = (0u).xx;
          while(true) {
            if (all((tint_loop_idx_1 == (4294967295u).xx))) {
              break;
            }
            if ((i > int(5))) {
              i = (i * int(2));
              break;
            } else {
              i = (i * int(2));
              break;
            }
            /* unreachable */
          }
        }
        if ((i > int(10))) { break; }
      }
      continue;
    }
  }
  buffer.Store(0u, asuint(i));
}

