void tint_symbol() {
  int idx = 3;
  int x = int2(1, 2)[min(uint(idx), 1u)];
}

void tint_symbol_1() {
  int idx = 4;
  float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[min(uint(idx), 1u)];
}

void fixed_size_array() {
  int arr[2] = {1, 2};
  int idx = 3;
  int x = arr[min(uint(idx), 1u)];
}

ByteAddressBuffer rarr : register(t0);

void runtime_size_array() {
  uint tint_symbol_3 = 0u;
  rarr.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = (tint_symbol_3 / 4u);
  int idx = -1;
  float x = asfloat(rarr.Load((4u * min(uint(idx), (tint_symbol_4 - 1u)))));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol();
  tint_symbol_1();
  fixed_size_array();
  runtime_size_array();
  return;
}
