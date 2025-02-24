[numthreads(1, 1, 1)]
void f() {
  int3 a = int3(1, 2, 3);
  uint3 b = uint3(4u, 5u, 6u);
  int3 r = (a >> (b & (31u).xxx));
  return;
}
