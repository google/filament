<dawn>/test/tint/diagnostic_filtering/directive.wgsl:9:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/directive.wgsl:8:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/directive.wgsl:8:7 note: user-defined input 'x' of 'main' may be non-uniform
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

