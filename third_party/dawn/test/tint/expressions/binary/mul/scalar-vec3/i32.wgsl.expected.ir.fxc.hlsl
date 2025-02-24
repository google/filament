
[numthreads(1, 1, 1)]
void f() {
  int a = int(4);
  int3 b = int3(int(1), int(2), int(3));
  int3 r = (a * b);
}

