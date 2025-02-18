[numthreads(1, 1, 1)]
void f() {
  uint a = 4u;
  uint3 b = uint3(1u, 2u, 3u);
  uint3 r = (a + b);
  return;
}
