
ByteAddressBuffer data : register(t1);
int foo() {
  uint v = 0u;
  data.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  return asint(data.Load((0u + (min(uint(int(0)), v_1) * 4u))));
}

[numthreads(16, 16, 1)]
void main() {
  foo();
}

