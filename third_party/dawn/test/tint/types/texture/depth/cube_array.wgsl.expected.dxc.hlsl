TextureCubeArray t_f : register(t0);

[numthreads(1, 1, 1)]
void main() {
  uint4 tint_tmp;
  t_f.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  uint2 dims = tint_tmp.xy;
  return;
}
