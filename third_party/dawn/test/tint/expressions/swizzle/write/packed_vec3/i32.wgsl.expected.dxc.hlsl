[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer U : register(u0);

void f() {
  U.Store3(0u, asuint(int3(1, 2, 3)));
  U.Store(0u, asuint(1));
  U.Store(4u, asuint(2));
  U.Store(8u, asuint(3));
}
