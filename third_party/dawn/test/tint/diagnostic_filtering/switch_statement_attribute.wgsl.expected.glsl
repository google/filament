<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  bool v = false;
  if ((x == 0.0f)) {
    v = (dFdx(1.0f) == 0.0f);
  } else {
    v = false;
  }
  switch(int(v)) {
    default:
    {
      break;
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
