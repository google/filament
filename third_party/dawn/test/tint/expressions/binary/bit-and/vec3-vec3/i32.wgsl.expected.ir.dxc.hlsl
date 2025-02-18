
[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(int(1), int(2), int(3));
  int3 b = int3(int(4), int(5), int(6));
  int3 r = (a & b);
}

