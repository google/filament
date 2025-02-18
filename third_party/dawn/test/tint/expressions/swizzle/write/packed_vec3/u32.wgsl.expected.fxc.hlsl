[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer U : register(u0);

void f() {
  U.Store3(0u, asuint(uint3(1u, 2u, 3u)));
  U.Store(0u, asuint(1u));
  U.Store(4u, asuint(2u));
  U.Store(8u, asuint(3u));
}
