SKIP: FAILED

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
  ^^^^^

<dawn>/test/tint/diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  while((x > v.x)) {
    v = t.Sample(s, (0.0f).xx);
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
FXC validation failure:
<scrubbed_path>(11,9-30): warning X3570: gradient instruction used in a loop with varying iteration, attempting to unroll the loop
<scrubbed_path>(10,3-18): error X3511: unable to unroll loop, loop does not appear to terminate in a timely manner (1024 iterations)


tint executable returned error: exit status 1
