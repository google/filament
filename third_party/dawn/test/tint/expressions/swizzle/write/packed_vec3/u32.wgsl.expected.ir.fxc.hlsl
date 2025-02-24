
RWByteAddressBuffer U : register(u0);
void f() {
  U.Store3(0u, uint3(1u, 2u, 3u));
  U.Store(0u, 1u);
  U.Store(4u, 2u);
  U.Store(8u, 3u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

