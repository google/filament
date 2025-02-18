
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void tint_entry_point() {
  v.Store(0u, 0u);
}

