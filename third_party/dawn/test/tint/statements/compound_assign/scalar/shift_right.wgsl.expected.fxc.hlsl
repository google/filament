[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer v : register(u0);

void foo() {
  v.Store(0u, asuint((asint(v.Load(0u)) >> 2u)));
}
