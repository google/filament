<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
layout(location = 0) in float tint_interstage_location0;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void main_inner(float x) {
  switch(tint_f32_to_i32(x)) {
    case 0:
    {
      texture(t_s, vec2(0.0f));
      break;
    }
    default:
    {
      break;
    }
  }
}
void main() {
  main_inner(tint_interstage_location0);
}
