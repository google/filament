
[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(int(1073757184), int(-1006616064), int(-998242304));
  uint3 b = asuint(a);
}

