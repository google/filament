<dawn>/test/tint/diagnostic_filtering/while_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
                    ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
  ^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  while (x > 0.0 && dpdx(1.0) > 0.0)  {
                    ^^^^^^^^^

struct main_inputs {
  float x : TEXCOORD0;
};


void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      bool v_1 = false;
      if ((x > 0.0f)) {
        v_1 = (ddx(1.0f) > 0.0f);
      } else {
        v_1 = false;
      }
      if (v_1) {
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

void main(main_inputs inputs) {
  main_inner(inputs.x);
}

