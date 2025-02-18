<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:8:14 warning: 'dpdx' must only be called from uniform control flow
  } else if (dpdx(1.0) > 0)  {
             ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  if ((x > 0.0f)) {
  } else {
    if ((dFdx(1.0f) > 0.0f)) {
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
