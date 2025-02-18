struct main_outputs {
  int tint_symbol : SV_Target2;
};

struct main_inputs {
  nointerpolation int3 x : TEXCOORD1;
};


static bool continue_execution = true;
int f(int x) {
  if ((x == int(10))) {
    continue_execution = false;
  }
  return x;
}

int main_inner(int3 x) {
  int y = x.x;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      int r = f(y);
      if ((r == int(0))) {
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
  return y;
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.x)};
  if (!(continue_execution)) {
    discard;
  }
  return v;
}

