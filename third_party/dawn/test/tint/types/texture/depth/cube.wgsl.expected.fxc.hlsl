TextureCube t_f : register(t0);

[numthreads(1, 1, 1)]
void main() {
  uint3 tint_tmp;
  t_f.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint2 dims = tint_tmp.xy;
  return;
}
