<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

struct tint_symbol_3 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  bool tint_symbol = (x == 0.0f);
  if (tint_symbol) {
    float tint_symbol_1 = ddx(1.0f);
    tint_symbol = (tint_symbol_1 == 0.0f);
  }
  do {
  } while (false);
}

void main(tint_symbol_3 tint_symbol_2) {
  main_inner(tint_symbol_2.x);
  return;
}
