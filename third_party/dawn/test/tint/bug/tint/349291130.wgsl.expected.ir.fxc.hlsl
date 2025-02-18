
Texture2D<float4> v : register(t0);
[numthreads(6, 1, 1)]
void e() {
  {
    uint2 tint_loop_idx = (0u).xx;
    uint3 v_1 = (0u).xxx;
    v.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
    uint level = v_1.z;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((level > 0u)) {
      } else {
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

