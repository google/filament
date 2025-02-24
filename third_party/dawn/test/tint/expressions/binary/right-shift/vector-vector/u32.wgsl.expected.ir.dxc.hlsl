
[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1u, 2u, 3u);
  uint3 b = uint3(4u, 5u, 6u);
  uint3 r = (a >> (b & (31u).xxx));
}

