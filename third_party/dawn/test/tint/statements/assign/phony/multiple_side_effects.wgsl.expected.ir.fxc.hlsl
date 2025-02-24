
int f(int a, int b, int c) {
  return ((a * b) + c);
}

[numthreads(1, 1, 1)]
void main() {
  int v = f(int(1), int(2), int(3));
  int v_1 = f(int(4), int(5), int(6));
  int v_2 = (v + (v_1 * f(int(7), f(int(8), int(9), int(10)), int(11))));
}

