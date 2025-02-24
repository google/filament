
RWByteAddressBuffer a : register(u0);
void main() {
  uint v = 0u;
  a.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = (min(uint(int(1)), v_1) * 4u);
  a.Store((0u + v_2), (a.Load((0u + v_2)) - 1u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

