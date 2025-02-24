int f(int a, int b, int c) {
  return ((a * b) + c);
}

[numthreads(1, 1, 1)]
void main() {
  f(1, 2, 3);
  return;
}
