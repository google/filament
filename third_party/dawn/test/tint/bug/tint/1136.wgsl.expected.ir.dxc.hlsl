
RWByteAddressBuffer buffer : register(u0);
void main() {
  buffer.Store(0u, (buffer.Load(0u) + 1u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

