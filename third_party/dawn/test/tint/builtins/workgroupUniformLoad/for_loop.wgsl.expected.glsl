#version 310 es

shared int a;
shared int b;
void foo() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    int i = 0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      int v = i;
      barrier();
      int v_1 = a;
      barrier();
      if ((v < v_1)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        barrier();
        int v_2 = b;
        barrier();
        i = (i + v_2);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
