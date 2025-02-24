<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x; ) @diagnostic(warning, derivative_uniformity) {
  ^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D t_s;
layout(location = 0) in float tint_interstage_location0;
void main_inner(float x) {
  vec4 v = vec4(0.0f);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((x > v.x)) {
      } else {
        break;
      }
      v = texture(t_s, vec2(0.0f));
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
void main() {
  main_inner(tint_interstage_location0);
}
