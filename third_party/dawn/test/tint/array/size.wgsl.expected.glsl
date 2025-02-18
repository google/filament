#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_s_block_ssbo {
  float inner;
} v;
void main() {
  float signed_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_literal[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float signed_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float unsigned_constant[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float shr_const_expr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  unsigned_literal = signed_literal;
  signed_constant = signed_literal;
  unsigned_constant = signed_literal;
  shr_const_expr = signed_literal;
  v.inner = ((((signed_literal[0u] + unsigned_literal[0u]) + signed_constant[0u]) + unsigned_constant[0u]) + shr_const_expr[0u]);
}
