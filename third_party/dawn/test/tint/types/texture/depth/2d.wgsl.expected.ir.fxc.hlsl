
Texture2D t_f : register(t0);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  t_f.GetDimensions(uint(int(0)), v.x, v.y, v.z);
  uint2 dims = v.xy;
}

