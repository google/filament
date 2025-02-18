
ByteAddressBuffer arr : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  arr.GetDimensions(v);
  uint len = (v / 4u);
}

