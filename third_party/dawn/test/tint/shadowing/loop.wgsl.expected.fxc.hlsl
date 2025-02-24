RWByteAddressBuffer output : register(u0);

[numthreads(1, 1, 1)]
void foo() {
  int i = 0;
  while (true) {
    int x = asint(output.Load((4u * min(uint(i), 9u))));
    {
      int x_1 = asint(output.Load((4u * min(uint(x), 9u))));
      i = (i + x_1);
      if ((i > 10)) { break; }
    }
  }
  output.Store(0u, asuint(i));
  return;
}
