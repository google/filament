[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer a : register(u0);

void main() {
  int tint_symbol_1 = 1;
  a.Store((4u * min(uint(tint_symbol_1), 3u)), asuint((a.Load((4u * min(uint(tint_symbol_1), 3u))) + 1u)));
  a.Store(8u, asuint((a.Load(8u) + 1u)));
}
