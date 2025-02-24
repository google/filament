
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
int f() {
  return int(0);
}

void g() {
  int j = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((j >= int(1))) {
        break;
      }
      j = (j + int(1));
      int k = f();
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
void main() {
  switch(asint(u[0u].x)) {
    case int(0):
    {
      switch(asint(u[0u].x)) {
        case int(0):
        {
          break;
        }
        default:
        {
          g();
          break;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}

