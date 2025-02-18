<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 warning: 'dpdx' must only be called from uniform control flow
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
  ^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_attribute.wgsl:5:21 note: return value of 'dpdx' may be non-uniform
  for (; x > v.x && dpdx(1.0) > 0.0; ) {
                    ^^^^^^^^^

struct tint_symbol_3 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  while (true) {
    bool tint_symbol = (x > v.x);
    if (tint_symbol) {
      float tint_symbol_1 = ddx(1.0f);
      tint_symbol = (tint_symbol_1 > 0.0f);
    }
    if (!(tint_symbol)) {
      break;
    }
    {
    }
  }
}

void main(tint_symbol_3 tint_symbol_2) {
  main_inner(tint_symbol_2.x);
  return;
}
