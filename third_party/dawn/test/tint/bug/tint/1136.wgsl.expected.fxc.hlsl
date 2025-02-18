[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer buffer : register(u0);

void main() {
  buffer.Store(0u, asuint((buffer.Load(0u) + 1u)));
}
