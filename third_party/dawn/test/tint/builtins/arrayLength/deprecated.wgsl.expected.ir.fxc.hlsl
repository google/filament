
ByteAddressBuffer G : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  G.GetDimensions(v);
  uint l1 = (v / 4u);
  uint v_1 = 0u;
  G.GetDimensions(v_1);
  uint l2 = (v_1 / 4u);
}

