struct f_inputs {
  uint3 v : SV_DispatchThreadID;
};


uint tint_mod_u32(uint lhs, uint rhs) {
  uint v_1 = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}

void f_inner(uint3 v) {
  uint l = (v.x << (tint_mod_u32(v.y, 1u) & 31u));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.v);
}

