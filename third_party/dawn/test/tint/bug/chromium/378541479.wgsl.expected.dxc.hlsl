cbuffer cbuffer_level : register(b0) {
  uint4 level[1];
};
cbuffer cbuffer_coords : register(b1) {
  uint4 coords[1];
};
Texture2D tex : register(t2);

[numthreads(1, 1, 1)]
void compute_main() {
  float res = tex.Load(uint3(coords[0].xy, level[0].x)).x;
  return;
}
