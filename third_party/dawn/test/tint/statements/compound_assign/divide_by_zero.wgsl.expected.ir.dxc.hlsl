
static int a = int(0);
static float b = 0.0f;
int tint_mod_i32(int lhs, int rhs) {
  int v = ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs));
  return (lhs - ((lhs / v) * v));
}

int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs)));
}

void foo(int maybe_zero) {
  a = tint_div_i32(a, maybe_zero);
  a = tint_mod_i32(a, maybe_zero);
  b = (b / 0.0f);
  float v_1 = b;
  float v_2 = (v_1 / 0.0f);
  b = (v_1 - ((((v_2 < 0.0f)) ? (ceil(v_2)) : (floor(v_2))) * 0.0f));
  float v_3 = float(maybe_zero);
  b = (b / v_3);
  float v_4 = float(maybe_zero);
  float v_5 = b;
  float v_6 = (v_5 / v_4);
  b = (v_5 - ((((v_6 < 0.0f)) ? (ceil(v_6)) : (floor(v_6))) * v_4));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

