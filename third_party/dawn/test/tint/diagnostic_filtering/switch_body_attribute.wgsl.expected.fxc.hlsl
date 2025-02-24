<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:5:11 warning: 'dpdx' must only be called from uniform control flow
      _ = dpdx(1.0);
          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:3 note: control flow depends on possibly non-uniform value
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
  ^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_body_attribute.wgsl:3:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x)) @diagnostic(warning, derivative_uniformity) {
              ^

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  tint_ftoi(x);
  do {
    float tint_phony = ddx(1.0f);
  } while (false);
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
