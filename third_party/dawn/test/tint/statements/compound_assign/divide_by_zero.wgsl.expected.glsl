#version 310 es

int a = 0;
float b = 0.0f;
float tint_float_modulo(float x, float y) {
  return (x - (y * trunc((x / y))));
}
int tint_mod_i32(int lhs, int rhs) {
  uint v = uint((lhs == (-2147483647 - 1)));
  bool v_1 = bool((v & uint((rhs == -1))));
  uint v_2 = uint((rhs == 0));
  int v_3 = mix(rhs, 1, bool((v_2 | uint(v_1))));
  return (lhs - ((lhs / v_3) * v_3));
}
int tint_div_i32(int lhs, int rhs) {
  uint v_4 = uint((lhs == (-2147483647 - 1)));
  bool v_5 = bool((v_4 & uint((rhs == -1))));
  uint v_6 = uint((rhs == 0));
  return (lhs / mix(rhs, 1, bool((v_6 | uint(v_5)))));
}
void foo(int maybe_zero) {
  a = tint_div_i32(a, maybe_zero);
  a = tint_mod_i32(a, maybe_zero);
  b = (b / 0.0f);
  b = tint_float_modulo(b, 0.0f);
  float v_7 = float(maybe_zero);
  b = (b / v_7);
  float v_8 = float(maybe_zero);
  b = tint_float_modulo(b, v_8);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
