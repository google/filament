
ByteAddressBuffer weights : register(t0);
void main() {
  uint v = 0u;
  weights.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  float a = asfloat(weights.Load((0u + (min(uint(int(0)), v_1) * 4u))));
}

