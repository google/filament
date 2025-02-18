
Texture2DArray t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint4 v = (0u).xxxx;
  t_f.GetDimensions(uint(int(0)), v.x, v.y, v.z, v.w);
  uint2 dims = v.xy;
}

