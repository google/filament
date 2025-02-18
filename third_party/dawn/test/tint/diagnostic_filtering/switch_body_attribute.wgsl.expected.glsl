<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:5:11 warning: 'dpdx' must only be called from uniform control flow
      _ = dpdx(1.0);
          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
              ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float tint_interstage_location0;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void main_inner(float x) {
  switch(tint_f32_to_i32(x)) {
    default:
    {
      dFdx(1.0f);
      break;
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
