<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

struct main_inputs {
  float x : TEXCOORD0;
};


void main_inner(float x) {
  bool v = false;
  if ((x == 0.0f)) {
    v = (ddx(1.0f) == 0.0f);
  } else {
    v = false;
  }
  switch(int(v)) {
    default:
    {
      break;
    }
  }
}

void main(main_inputs inputs) {
  main_inner(inputs.x);
}

