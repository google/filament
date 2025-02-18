<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:8:14 warning: 'dpdx' must only be called from uniform control flow
  } else if (dpdx(1.0) > 0)  {
             ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

<dawn>/test/tint/diagnostic_filtering/if_statement_attribute.wgsl:7:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

struct tint_symbol_2 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  if ((x > 0.0f)) {
  } else {
    float tint_symbol = ddx(1.0f);
    if ((tint_symbol > 0.0f)) {
    }
  }
}

void main(tint_symbol_2 tint_symbol_1) {
  main_inner(tint_symbol_1.x);
  return;
}
