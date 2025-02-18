
RWByteAddressBuffer U : register(u0);
void f() {
  U.Store3(0u, asuint(int3(int(1), int(2), int(3))));
  U.Store(0u, asuint(int(1)));
  U.Store(4u, asuint(int(2)));
  U.Store(8u, asuint(int(3)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

