
ByteAddressBuffer G : register(t0);
void n() {
  uint v = 0u;
  G.GetDimensions(v);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

