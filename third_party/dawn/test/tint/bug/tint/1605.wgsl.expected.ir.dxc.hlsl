
cbuffer cbuffer_b : register(b0) {
  uint4 b[1];
};
bool func_3() {
  {
    uint2 tint_loop_idx = (0u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < asint(b[0u].x))) {
      } else {
        break;
      }
      {
        uint2 tint_loop_idx_1 = (0u).xx;
        int j = int(-1);
        while(true) {
          if (all((tint_loop_idx_1 == (4294967295u).xx))) {
            break;
          }
          if ((j == int(1))) {
          } else {
            break;
          }
          return false;
        }
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
  return false;
}

[numthreads(1, 1, 1)]
void main() {
  func_3();
}

