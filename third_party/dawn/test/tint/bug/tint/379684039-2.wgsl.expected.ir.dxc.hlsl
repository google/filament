
static uint idx = 0u;
ByteAddressBuffer _storage : register(t2);
void main() {
  int2 vec = (int(0)).xx;
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      uint v = 0u;
      _storage.GetDimensions(v);
      if ((vec.y >= asint(_storage.Load((116u + (min(idx, ((v / 128u) - 1u)) * 128u)))))) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

