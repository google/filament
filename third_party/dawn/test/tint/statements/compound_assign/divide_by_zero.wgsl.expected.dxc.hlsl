float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int a = 0;
static float b = 0.0f;

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

int tint_mod(int lhs, int rhs) {
  int rhs_or_one = (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs);
  if (any(((uint((lhs | rhs_or_one)) & 2147483648u) != 0u))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

void foo(int maybe_zero) {
  a = tint_div(a, maybe_zero);
  a = tint_mod(a, maybe_zero);
  b = (b / 0.0f);
  b = tint_float_mod(b, 0.0f);
  b = (b / float(maybe_zero));
  b = tint_float_mod(b, float(maybe_zero));
}
