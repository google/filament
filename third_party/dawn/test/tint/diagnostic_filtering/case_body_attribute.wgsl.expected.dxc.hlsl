<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:8:11 warning: 'textureSample' must only be called from uniform control flow
      _ = textureSample(t, s, vec2(0, 0));
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/case_body_attribute.wgsl:6:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) {
              ^

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  switch(tint_ftoi(x)) {
    case 0: {
      float4 tint_phony = t.Sample(s, (0.0f).xx);
      break;
    }
    default: {
      break;
    }
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
