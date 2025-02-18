ByteAddressBuffer data : register(t1);

int foo() {
  uint tint_symbol_1 = 0u;
  data.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  return asint(data.Load((4u * min(0u, (tint_symbol_2 - 1u)))));
}

[numthreads(16, 16, 1)]
void main() {
  foo();
  return;
}
