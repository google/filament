<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
  ^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

struct main_inputs {
  float x : TEXCOORD0;
};


Texture2D<float4> t : register(t1);
SamplerState s : register(s2);
void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((x > v.x)) {
      } else {
        break;
      }
      v = t.Sample(s, (0.0f).xx);
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

