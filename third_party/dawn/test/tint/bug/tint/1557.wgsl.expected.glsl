#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  int inner;
} v;
int f() {
  return 0;
}
void g() {
  int j = 0;
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((j >= 1)) {
        break;
      }
      j = (j + 1);
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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  switch(v.inner) {
    case 0:
    {
      switch(v.inner) {
        case 0:
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
