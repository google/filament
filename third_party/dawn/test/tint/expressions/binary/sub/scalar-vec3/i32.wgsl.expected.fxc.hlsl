[numthreads(1, 1, 1)]
void f() {
  int a = 4;
  int3 b = int3(1, 2, 3);
  int3 r = (a - b);
  return;
}
