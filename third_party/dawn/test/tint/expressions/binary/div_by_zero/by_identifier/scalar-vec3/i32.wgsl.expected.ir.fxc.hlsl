
int3 tint_div_v3i32(int3 lhs, int3 rhs) {
  return (lhs / ((((rhs == (int(0)).xxx) | ((lhs == (int(-2147483648)).xxx) & (rhs == (int(-1)).xxx)))) ? ((int(1)).xxx) : (rhs)));
}

[numthreads(1, 1, 1)]
void f() {
  int a = int(4);
  int3 b = int3(int(0), int(2), int(0));
  int3 v = b;
  int3 r = tint_div_v3i32(int3((a).xxx), v);
}

