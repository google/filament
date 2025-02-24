
[numthreads(1, 1, 1)]
void f() {
  int4 a = int4(int(1073757184), int(-1006616064), int(-998242304), int(987654321));
  uint4 b = asuint(a);
}

