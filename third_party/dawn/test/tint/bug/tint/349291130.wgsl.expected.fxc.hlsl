Texture2D<float4> tint_symbol : register(t0);

[numthreads(6, 1, 1)]
void e() {
  {
    {
      uint3 tint_tmp;
      tint_symbol.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
      uint level = tint_tmp.z;
      for(; (level > 0u); ) {
      }
    }
  }
  return;
}
