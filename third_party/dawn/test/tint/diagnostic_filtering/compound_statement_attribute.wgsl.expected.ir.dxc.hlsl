<dawn>/test/tint/diagnostic_filtering/compound_statement_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/compound_statement_attribute.wgsl:7:5 note: control flow depends on possibly non-uniform value
    if (x > 0) {
    ^^

<dawn>/test/tint/diagnostic_filtering/compound_statement_attribute.wgsl:7:9 note: user-defined input 'x' of 'main' may be non-uniform
    if (x > 0) {
        ^

struct main_inputs {
  float x : TEXCOORD0;
};


Texture2D<float4> t : register(t1);
SamplerState s : register(s2);
void main_inner(float x) {
  if ((x > 0.0f)) {
    t.Sample(s, (0.0f).xx);
  }
}

void main(main_inputs inputs) {
  main_inner(inputs.x);
}

