
RWByteAddressBuffer buf : register(u1);
int g() {
  return int(0);
}

int f() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      g();
      break;
    }
  }
  int o = g();
  return int(0);
}

[numthreads(1, 1, 1)]
void main() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((buf.Load(0u) == 0u)) {
        break;
      }
      int s = f();
      buf.Store(0u, 0u);
      {
        uint tint_low_inc_1 = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc_1;
        uint tint_carry_1 = uint((tint_low_inc_1 == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry_1);
      }
      continue;
    }
  }
}

