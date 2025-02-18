#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
layout(location = 1) flat in ivec3 tint_interstage_location1;
layout(location = 2) out int main_loc2_Output;
int f(int x) {
  if ((x == 10)) {
    continue_execution = false;
  }
  return x;
}
int main_inner(ivec3 x) {
  int y = x.x;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      int r = f(y);
      if ((r == 0)) {
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
  if (!(continue_execution)) {
    discard;
  }
  return y;
}
void main() {
  main_loc2_Output = main_inner(tint_interstage_location1);
}
