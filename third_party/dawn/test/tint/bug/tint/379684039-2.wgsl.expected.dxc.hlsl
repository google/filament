[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint idx = 0u;
ByteAddressBuffer _storage : register(t2);

void main() {
  int2 vec = (0).xx;
  while (true) {
    uint tint_symbol_1 = 0u;
    _storage.GetDimensions(tint_symbol_1);
    uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 128u);
    if ((vec.y >= asint(_storage.Load((((128u * min(idx, (tint_symbol_2 - 1u))) + 112u) + 4u))))) {
      break;
    }
  }
}
