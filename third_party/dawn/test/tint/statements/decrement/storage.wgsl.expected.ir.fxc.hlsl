
RWByteAddressBuffer i : register(u0);
void main() {
  i.Store(0u, (i.Load(0u) - 1u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

