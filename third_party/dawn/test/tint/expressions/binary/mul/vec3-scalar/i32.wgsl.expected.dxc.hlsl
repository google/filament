[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  int b = 4;
  int3 r = (a * b);
  return;
}
