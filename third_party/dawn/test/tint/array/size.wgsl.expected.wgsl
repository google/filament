@group(0) @binding(0) var<storage, read_write> s : f32;

const slen = 4;

const ulen = 4u;

@fragment
fn main() {
  var signed_literal : array<f32, 4>;
  var unsigned_literal : array<f32, 4u>;
  var signed_constant : array<f32, slen>;
  var unsigned_constant : array<f32, ulen>;
  var shr_const_expr : array<f32, (1 << 2)>;
  unsigned_literal = signed_literal;
  signed_constant = signed_literal;
  unsigned_constant = signed_literal;
  shr_const_expr = signed_literal;
  s = ((((signed_literal[0] + unsigned_literal[0]) + signed_constant[0]) + unsigned_constant[0]) + shr_const_expr[0]);
}
