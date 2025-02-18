<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:5:9 warning: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:7:7 note: control flow depends on possibly non-uniform value
      break if x > 0.0;
      ^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_attribute.wgsl:7:16 note: user-defined input 'x' of 'main' may be non-uniform
      break if x > 0.0;
               ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      dFdx(1.0f);
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        if ((x > 0.0f)) { break; }
      }
      continue;
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
