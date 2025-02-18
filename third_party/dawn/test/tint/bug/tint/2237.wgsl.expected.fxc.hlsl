RWByteAddressBuffer buffer : register(u0);

uint foo() {
  uint tint_symbol_2[4] = {0u, 1u, 2u, 4u};
  return tint_symbol_2[min(buffer.Load(0u), 3u)];
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_3[4] = {0u, 1u, 2u, 4u};
  uint v = tint_symbol_3[min(buffer.Load(0u), 3u)];
  uint tint_symbol = v;
  uint tint_symbol_1 = foo();
  buffer.Store(0u, asuint((tint_symbol + tint_symbol_1)));
  return;
}
