
ByteAddressBuffer rarr : register(t0);
void v() {
  int idx = int(3);
  int x = int2(int(1), int(2))[min(uint(idx), 1u)];
}

void v_1() {
  int idx = int(4);
  float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[min(uint(idx), 1u)];
}

void fixed_size_array() {
  int arr[2] = {int(1), int(2)};
  int idx = int(3);
  int x = arr[min(uint(idx), 1u)];
}

void runtime_size_array() {
  int idx = int(-1);
  uint v_2 = 0u;
  rarr.GetDimensions(v_2);
  uint v_3 = ((v_2 / 4u) - 1u);
  float x = asfloat(rarr.Load((0u + (min(uint(idx), v_3) * 4u))));
}

[numthreads(1, 1, 1)]
void f() {
  v();
  v_1();
  fixed_size_array();
  runtime_size_array();
}

