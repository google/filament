
[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(int(1), int(2), int(3));
  int b = int(4);
  int3 r = (a * b);
}

